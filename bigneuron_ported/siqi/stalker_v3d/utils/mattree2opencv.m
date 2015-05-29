% Author: SQ Liu

function matrtree2opencv(matrtreepath)
% Convert .mat file contains a single random forest tree to opencv
% compatible xml file
% WIP
addpath('.');

matrtree = load(matrtreepath, 't');
matrtree = matrtree.t;

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
isclassifierNode.appendChild(docNode.createTextNode(num2str(matrtree.is_classifier)));
mlrtreesNode.appendChild(isclassifierNode);

% TODO: DH, pls help me check the meaning of these parameters
% For the parameters, you may find this helpful 
% http://docs.opencv.org/2.4.9/modules/ml/doc/decision_trees.html
varallNode = docNode.createElement('var_all');
varallNode.appendChild(docNode.createTextNode(num2str(matrtree.var_count+1)));
mlrtreesNode.appendChild(varallNode);

varcountNode = docNode.createElement('var_count');
varcountNode.appendChild(docNode.createTextNode(num2str(matrtree.var_count)));
mlrtreesNode.appendChild(varcountNode);

ordvarcountNode = docNode.createElement('ord_var_count'); % Seems for regression all var should be ordered
% http://stackoverflow.com/questions/24327230/using-opencv-random-forest-for-regression
ordvarcountNode.appendChild(docNode.createTextNode(num2str(matrtree.var_count+1)));
mlrtreesNode.appendChild(ordvarcountNode);

catvarcountNode = docNode.createElement('cat_var_count');
catvarcountNode.appendChild(docNode.createTextNode(num2str(0)));
mlrtreesNode.appendChild(catvarcountNode);

% --- Third layer training_params: Keep them as 0 for now, since we only
% use the saved file for prediction making
trainingParamsNode = docNode.createElement('traning_params');
mlrtreesNode.appendChild(trainingParamsNode);

usesurrogatesNode = docNode.createElement('use_surrogates');
usesurrogatesNode.appendChild(docNode.createTextNode('0'));
trainingParamsNode.appendChild(usesurrogatesNode);

maxcategoriesNode = docNode.createElement('max_categories');
maxcategoriesNode.appendChild(docNode.createTextNode('10'));
trainingParamsNode.appendChild(maxcategoriesNode);

regressionaccuracyNode = docNode.createElement('regression_accuracy');
regressionaccuracyNode.appendChild(docNode.createTextNode('0'));
trainingParamsNode.appendChild(regressionaccuracyNode);

maxdepthNode = docNode.createElement('max_depth');
maxdepthNode.appendChild(docNode.createTextNode('0'));
trainingParamsNode.appendChild(maxdepthNode);

minsamplecountNode = docNode.createElement('min_sample_count');
minsamplecountNode.appendChild(docNode.createTextNode('1'));
trainingParamsNode.appendChild(minsamplecountNode);

crossvalidationfoldsNode = docNode.createElement('cross_validation_folds');
crossvalidationfoldsNode.appendChild(docNode.createTextNode('0'));
trainingParamsNode.appendChild(crossvalidationfoldsNode);

nactivevarsNode = docNode.createElement('nactive_vars');
nactivevarsNode.appendChild(docNode.createTextNode('0'));
trainingParamsNode.appendChild(nactivevarsNode);
% --- End Third layer training_params

globalvaridxNode = docNode.createElement('global_var_idx');
globalvaridxNode.appendChild(docNode.createTextNode('1'));
mlrtreesNode.appendChild(globalvaridxNode);

varidxNode = docNode.createElement('var_idx');
varidx = 0:matrtree.var_count-1; % var_idx starts from 0 in OPENCV
matstr = mat2str(varidx);
matstr = strrep(matstr, '[', '');
matstr = strrep(matstr, ']', '');
matstr = strrep(matstr, ';', sprintf('\n'));
varidxNode.appendChild(docNode.createTextNode(matstr));
mlrtreesNode.appendChild(varidxNode);

vartypeNode = docNode.createElement('var_type');
vartypeNode.appendChild(docNode.createTextNode(num2str(zeros(1,matrtree.var_count), '%d ')));
mlrtreesNode.appendChild(vartypeNode);

catofsNode = docNode.createElement('cat_ofs');
catofsNode.appendChild(docNode.createTextNode(num2str(zeros(1, matrtree.var_count * 2), '%.1f '))); % Not sure what it is for now
mlrtreesNode.appendChild(catofsNode);

% This might not be used for regresssion trees
%classlabelsNode = docNode.createElement('class_labels');
%classlabelsNode.appendChild(docNode.createTextNode('1 2')); 
%mlrtreesNode.appendChild(classlabelsNode);

missingsubstNode = docNode.createElement('missing_subst');
missingsubstNode.appendChild(docNode.createTextNode(num2str(zeros(1,matrtree.var_count), '%.1f ')));
mlrtreesNode.appendChild(missingsubstNode);

% Out-of-bag permutations were not saved. Run with 'oobvarimp' set to 'on'.
% in matlab 
% ooberrorNode = docNode.createElement('oob_error'); 
% ooberrorNode.appendChild(docNode.createTextNode('0.')); % Not saved
% mlrtreesNode.appendChild(ooberrorNode);

ntreesNode = docNode.createElement('ntrees');
ntreesNode.appendChild(docNode.createTextNode(num2str(matrtree.ntrees))); % Done
mlrtreesNode.appendChild(ntreesNode);

treesNode = docNode.createElement('trees');
% --- Start the tree
for i = 1 : numel(matrtree.trees)
    treeNode = docNode.createElement('_');
    nodesNode = docNode.createElement('nodes');
    
    % Things get a bit complicated here, matlab saves the tree in a
    % different manner regarding the openCV
    % Matlab saves the topology by stating the node IDs in an array
    % According to the implementation of writeTree in OpenCV - 
    % https://github.com/Itseez/opencv/blob/d7cb99254eb4e545bfe9c2d6bed1a2adbe30a547/modules/ml/src/tree.cpp#L1649
    % It save the tree nodes following the depth-first order

    % Build Graph G compatible with graphtraverse
    % graphtraverse doc: http://au.mathworks.com/help/bioinfo/ref/graphtraverse.html
    curtree = matrtree.trees{i};
    
    [g, s] = tree2graph(curtree);
    [d dt ft pred] = dfs(sparse(g), s); % should work for well-built trees
    
    [~, I] = sort(dt);
    sorteddepth = d(I);
    sortedvalue = curtree.NodeMean(I);
    sortedbranchflag = curtree.IsBranchNode(I);
    sortedfeatidx = curtree.CutPredictor(I);
    sortederror = curtree.NodeError(I);
    sortedcutpoints = curtree.CutPoint(I);
    
    for j = 1 : matrtree.trees{i}.NumNodes
        nodeNode = docNode.createElement('_');
        
        % Depth
        depthnode = docNode.createElement('depth');
        depthnode.appendChild(docNode.createTextNode(num2str(sorteddepth(j))));
        
        % Value - According to opencv doc: 
        % A class label in case of classification or estimated function value in case of regression.
        valuenode = docNode.createElement('value');
        valuenode.appendChild(docNode.createTextNode(num2str(sortedvalue(j), '%f ')));
        
        % Split - Only branching node has a split
        if sortedbranchflag(j)
            splitsnode = docNode.createElement('split');
            splitnode = docNode.createElement('_');
            
            splitvarnode = docNode.createElement('var');
            idxstr = strrep(sortedfeatidx(j), 'x', '');
            splitvarnode.appendChild(docNode.createTextNode(num2str(str2num(idxstr{:})-1, '%d ')));
            
            % The definition of quality may be the squared error for
            % regression in OpenCV - http://answers.opencv.org/question/566/how-is-decision-tree-split-quality-computed/
            splitqualitynode = docNode.createElement('quality'); 
            splitqualitynode.appendChild(docNode.createTextNode(num2str(sortederror(j), '%f ')));
            
            splitlenode = docNode.createElement('le'); 
            splitlenode.appendChild(docNode.createTextNode(num2str(sortedcutpoints(j), '%f ')));
            
            splitnode.appendChild(splitvarnode);
            splitnode.appendChild(splitqualitynode);
            splitnode.appendChild(splitlenode);
            splitsnode.appendChild(splitnode);
        end
        
        nodeNode.appendChild(splitsnode);
        nodeNode.appendChild(depthnode);
        nodeNode.appendChild(valuenode);
        nodesNode.appendChild(nodeNode);
    end
    treeNode.appendChild(nodesNode);
    treesNode.appendChild(treeNode);
end

mlrtreesNode.appendChild(treesNode);

% --- Extract the path without extension and make new name for the xml
[pathstr, ~, ~] = fileparts(mfilename);
[~, name, ~] = fileparts(matrtreepath);
opencvrtreepath = fullfile(pathstr, strcat(name,'.xml'));

xmlwrite(opencvrtreepath, docNode);
end

function [g, s] = tree2graph(tree)
% Convert the tree node index to a directed graph matrix with size N*N
% 
% [g, s] = tree2graph(tree)
% 
% It uses the parent index stored in the compact tree
% input - tree: a CompactTree object
% output - g: a N*N matrix 
%          s: the source index

p = tree.Parent;
g = zeros(tree.NumNodes, tree.NumNodes);
s = find(p==0); % Find the source index
p(s) = [];
p = [p, (2:numel(tree.Parent))'];
idx = sub2ind(size(g), p(:,1), p(:,2));
g(idx) = 1;
end