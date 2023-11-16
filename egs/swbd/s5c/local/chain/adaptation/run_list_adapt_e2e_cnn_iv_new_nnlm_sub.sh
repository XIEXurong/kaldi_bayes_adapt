cd /home/hei/works/kaldi_bayes_adapt/egs/swbd_mod/s5c

dir=exp/chain/adaptation/LHUC_e2e
for f1 in `ls $dir/ | grep "e2e_cnn_tdnn"`; do
    for f2 in `ls $dir/$f1 | grep "decode_"`; do
        rm -r $dir/$f1/$f2/score_*
        rm -r $dir/$f1/$f2/scoring
    done
    rm -r $dir/$f1/{0.raw,0.mdl,21.mdl,42.mdl,egs,cache*,configs/ref.raw}
done


. ./path.sh

for N in {5,10,20,40}; do
    cp -r data/eval2000_e2e_hires data/eval2000_e2e_hires_spk_sub${N}
    mv data/eval2000_e2e_hires_spk_sub${N}/feats.scp data/eval2000_e2e_hires_spk_sub${N}/feats_ori.scp
    perl local/chain/adaptation/utt2spk_split_everyN.pl data/eval2000_e2e_hires_spk/utt2spk $N | grep "_sub1$" > data/eval2000_e2e_hires_spk_sub${N}/utt2spk
    perl local/chain/adaptation/find_pdf.pl data/eval2000_e2e_hires_spk/align1.pdf data/eval2000_e2e_hires_spk_sub${N}/utt2spk > data/eval2000_e2e_hires_spk_sub${N}/align1.pdf
    perl local/chain/adaptation/segment2id.pl data/eval2000_e2e_hires_spk_sub${N}/utt2spk data/eval2000_e2e_hires_spk_sub${N}/align1.pdf data/eval2000_e2e_hires_spk_sub${N}/num_spk > data/eval2000_e2e_hires_spk_sub${N}/spk
    perl local/chain/adaptation/pdf2ark_simple.pl data/eval2000_e2e_hires_spk_sub${N}/spk > data/eval2000_e2e_hires_spk_sub${N}/spk.ark
    analyze-counts --binary=false ark:data/eval2000_e2e_hires_spk_sub${N}/spk data/eval2000_e2e_hires_spk_sub${N}/spk_count

    paste-feats scp:data/eval2000_e2e_hires_spk_sub${N}/feats_ori.scp ark:data/eval2000_e2e_hires_spk_sub${N}/spk.ark ark,scp:data/eval2000_e2e_hires_spk_sub${N}/feats.ark,data/eval2000_e2e_hires_spk_sub${N}/feats.scp
    steps/compute_cmvn_stats.sh data/eval2000_e2e_hires_spk_sub${N}
    mv data/eval2000_e2e_hires_spk_sub${N}/utt2spk data/eval2000_e2e_hires_spk_sub${N}/utt2spk_ori
    perl local/chain/adaptation/find_pdf.pl data/eval2000_e2e_hires_spk_sub${N}/utt2spk_ori data/eval2000_e2e_hires_spk_sub${N}/feats.scp > data/eval2000_e2e_hires_spk_sub${N}/utt2spk
    mv data/eval2000_e2e_hires_spk_sub${N}/text data/eval2000_e2e_hires_spk_sub${N}/text_all
    perl local/chain/adaptation/find_pdf.pl data/eval2000_e2e_hires_spk_sub${N}/text_all data/eval2000_e2e_hires_spk_sub${N}/feats.scp > data/eval2000_e2e_hires_spk_sub${N}/text
    perl utils/data/get_utt2dur.sh data/eval2000_e2e_hires_spk_sub${N}
    mv data/eval2000_e2e_hires_spk_sub${N}/utt2dur data/eval2000_e2e_hires_spk_sub${N}/utt2dur_all
    perl local/chain/adaptation/find_pdf.pl data/eval2000_e2e_hires_spk_sub${N}/utt2dur_all data/eval2000_e2e_hires_spk_sub${N}/feats.scp > data/eval2000_e2e_hires_spk_sub${N}/utt2dur
    mv data/eval2000_e2e_hires_spk_sub${N}/spk2utt data/eval2000_e2e_hires_spk_sub${N}/spk2utt_all
    perl local/chain/adaptation/utt2spk_to_spk2utt.pl < data/eval2000_e2e_hires_spk_sub${N}/utt2spk > data/eval2000_e2e_hires_spk_sub${N}/spk2utt
done

for N in {5,10,20,40}; do
    cp -r data/rt03_e2e_hires data/rt03_e2e_hires_spk_sub${N}
    mv data/rt03_e2e_hires_spk_sub${N}/feats.scp data/rt03_e2e_hires_spk_sub${N}/feats_ori.scp
    perl local/chain/adaptation/utt2spk_split_everyN.pl data/rt03_e2e_hires_spk/utt2spk $N | grep "_sub1$" > data/rt03_e2e_hires_spk_sub${N}/utt2spk
    perl local/chain/adaptation/find_pdf.pl data/rt03_e2e_hires_spk/align1.pdf data/rt03_e2e_hires_spk_sub${N}/utt2spk > data/rt03_e2e_hires_spk_sub${N}/align1.pdf
    perl local/chain/adaptation/segment2id.pl data/rt03_e2e_hires_spk_sub${N}/utt2spk data/rt03_e2e_hires_spk_sub${N}/align1.pdf data/rt03_e2e_hires_spk_sub${N}/num_spk > data/rt03_e2e_hires_spk_sub${N}/spk
    perl local/chain/adaptation/pdf2ark_simple.pl data/rt03_e2e_hires_spk_sub${N}/spk > data/rt03_e2e_hires_spk_sub${N}/spk.ark
    analyze-counts --binary=false ark:data/rt03_e2e_hires_spk_sub${N}/spk data/rt03_e2e_hires_spk_sub${N}/spk_count

    paste-feats scp:data/rt03_e2e_hires_spk_sub${N}/feats_ori.scp ark:data/rt03_e2e_hires_spk_sub${N}/spk.ark ark,scp:data/rt03_e2e_hires_spk_sub${N}/feats.ark,data/rt03_e2e_hires_spk_sub${N}/feats.scp
    steps/compute_cmvn_stats.sh data/rt03_e2e_hires_spk_sub${N}
    mv data/rt03_e2e_hires_spk_sub${N}/utt2spk data/rt03_e2e_hires_spk_sub${N}/utt2spk_ori
    perl local/chain/adaptation/find_pdf.pl data/rt03_e2e_hires_spk_sub${N}/utt2spk_ori data/rt03_e2e_hires_spk_sub${N}/feats.scp > data/rt03_e2e_hires_spk_sub${N}/utt2spk
    mv data/rt03_e2e_hires_spk_sub${N}/text data/rt03_e2e_hires_spk_sub${N}/text_all
    perl local/chain/adaptation/find_pdf.pl data/rt03_e2e_hires_spk_sub${N}/text_all data/rt03_e2e_hires_spk_sub${N}/feats.scp > data/rt03_e2e_hires_spk_sub${N}/text
    perl utils/data/get_utt2dur.sh data/rt03_e2e_hires_spk_sub${N}
    mv data/rt03_e2e_hires_spk_sub${N}/utt2dur data/rt03_e2e_hires_spk_sub${N}/utt2dur_all
    perl local/chain/adaptation/find_pdf.pl data/rt03_e2e_hires_spk_sub${N}/utt2dur_all data/rt03_e2e_hires_spk_sub${N}/feats.scp > data/rt03_e2e_hires_spk_sub${N}/utt2dur
    mv data/rt03_e2e_hires_spk_sub${N}/spk2utt data/rt03_e2e_hires_spk_sub${N}/spk2utt_all
    perl local/chain/adaptation/utt2spk_to_spk2utt.pl < data/rt03_e2e_hires_spk_sub${N}/utt2spk > data/rt03_e2e_hires_spk_sub${N}/spk2utt
done


for decode_set in eval2000 rt03; do
    lab_dir=exp/chain/e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi/decode_${decode_set}_sw1_fsh_fg_pytorch_transformer_20best_0.8/1BEST_fst/score_10_0.0
    for N in {5,10,20,40}; do
        mkdir -p ${lab_dir}_sub${N}
        cat ${lab_dir}/fst.*.scp | perl local/chain/adaptation/find_pdf_stdin.pl data/${decode_set}_e2e_hires_spk_sub${N}/feats.scp > ${lab_dir}_sub${N}/fst.1.scp
        echo "1" > ${lab_dir}_sub${N}/num_jobs
    done
done



## LHUC adapt

# 1best

for subN in _sub5 _sub10 _sub20 _sub40; do
    bash local/chain/adaptation/LHUC/LHUC_adaptation_e2e.sh --egs_opts "--num_utts_subset 100" \
     --baseline e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi \
     --adapt-ivector-dir exp/nnet3/ivectors_eval2000 \
     --test-ivector-dir exp/nnet3/ivectors_eval2000 \
     --LM-path data/lang_ --pre_out_layer prefinal-l --pre_out_dim 256 \
     --adapted-layer "cnn1 tdnnf7 tdnnf8 tdnnf9 tdnnf10 tdnnf11 tdnnf12" \
     --layer-dim "2560 1536 1536 1536 1536 1536 1536" \
     --input-config "component-node name=idct component=idct input=feature1" \
     --act "Sig" --tag "_eval2000${subN}_e2ehires_transformer" \
     --epoch-num 7 --lr1 0.01 --lr2 0.01 --param-init 0.0 \
     eval2000_e2e_hires_spk${subN} \
     exp/chain/e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi/decode_eval2000_sw1_fsh_fg_pytorch_transformer_20best_0.8/1BEST_fst/score_10_0.0${subN} \
     eval2000_hires_spk

    for decode_set in eval2000_hires_spk; do
        for lm in tg fsh_fg; do
            dir=exp/chain/adaptation/LHUC_e2e/e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi_LHUC_e2e_eval2000${subN}_e2ehires_transformer_adaptlayer7_actSig_epoch7_lr10.01_lr20.01/decode_${decode_set}_sw1_${lm}
            bash compute_score.sh $dir >> $dir/../scoring_all
        done
    done

    bash local/pytorchnn/run_nnlm_decode_mod.sh --use_gpu true --use-nbest true \
     --LM-path data/lang_ --LM sw1_fsh_fg --other_opt '--gpu_wait true --limit_num_gpus_cmd "\"\""' \
     "eval2000_hires_spk" exp/chain/adaptation/LHUC_e2e/e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi_LHUC_e2e_eval2000${subN}_e2ehires_transformer_adaptlayer7_actSig_epoch7_lr10.01_lr20.01
done


for subN in _sub5 _sub10 _sub20 _sub40; do
    bash local/chain/adaptation/LHUC/LHUC_adaptation_e2e.sh --egs_opts "--num_utts_subset 100" \
     --baseline e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi \
     --adapt-ivector-dir exp/nnet3/ivectors_rt03 \
     --test-ivector-dir exp/nnet3/ivectors_rt03 \
     --LM-path data/lang_ --pre_out_layer prefinal-l --pre_out_dim 256 \
     --adapted-layer "cnn1 tdnnf7 tdnnf8 tdnnf9 tdnnf10 tdnnf11 tdnnf12" \
     --layer-dim "2560 1536 1536 1536 1536 1536 1536" \
     --input-config "component-node name=idct component=idct input=feature1" \
     --act "Sig" --tag "_rt03${subN}_e2ehires_transformer" \
     --epoch-num 7 --lr1 0.01 --lr2 0.01 --param-init 0.0 \
     rt03_e2e_hires_spk${subN} \
     exp/chain/e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi/decode_rt03_sw1_fsh_fg_pytorch_transformer_20best_0.8/1BEST_fst/score_10_0.0${subN} \
     rt03_hires_spk

    for decode_set in rt03_hires_spk; do
        for lm in tg fsh_fg; do
            dir=exp/chain/adaptation/LHUC_e2e/e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi_LHUC_e2e_rt03${subN}_e2ehires_transformer_adaptlayer7_actSig_epoch7_lr10.01_lr20.01/decode_${decode_set}_sw1_${lm}
            bash compute_score.sh $dir >> $dir/../scoring_all
        done
    done

    bash local/pytorchnn/run_nnlm_decode_mod.sh --use_gpu true --use-nbest true \
     --LM-path data/lang_ --LM sw1_fsh_fg --other_opt '--gpu_wait true --limit_num_gpus_cmd "\"\""' \
     "rt03_hires_spk" exp/chain/adaptation/LHUC_e2e/e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi_LHUC_e2e_rt03${subN}_e2ehires_transformer_adaptlayer7_actSig_epoch7_lr10.01_lr20.01
done


## BLHUC adapt

# 1best

for subN in _sub5 _sub10 _sub20 _sub40; do
    bash local/chain/adaptation/LHUC/BLHUC_adaptation_e2e.sh --egs_opts "--num_utts_subset 100" \
     --baseline e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi \
     --adapt-ivector-dir exp/nnet3/ivectors_eval2000 \
     --test-ivector-dir exp/nnet3/ivectors_eval2000 \
     --LM-path data/lang_ --pre_out_layer prefinal-l --pre_out_dim 256 \
     --adapted-layer "cnn1 tdnnf7 tdnnf8 tdnnf9 tdnnf10 tdnnf11 tdnnf12" \
     --layer-dim "2560 1536 1536 1536 1536 1536 1536" \
     --KL-scale "0.0001 1 1 1 1 1 1" \
     --input-config "component-node name=idct component=idct input=feature1" \
     --act "Sig" --tag "_eval2000${subN}_e2ehires_transformer" \
     --epoch-num 7 --lr1 0.01 --lr2 0.01 --param-mean-init 0.0 --param-std-init 1.0 \
     --prior-mean "0.0 0.0" --prior-std "1.0 1.0" \
     eval2000_e2e_hires_spk${subN} \
     exp/chain/e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi/decode_eval2000_sw1_fsh_fg_pytorch_transformer_20best_0.8/1BEST_fst/score_10_0.0${subN} \
     eval2000_hires_spk

    for decode_set in eval2000_hires_spk; do
        for lm in tg fsh_fg; do
            dir=exp/chain/adaptation/LHUC_e2e/e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi_BLHUC_e2e_eval2000${subN}_e2ehires_transformer_adaptlayer7_actSig_epoch7_lr10.01_lr20.01/decode_${decode_set}_sw1_${lm}
            bash compute_score.sh $dir >> $dir/../scoring_all
        done
    done

    bash local/pytorchnn/run_nnlm_decode_mod.sh --use_gpu true --use-nbest true \
     --LM-path data/lang_ --LM sw1_fsh_fg --other_opt '--gpu_wait true --limit_num_gpus_cmd "\"\""' \
     "eval2000_hires_spk" exp/chain/adaptation/LHUC_e2e/e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi_BLHUC_e2e_eval2000${subN}_e2ehires_transformer_adaptlayer7_actSig_epoch7_lr10.01_lr20.01
done


for subN in _sub5 _sub10 _sub20 _sub40; do
    bash local/chain/adaptation/LHUC/BLHUC_adaptation_e2e.sh --egs_opts "--num_utts_subset 100" \
     --baseline e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi \
     --adapt-ivector-dir exp/nnet3/ivectors_rt03 \
     --test-ivector-dir exp/nnet3/ivectors_rt03 \
     --LM-path data/lang_ --pre_out_layer prefinal-l --pre_out_dim 256 \
     --adapted-layer "cnn1 tdnnf7 tdnnf8 tdnnf9 tdnnf10 tdnnf11 tdnnf12" \
     --layer-dim "2560 1536 1536 1536 1536 1536 1536" \
     --KL-scale "0.0001 1 1 1 1 1 1" \
     --input-config "component-node name=idct component=idct input=feature1" \
     --act "Sig" --tag "_rt03${subN}_e2ehires_transformer" \
     --epoch-num 7 --lr1 0.01 --lr2 0.01 --param-mean-init 0.0 --param-std-init 1.0 \
     --prior-mean "0.0 0.0" --prior-std "1.0 1.0" \
     rt03_e2e_hires_spk${subN} \
     exp/chain/e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi/decode_rt03_sw1_fsh_fg_pytorch_transformer_20best_0.8/1BEST_fst/score_10_0.0${subN} \
     rt03_hires_spk

    for decode_set in rt03_hires_spk; do
        for lm in tg fsh_fg; do
            dir=exp/chain/adaptation/LHUC_e2e/e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi_BLHUC_e2e_rt03${subN}_e2ehires_transformer_adaptlayer7_actSig_epoch7_lr10.01_lr20.01/decode_${decode_set}_sw1_${lm}
            bash compute_score.sh $dir >> $dir/../scoring_all
        done
    done

    bash local/pytorchnn/run_nnlm_decode_mod.sh --use_gpu true --use-nbest true \
     --LM-path data/lang_ --LM sw1_fsh_fg --other_opt '--gpu_wait true --limit_num_gpus_cmd "\"\""' \
     "rt03_hires_spk" exp/chain/adaptation/LHUC_e2e/e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi_BLHUC_e2e_rt03${subN}_e2ehires_transformer_adaptlayer7_actSig_epoch7_lr10.01_lr20.01
done


result_file=exp/chain/adaptation/LHUC_e2e/results_e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi_BLHUC_e2e_transformer_sub
> $result_file
for decode_set in eval2000 rt03; do
    echo "### ${decode_set}" >> $result_file
    for subN in _sub5 _sub10 _sub20 _sub40 ""; do
        echo "## ${subN}" >> $result_file
        dir=exp/chain/adaptation/LHUC_e2e/e2e_cnn_tdnnf_1a_iv_bi_mmice_specaugkaldi_BLHUC_e2e_${decode_set}${subN}_e2ehires_transformer_adaptlayer7_actSig_epoch7_lr10.01_lr20.01
        tail -n 3 $dir/decode_${decode_set}_hires_spk_sw1_fsh_fg_pytorch_transformer_20best_0.8/scoring_all >> $result_file
    done
done