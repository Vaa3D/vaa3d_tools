function saveRTree( tree2save , path)
%SAVETREE Save the regression TreeBagger object with only interested fields (Compatible with OpenCV RTree XML)
% PARAMETERS  
% tree2save : Matlab TreeBagger Object
% path : filepath of the .mat file to save

if strcmp(tree2save.Method, 'regression')
t.is_classifier = 0;
else
t.is_classifier = 1;
end
t.var_count = size(tree2save.X, 2);
% Skip 'var_all' and 'cat_var_count'
t.ord_var_count = size(tree2save.X, 2); % All variables we are using here should be order variables in regression

% Try skip the training_params for now
%t.use_surrogates = isdiag(tree2save.VarAssoc);

%t.ooberr = tree2save.oobError;
t.ntrees = tree2save.NTrees;
t.trees = tree2save.Trees;


save(path, 't');
end

