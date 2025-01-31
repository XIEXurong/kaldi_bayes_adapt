#!/usr/bin/env bash
# Copyright    2017  Hossein Hadian

# steps/info/chain_dir_info.pl exp/chain/e2e_tdnnf_1a
# exp/chain/e2e_tdnnf_1a: num-iters=180 nj=2..8 num-params=6.8M dim=40->84 combine=-0.060->-0.060 (over 3) logprob:train/valid[119,179,final]=(-0.080,-0.062,-0.062/-0.089,-0.083,-0.083)

set -e

# configs for 'chain'
stage=2
train_stage=-10
get_egs_stage=-10
affix=1a_ivpca3_bpe3g_mmice_specaugkaldi
if [ -e data/rt03 ]; then maybe_rt03=rt03; else maybe_rt03= ; fi

decode_nj=50

# training options
dropout_schedule='0,0@0.20,0.3@0.50,0'
num_epochs=6
num_jobs_initial=3
num_jobs_final=16
minibatch_size=150=64/300=64,32/600=32,16/1200=8
frames_per_chunk_primary=150
chunk_left_context=40
chunk_right_context=40
xent_regularize=0.025
label_delay=0
extra_left_context=50
extra_right_context=50
common_egs_dir=
l2_regularize=0.00005
frames_per_iter=1500000
cmvn_opts="--norm-means=false --norm-vars=false"
train_set=train_nodup_spe2e_hires
add_opt=
cleanup=true

gpu_memory_required=7000
gpu_exclusive=true

# End configuration section.
echo "$0 $@"  # Print the command line for logging

. ./cmd.sh
. ./path.sh
. ./utils/parse_options.sh

if ! cuda-compiled; then
  cat <<EOF && exit 1
This script is intended to be used with GPUs but you have not compiled Kaldi with CUDA
If you want to use GPUs (and have them), go to src/, and configure and make on a machine
where "nvcc" is installed.
EOF
fi

lang=data/lang_e2e_bpe
treedir=exp/chain/e2e_tree_bpe3g  # it's actually just a trivial tree (no tree building)
dir=exp/chain/e2e_cnn_tdnn_blstm_${affix}


if [ $stage -le 2 ]; then
  echo "$0: creating neural net configs using the xconfig parser";
  num_targets=$(tree-info $treedir/tree |grep num-pdfs|awk '{print $2}')
  learning_rate_factor=$(echo "print (0.5/$xent_regularize)" | python)
  
  cnn_opts="l2-regularize=0.01"
  ivector_affine_opts="l2-regularize=0.01"
  linear_opts="orthonormal-constraint=1.0"
  lstm_opts="l2-regularize=0.0005 decay-time=20"
  opts="l2-regularize=0.002"
  output_opts="l2-regularize=0.0005 output-delay=$label_delay max-change=1.5 dim=$num_targets"

  mkdir -p $dir/configs
  cat <<EOF > $dir/configs/network.xconfig
  input dim=100 name=ivector
  input dim=40 name=input

  idct-layer name=idct input=input dim=40 cepstral-lifter=22 affine-transform-file=$dir/configs/idct.mat
  linear-component name=ivector-linear $ivector_affine_opts dim=200 input=ReplaceIndex(ivector, t, 0)
  batchnorm-component name=ivector-batchnorm target-rms=0.025
  batchnorm-component name=idct-batchnorm input=idct
  spec-augment-layer name=idct-spec-augment freq-max-proportion=0.5 time-zeroed-proportion=0.2 time-mask-max-frames=20
  combine-feature-maps-layer name=combine_inputs input=Append(idct-spec-augment, ivector-batchnorm) num-filters1=1 num-filters2=5 height=40
  conv-relu-batchnorm-layer name=cnn1 $cnn_opts height-in=40 height-out=40 time-offsets=-1,0,1 height-offsets=-1,0,1 num-filters-out=64 
  conv-relu-batchnorm-layer name=cnn2 $cnn_opts height-in=40 height-out=40 time-offsets=-1,0,1 height-offsets=-1,0,1 num-filters-out=64
  conv-relu-batchnorm-layer name=cnn3 $cnn_opts height-in=40 height-out=20 height-subsample-out=2 time-offsets=-1,0,1 height-offsets=-1,0,1 num-filters-out=128
  conv-relu-batchnorm-layer name=cnn4 $cnn_opts height-in=20 height-out=20 time-offsets=-1,0,1 height-offsets=-1,0,1 num-filters-out=128
  conv-relu-batchnorm-layer name=cnn5 $cnn_opts height-in=20 height-out=10 height-subsample-out=2 time-offsets=-1,0,1 height-offsets=-1,0,1 num-filters-out=256
  conv-relu-batchnorm-layer name=cnn6 $cnn_opts height-in=10 height-out=10  time-offsets=-1,0,1 height-offsets=-1,0,1 num-filters-out=256

  # the first splicing is moved before the lda layer, so no splicing here
  relu-batchnorm-layer name=tdnn1 $opts dim=1280
  linear-component name=tdnn2l dim=256 $linear_opts input=Append(-1,0)
  relu-batchnorm-layer name=tdnn2 $opts input=Append(0,1) dim=1280
  linear-component name=tdnn3l dim=256 $linear_opts
  relu-batchnorm-layer name=tdnn3 $opts dim=1280
  linear-component name=tdnn4l dim=256 $linear_opts input=Append(-1,0)
  relu-batchnorm-layer name=tdnn4 $opts input=Append(0,1) dim=1280
  linear-component name=tdnn5l dim=256 $linear_opts
  relu-batchnorm-layer name=tdnn5 $opts dim=1280 input=Append(tdnn5l, tdnn3l)
  linear-component name=tdnn6l dim=256 $linear_opts input=Append(-3,0)
  relu-batchnorm-layer name=tdnn6 $opts input=Append(0,3) dim=1280
  linear-component name=lstm1l dim=256 $linear_opts input=Append(-3,0,3)
  fast-lstmp-layer name=lstm1-forward input=lstm1l cell-dim=1024 recurrent-projection-dim=256 non-recurrent-projection-dim=128 delay=-3 dropout-proportion=0.0 $lstm_opts
  fast-lstmp-layer name=lstm1-backward input=lstm1l cell-dim=1024 recurrent-projection-dim=256 non-recurrent-projection-dim=128 delay=3 dropout-proportion=0.0 $lstm_opts
  no-op-component name=blstm1 input=Append(lstm1-forward,lstm1-backward)
  relu-batchnorm-layer name=tdnn7 $opts input=Append(-3,0,3,tdnn6l,tdnn4l,tdnn2l) dim=1280
  linear-component name=tdnn8l dim=256 $linear_opts input=Append(-3,0)
  relu-batchnorm-layer name=tdnn8 $opts input=Append(0,3) dim=1280
  linear-component name=lstm2l dim=256 $linear_opts input=Append(-3,0,3)
  fast-lstmp-layer name=lstm2-forward input=lstm2l cell-dim=1280 recurrent-projection-dim=256 non-recurrent-projection-dim=128 delay=-3 dropout-proportion=0.0 $lstm_opts
  fast-lstmp-layer name=lstm2-backward input=lstm2l cell-dim=1280 recurrent-projection-dim=256 non-recurrent-projection-dim=128 delay=3 dropout-proportion=0.0 $lstm_opts
  no-op-component name=blstm2 input=Append(lstm2-forward,lstm2-backward)
  relu-batchnorm-layer name=tdnn9 $opts input=Append(-3,0,3,tdnn8l,tdnn6l,tdnn4l) dim=1280
  linear-component name=tdnn10l dim=256 $linear_opts input=Append(-3,0)
  relu-batchnorm-layer name=tdnn10 $opts input=Append(0,3) dim=1280
  linear-component name=lstm3l dim=256 $linear_opts input=Append(-3,0,3)
  fast-lstmp-layer name=lstm3-forward input=lstm3l cell-dim=1280 recurrent-projection-dim=256 non-recurrent-projection-dim=128 delay=-3 dropout-proportion=0.0 $lstm_opts
  fast-lstmp-layer name=lstm3-backward input=lstm3l cell-dim=1280 recurrent-projection-dim=256 non-recurrent-projection-dim=128 delay=3 dropout-proportion=0.0 $lstm_opts
  no-op-component name=blstm3 input=Append(lstm3-forward,lstm3-backward)

  output-layer name=output input=blstm3  include-log-softmax=false $output_opts

  output-layer name=output-xent input=blstm3 learning-rate-factor=$learning_rate_factor $output_opts
EOF
  steps/nnet3/xconfig_to_configs.py --xconfig-file $dir/configs/network.xconfig --config-dir $dir/configs
fi

if [ $stage -le 3 ]; then
  # no need to store the egs in a shared storage because we always
  # remove them. Anyway, it takes only 5 minutes to generate them.

  use_gpu=""
  train_par="_par"
  if $gpu_exclusive; then
    train_par=""
    gpu_memory_required=
    use_gpu="wait"
  fi

  steps/nnet3/chain/e2e/train_e2e${train_par}.py --stage $train_stage \
    --cmd "$decode_cmd" \
    ${gpu_memory_required:+ --free-memory-required $gpu_memory_required} ${use_gpu:+ --use-gpu "$use_gpu"} \
    --feat.online-ivector-dir exp/nnet3/ivectors_pca3_train_nodup_spe2e_max2 \
    --feat.cmvn-opts "$cmvn_opts" \
    --chain.xent-regularize $xent_regularize \
    --chain.leaky-hmm-coefficient 0.1 \
    --chain.l2-regularize $l2_regularize \
    --chain.apply-deriv-weights false \
    --egs.dir "$common_egs_dir" \
    --egs.stage $get_egs_stage \
    --egs.opts "" \
    --trainer.dropout-schedule $dropout_schedule \
    --trainer.num-chunk-per-minibatch $minibatch_size \
    --trainer.frames-per-iter $frames_per_iter \
    --trainer.num-epochs $num_epochs \
    --trainer.optimization.momentum 0 \
    --trainer.optimization.num-jobs-initial $num_jobs_initial \
    --trainer.optimization.num-jobs-final $num_jobs_final \
    --trainer.optimization.initial-effective-lrate 0.001 \
    --trainer.optimization.final-effective-lrate 0.0001 \
    --trainer.deriv-truncate-margin 8 \
    --trainer.add-option="$add_opt" \
    --egs.chunk-left-context $chunk_left_context \
    --egs.chunk-right-context $chunk_right_context \
    --egs.chunk-left-context-initial 0 \
    --egs.chunk-right-context-final 0 \
    --trainer.optimization.shrink-value 1.0 \
    --trainer.max-param-change 2.0 \
    --cleanup.remove-egs false \
	--cleanup $cleanup \
    --feat-dir data/${train_set} \
    --tree-dir $treedir \
    --dir $dir  || exit 1;
fi

if [ $stage -le 4 ]; then
  # The reason we are using data/lang here, instead of $lang, is just to
  # emphasize that it's not actually important to give mkgraph.sh the
  # lang directory with the matched topology (since it gets the
  # topology file from the model).  So you could give it a different
  # lang directory, one that contained a wordlist and LM of your choice,
  # as long as phones.txt was compatible.

  utils/mkgraph.sh --self-loop-scale 1.0 data/lang_bpe_sw1_tg $dir $dir/graph_sw1_tg
fi

graph_dir=$dir/graph_sw1_tg

if [ $stage -le 15 ]; then
  rm $dir/.error 2>/dev/null || true
  for decode_set in eval2000 $maybe_rt03; do
      (
      steps/nnet3/decode.sh --acwt 1.0 --post-decode-acwt 10.0 \
          --nj $decode_nj --cmd "$decode_cmd" \
          --extra-left-context $extra_left_context \
          --extra-right-context $extra_right_context \
          --extra-left-context-initial 0 \
          --extra-right-context-final 0 \
          --frames-per-chunk "$frames_per_chunk_primary" \
          --online-ivector-dir exp/nnet3/ivectors_pca3_${decode_set} \
          $graph_dir data/${decode_set}_hires \
          $dir/decode_${decode_set}_sw1_tg || exit 1;
      if $has_fisher; then
          steps/lmrescore_const_arpa.sh --cmd "$decode_cmd" \
            data/lang_bpe_sw1_{tg,fsh_fg} data/${decode_set}_hires \
            $dir/decode_${decode_set}_sw1_{tg,fsh_fg} || exit 1;
      fi
      ) || touch $dir/.error &
  done
  wait
  if [ -f $dir/.error ]; then
    echo "$0: something went wrong in decoding"
    exit 1
  fi
fi

exit 0;
