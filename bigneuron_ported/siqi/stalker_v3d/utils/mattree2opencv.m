function matrtree2opencv(matrtreepath)
% Convert .mat file contains a single random forest tree to opencv
% compatible xml file
% WIP

matrtree = load(matrtreepath, 'model_alpha');
matrtree = matrtree.model_alpha;

% --- Top layer of the xml
docNode = com.mathworks.xml.XMLUtils.createDocument('opencv_storage');
opencvStorageNode = docNode.getDocumentElement();
mlrtreesNode = docNode.createElement('opencv_ml_rtrees');
opencvStorageNode.appendChild(mlrtreesNode);

% --- Second Layer under opencv_ml_rtrees
formatNode = docNode.createElement('format');
formatNode.appendChild(docNode.createTextNode('3'));
mlrtreesNode.appendChild(formatNode);

isclassifierNode = docNode.createElement('is_classifier');
if strcmp(matrtree.Method, 'regresssion')
    isclassifierNode.appendChild(docNode.createTextNode('0'));
else
    isclassifierNode.appendChild(docNode.createTextNode('1'));
end
mlrtreesNode.appendChild(isclassifierNode);

% TODO: DH, pls help me check the meaning of these parameters
% For the parameters, you may find this helpful 
% http://docs.opencv.org/2.4.9/modules/ml/doc/decision_trees.html
varallNode = docNode.createElement('var_all');
varallNode.appendChild(docNode.createTextNode('0'));
mlrtreesNode.appendChild(varallNode);

varcountNode = docNode.createElement('var_count');
varcountNode.appendChild(docNode.createTextNode('0'));
mlrtreesNode.appendChild(varcountNode);

ordvarcountNode = docNode.createElement('ord_var_count'); % Seems for regression all var should be ordered
% http://stackoverflow.com/questions/24327230/using-opencv-random-forest-for-regression
ordvarcountNode.appendChild(docNode.createTextNode('0'));
mlrtreesNode.appendChild(ordvarcountNode);

catvarcountNode = docNode.createElement('cat_var_count');
catvarcountNode.appendChild(docNode.createTextNode('0'));
mlrtreesNode.appendChild(varcountNode);

% --- Third layer training_params
trainingParamsNode = docNode.createElement('traning_params');
mlrtreesNode.appendChild(trainingParamsNode);

usesurrogatesNode = docNode.createElement('use_surrogates');
usesurrogatesNode.appendChild(docNode.createTextNode('0'));
trainingParamsNode.appendChild(usesurrogatesNode);

maxcategoriesNode = docNode.createElement('max_categories');
maxcategoriesNode.appendChild(docNode.createTextNode('0'));
trainingParamsNode.appendChild(maxcategoriesNode);

regressionaccuracyNode = docNode.createElement('regression_accuracy');
regressionaccuracyNode.appendChild(docNode.createTextNode('0'));
trainingParamsNode.appendChild(regressionaccuracyNode);

maxdepthNode = docNode.createElement('max_depth');
maxdepthNode.appendChild(docNode.createTextNode('0'));
trainingParamsNode.appendChild(maxdepthNode);

minsamplecountNode = docNode.createElement('min_sample_count');
minsamplecountNode.appendChild(docNode.createTextNode('0'));
trainingParamsNode.appendChild(minsamplecountNode);

crossvalidationfoldsNode = docNode.createElement('cross_validation_folds');
crossvalidationfoldsNode.appendChild(docNode.createTextNode('0'));
trainingParamsNode.appendChild(crossvalidationfoldsNode);

nactivevarsNode = docNode.createElement('nactive_vars');
nactivevarsNode.appendChild(docNode.createTextNode('0'));
trainingParamsNode.appendChild(nactivevarsNode);
% --- End Third layer training_params

globalvaridxNode = docNode.createElement('global_var_idx');
globalvaridxNode.appendChild(docNode.createTextNode('0'));
mlrtreesNode.appendChild(globalvaridxNode);

varidxNode = docNode.createElement('var_idx');
varidxNode.appendChild(docNode.createTextNode('0 1'));
mlrtreesNode.appendChild(varidxNode);

vartypeNode = docNode.createElement('var_type');
vartypeNode.appendChild(docNode.createTextNode('0 0 1'));
mlrtreesNode.appendChild(vartypeNode);

catofsNode = docNode.createElement('cat_ofs');
catofsNode.appendChild(docNode.createTextNode('0 0 0 0'));
mlrtreesNode.appendChild(catofsNode);

classlabelsNode = docNode.createElement('class_labels');
classlabelsNode.appendChild(docNode.createTextNode('1 2'));
mlrtreesNode.appendChild(classlabelsNode);

missingsubstNode = docNode.createElement('missing_subst');
missingsubstNode.appendChild(docNode.createTextNode('0. 0. 0.'));
mlrtreesNode.appendChild(missingsubstNode);

ooberrorNode = docNode.createElement('oob_error');
ooberrorNode.appendChild(docNode.createTextNode('0.'));
mlrtreesNode.appendChild(ooberrorNode);

ntreesNode = docNode.createElement('ntrees');
ntreesNode.appendChild(docNode.createTextNode('1'));
mlrtreesNode.appendChild(ntreesNode);

% --- Start the tree
% TODO:

% --- Extract the path without extension and make new name for the xml
[pathstr, name, ~] = fileparts(matrtreepath);
opencvrtreepath = fullfile(pathstr, strcat(name,'.xml'));

xmlwrite(opencvrtreepath, docNode);
end