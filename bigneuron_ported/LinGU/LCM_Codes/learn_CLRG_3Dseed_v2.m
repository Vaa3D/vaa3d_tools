function [CLRG_tree,node_classifier] = learn_CLRG_3Dseed_v2(localfeature,hds,...
    sample_idx,l2_stack,hdc_stack)


% learn the CLRG based classifier from the input 3D fragments

% use the sorounding cube to 

CLRG_tree = build_CLRG_hd(hds);

sample_idx(:,5) = l2_stack;

node_classifier = train_RF_3Dseed_v2(CLRG_tree,localfeature,...
    hdc_stack,sample_idx);


