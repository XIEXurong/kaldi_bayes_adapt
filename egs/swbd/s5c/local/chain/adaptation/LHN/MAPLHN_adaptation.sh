#!/bin/bash

set -e

# configs for 'chain'
stage=0
train_stage=-10
get_egs_stage=-10
baseline=tdnn_7q_hires_sp
adapt_ivector_dir=exp/nnet3/ivectors_eval2000
test_ivector_dir=exp/nnet3/ivectors_eval2000
epoch_num=7
lr1=0.001
lr2=0.001
num_chunk=64 # use a small size if out of memory
param_init_file=
tag= # any other marks

decode_iter=
decode_nj=50

# training options
frames_per_eg=150,100,50,20,10,5
remove_egs=false
xent_regularize=1.0

adapted_layer="tdnnf2"
layer_dim="160" # should be corresponding to the $adapted_layer
KL_scale="0.000001"
input_config="component-node name=idct component=idct input=feature1" # cnn-tdnn, for tdnn, it can be "component-node name=lda component=lda input=Append(Offset(feature1, -1), feature1, Offset(feature1, 1), ReplaceIndex(ivector, t, 0))"
input_dim=41
prior_mean_file=  # a file or a list of files
prior_std_file=  # a file or a list of files
prior_mean="0.0" # a value or a list of values
prior_std="1.0"  # a value or a list of values
common_egs_dir=

# End configuration section.
echo "$0 $@"  # Print the command line for logging

. ./cmd.sh
. ./path.sh
. ./utils/parse_options.sh

adapted_layer_array=($adapted_layer)
layer_dim_array=($layer_dim)
KL_scale_array=($KL_scale)
prior_mean_file_array=($prior_mean_file)
prior_std_file_array=($prior_std_file)
prior_mean_array=($prior_mean)
prior_std_array=($prior_std)
param_init_file_array=($param_init_file)
layer_num=`echo ${#adapted_layer_array[*]}`
layer_num1=`echo ${#layer_dim_array[*]}`
layer_num2=`echo ${#KL_scale_array[*]}`

[[ "$layer_num" == "$layer_num1" && "$layer_num" == "$layer_num2" ]] || exit 1;

adapt_set=$1 # eval2000_hires_spk_sub20
label_lat_dir=$2 # label_lat_dir=exp/chain/cnn_tdnn_iv_1a_hires_sp/decode_eval2000_hires_sw1_fsh_fg/1BEST_lat/score_10_0.0
decode_set=$3 # eval2000_hires_spk

version=_MAPLHN${tag}_adaptlayer${layer_num}_batch${num_chunk}_epoch${epoch_num}_lr1${lr1}_lr2${lr2}

dirbase=exp/chain/${baseline}
dir=exp/chain/adaptation/LHN/${baseline}${version}

if [ $stage -le 0 ]; then

mkdir -p $dir
cp -r $dirbase/{configs,phones.txt,phone_lm.fst,tree,den.fst,normalization.fst,0.trans_mdl} $dir/
cp -r $dirbase/{final.mdl,tree,phones.txt} $label_lat_dir/

spk_count=data/${adapt_set}/spk_count
spk_num=`cat data/${adapt_set}/num_spk`
input_dim_nospk=$(awk "BEGIN{print($input_dim-1)}")

# input features
cat <<EOF > $dir/configs/change.config
	input-node name=input dim=$input_dim
	
	# acoustic features
	dim-range-node name=feature1 input-node=input dim=$input_dim_nospk dim-offset=0
	${input_config}

	# speaker id
	dim-range-node name=feature2 input-node=input dim=1 dim-offset=40
EOF


layer_num_minus1=$(awk "BEGIN{print($layer_num-1)}")

# adaptation in each layer
for i in `seq 0 $layer_num_minus1`; do

layer=`echo ${adapted_layer_array[i]}`
dim_tmp=`echo ${layer_dim_array[i]}`
dim_tmp2=$(awk "BEGIN{print($dim_tmp*$dim_tmp)}")
dim_tmp2_plus_dim_tmp=$(awk "BEGIN{print($dim_tmp2+$dim_tmp)}")
dim_tmp2_x4=$(awk "BEGIN{print(4*$dim_tmp2)}")
KL=`echo ${KL_scale_array[i]}`


if [[ "$i" < "${#prior_mean_array[*]}" ]]; then
  prior_mean_tmp=`echo ${prior_mean_array[i]}`
else
  prior_mean_tmp=`echo ${prior_mean_array[-1]}`
fi
prior_mean_config="output-mean=$prior_mean_tmp output-stddev=0"
if [ ! -z $prior_mean_file ]; then
  if [[ "$i" < "${#prior_mean_file_array[*]}" ]]; then
    prior_mean_file_tmp=`echo ${prior_mean_file_array[i]}`
  else
    prior_mean_file_tmp=`echo ${prior_mean_file_array[-1]}`
  fi
  prior_mean_config="vector=$prior_mean_file_tmp"
fi
if [[ "$i" < "${#prior_std_array[*]}" ]]; then
  prior_std_tmp=`echo ${prior_std_array[i]}`
else
  prior_std_tmp=`echo ${prior_std_array[-1]}`
fi
prior_std_config="output-mean=$prior_std_tmp output-stddev=0"
if [ ! -z $prior_std_file ]; then
  if [[ "$i" < "${#prior_std_file_array[*]}" ]]; then
    prior_std_file_tmp=`echo ${prior_std_file_array[i]}`
  else
    prior_std_file_tmp=`echo ${prior_std_file_array[-1]}`
  fi
  prior_std_config="vector=$prior_std_file_tmp"
fi

if [[ "$i" < "${#prior_mean_array[*]}" || "$i" < "${#prior_mean_file_array[*]}" || "$i" < "${#prior_std_array[*]}" || "$i" < "${#prior_std_file_array[*]}" ]]; then
  prior_layer=$layer
cat <<EOF >> $dir/configs/change.config
	component name=MAPLHN.prior_mean.$prior_layer type=ConstantFunctionComponent input-dim=1 is-updatable=false output-dim=$dim_tmp2 $prior_mean_config
	component-node name=MAPLHN.prior_mean.$prior_layer component=MAPLHN.prior_mean.$prior_layer input=feature2
	component name=MAPLHN.prior_std.$prior_layer type=ConstantFunctionComponent input-dim=1 is-updatable=false output-dim=$dim_tmp2 $prior_std_config
	component-node name=MAPLHN.prior_std.$prior_layer component=MAPLHN.prior_std.$prior_layer input=feature2
EOF
fi

param_init_config=
if [ ! -z $param_init_file ]; then
  param_init_config="matrix=${param_init_file_array[i]}"
fi

cat <<EOF >> $dir/configs/change.config	
	component name=MAPLHN.mean.$layer type=LinearSelectColComponent input-dim=1 output-dim=$dim_tmp2 col-num=$spk_num l2-regularize=0.00 use-natural-gradient=false $param_init_config
	component-node name=MAPLHN.mean.$layer component=MAPLHN.mean.$layer input=feature2
EOF

# the param std is unused and defined to keep the format
cat <<EOF >> $dir/configs/change.config	
	component name=MAPLHN.std.$layer type=LinearSelectColComponent input-dim=1 output-dim=$dim_tmp2 col-num=$spk_num l2-regularize=0.00 param-mean=1 param-stddev=0 use-natural-gradient=false learning-rate-factor=0
	component-node name=MAPLHN.std.$layer component=MAPLHN.std.$layer input=feature2
EOF

cat <<EOF >> $dir/configs/change.config	
	# compared to BLHN, no random effect and no frame counts are used
	component name=MAPLHN.vec.$layer type=BayesVecKLGaussianComponent output-dim=$dim_tmp2 input-dim=$dim_tmp2_x4 KL-scale=${KL} input-frame-scale=false rand-per-frame=false KL-output=false test-mode=false fix-rand-val=true
	component-node name=MAPLHN.vec.$layer component=MAPLHN.vec.$layer input=Append(MAPLHN.mean.$layer, MAPLHN.std.$layer, MAPLHN.prior_mean.$prior_layer, MAPLHN.prior_std.$prior_layer)
	component name=MAPLHN.multiply.$layer type=FramewiseLinearComponent input-dim=$dim_tmp2_plus_dim_tmp output-dim=$dim_tmp feat-dim=$dim_tmp
	component-node name=MAPLHN.multiply.$layer component=MAPLHN.multiply.$layer input=Append(tdnnf$layer.linear,MAPLHN.linear.$layer)
	component-node name=$layer.affine component=$layer.affine input=MAPLHN.multiply.$layer
EOF

done

# use cross entropy only
cat <<EOF >> $dir/configs/change.config
	component name=no_mmi type=NoOpComponent dim=256 backprop-scale=0.0
	component-node name=no_mmi component=no_mmi input=prefinal-l
	component-node name=prefinal-chain.affine component=prefinal-chain.affine input=no_mmi
EOF

nnet3-am-copy --raw --binary=false --edits="set-learning-rate-factor learning-rate-factor=0" $dirbase/final.mdl - | \
 sed "s/<TestMode> F/<TestMode> T/g" | sed "s/BatchNormComponent/BatchNormTestComponent/g" | sed "s/<OrthonormalConstraint> [^ ]* /<OrthonormalConstraint> 0/g" | \
 nnet3-copy --nnet-config=$dir/configs/change.config - $dir/0.raw

nnet3-info $dir/0.raw > $dir/0.raw.info

fi

if [ $stage -le 1 ]; then
  local/chain/adaptation/train_adapt.py --stage $train_stage \
    --cmd "$train_cmd" \
	--feat.online-ivector-dir $adapt_ivector_dir \
    --feat.cmvn-opts "--norm-means=false --norm-vars=false" \
    --chain.xent-regularize $xent_regularize \
    --chain.leaky-hmm-coefficient 0.1 \
    --chain.l2-regularize 0.0 \
    --chain.apply-deriv-weights false \
    --chain.lm-opts="--num-extra-lm-states=2000" \
	--chain.alignment-subsampling-factor 1 \
    --trainer.add-option="--optimization.memory-compression-level=2" \
    --egs.dir "$common_egs_dir" \
    --egs.stage $get_egs_stage \
    --egs.opts "--frames-overlap-per-eg 0 --constrained false" \
    --egs.chunk-width $frames_per_eg \
    --trainer.num-chunk-per-minibatch ${num_chunk} \
    --trainer.frames-per-iter 1500000 \
    --trainer.num-epochs $epoch_num \
    --trainer.optimization.num-jobs-initial 1 \
    --trainer.optimization.num-jobs-final 1 \
    --trainer.optimization.initial-effective-lrate $lr1 \
    --trainer.optimization.final-effective-lrate $lr2 \
	--trainer.optimization.do-final-combination false \
    --trainer.max-param-change 2.0 \
	--trainer.input-model $dir/0.raw \
    --cleanup.remove-egs $remove_egs \
    --feat-dir data/${adapt_set} \
    --lat-dir $label_lat_dir \
    --dir $dir || exit 1;

  mv $dir/final.mdl $dir/final_ori.mdl
  nnet3-am-copy --binary=false $dir/final_ori.mdl - | \
   sed "s/<TestMode> F/<TestMode> T/g" > $dir/final.mdl

  nnet3-am-info $dir/final.mdl > $dir/final.mdl.info
fi

if [ $stage -le 2 ]; then
  # Note: it might appear that this $lang directory is mismatched, and it is as
  # far as the 'topo' is concerned, but this script doesn't read the 'topo' from
  # the lang directory.
  utils/mkgraph.sh --self-loop-scale 1.0 data/lang_sw1_tg $dir $dir/graph_sw1_tg
fi


graph_dir=$dir/graph_sw1_tg
iter_opts=
if [ ! -z $decode_iter ]; then
  iter_opts=" --iter $decode_iter "
fi
if [ $stage -le 3 ]; then
  rm $dir/.error 2>/dev/null || true
  (
  steps/nnet3/decode.sh --acwt 1.0 --post-decode-acwt 10.0 \
	  --nj $decode_nj --cmd "$decode_cmd" $iter_opts \
	  --online-ivector-dir $test_ivector_dir \
	  $graph_dir data/${decode_set} \
	  $dir/decode_${decode_set}${decode_iter:+_$decode_iter}_sw1_tg || exit 1;
	  
	  steps/lmrescore_const_arpa.sh --cmd "$decode_cmd" \
		data/lang_sw1_{tg,fsh_fg} data/${decode_set} \
		$dir/decode_${decode_set}${decode_iter:+_$decode_iter}_sw1_{tg,fsh_fg} || exit 1;

  ) || touch $dir/.error &
  wait
  if [ -f $dir/.error ]; then
    echo "$0: something went wrong in decoding"
    exit 1
  fi
fi

