function LTClassifier = Learn_CLRG_Classifier_3D(fn)

% learn the CLRG on the 3D dataset

% author: Lin Gu

[patch_stack,hdc_stack,patch_idx,patch_stack1] = collect_fragment_3D(fn);

[centre_hd_seed_stack,p1_stack,l2_stack] = cluster_3Dseed(patch_stack,hdc_stack);

[CLRG_tree,node_classifier] = learn_CLRG_3Dseed_v2(patch_stack1,centre_hd_seed_stack,patch_idx,...
    l2_stack,hdc_stack);


LTClassifier.CLRG_tree = CLRG_tree;

LTClassifier.node_classifier = node_classifier;

LTClassifier.pot = p1_stack;