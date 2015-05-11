function train()
% Train neurostalker based on groundtruth extracted from swf trees and input from imagesstacks
% This version disregard the memory cost of loading training cases
tic;
clc; clear all; close all;

%------ PARA
% TEST
TESTSIZE        = 1;
TESTSTARTLOC    = [[89, 363, 30]];
TESTSTARTDIR    = [[-0.9278, 0.1916]];
NTESTSTEP       = 100;
STEPSIZE        = 1;
NOISETYPE       = 'original'; % The noise added to vision blocks: original, gauss, salt_pepper
% General Learning
DEBUG           = true; % DEBUG=true will only train models with 1000 cases
FRAMEWORK       = 'PUFFER' % The framework used for training/walking the neurostalker: 'NORMAL'/PUFFER
PREFIX          = 'OP_';
CACHETRAINDATA  = true;
CACHETRAINMODEL = false;
RUNTEST         = false;
% Random Forest 
RF.NTREE        = 200;
RF.MTRY         = 140;
% Neural Network
NN.NHLAYER      = 2;
NN.NHNEURON     = 13 * 5;
NN.BATCHSIZE    = 1000;
NN.SAEEPOCH     = 50;
NN.NNEPOCH      = 100;
NN.ZEROMASK     = 0.5;
% ENDPARA

curdir = fileparts(mfilename('fullpath'));
datadir = fullfile(curdir, '..', 'data', 'input', 'preprocessed');
addpath(fullfile(curdir, '..', 'utils'));
addpath(genpath(fullfile(curdir, '..', 'lib', 'DeepLearnToolbox')));

nsbj = numel(dir([datadir, [filesep, '*.mat']]));

disp(['cur dir:', curdir]);
disp(['data dir:', datadir]);
disp(dir([datadir, [filesep, PREFIX, '*.mat']]));
disp(sprintf('nsbj: %d\n', nsbj));


% Make the Train X and Y
% Load the cached Train X and Y matrix if possible
if CACHETRAINDATA && exist(fullfile(datadir, 'traincache.mat'))
    fcache = load(fullfile(datadir, 'traincache.mat'));
    train_x = fcache.train_x;
    train_y = fcache.train_y;
    fprintf('Loaded train X(%d*%d) and Y(%d,%d).', size(train_x, 1),...
                                                   size(train_x, 2),...
                                                   size(train_y, 1),...
                                                   size(train_y, 2));
    clearvars fcache; 
else
    % Read in the robots 
    % Only deal with low memory use
    for i = 1 : nsbj-TESTSIZE
        fsbj = load(fullfile(datadir, strcat(PREFIX, num2str(nsbj), '.mat')), 'sbj');   
        ltrainrobot{i} = fsbj.sbj;
    end

    %ltrainrobot = sbj{1 : numel(sbj)-TESTSIZE};
    %ltestrobot = sbj{numel(sbj)-TESTSIZE+1 : end};

    % Count the train vbox
    ntrain = 0;
    for r = 1 : numel(ltrainrobot)
        ntrain = ntrain + numel(ltrainrobot(r));
    end

    % Count the test vbox
    %ntest = 0;
    %for r = 1 : numel(ltestrobot)
    %    ntest = ntest + numel(ltestrobot(r));
    %end

    vboxsize = ltrainrobot{1}.vboxsize;
    train_x = zeros(ntrain, vboxsize^3);
    row = 1;

    for i = 1 : numel(ltrainrobot) - TESTSIZE
        for j = 1 : numel(ltrainrobot{i}.lrobot)
            disp(sprintf('Reading train matrix row %d\n', row));
            vb = ltrainrobot{i}.lrobot(j).visionbox.(NOISETYPE);

            if strcmp(FRAMEWORK, 'NORMAL')
                if ltrainrobot{i}.lrobot(j).fissure == 1 % Only use the vboxes not at branching locations
                    continue;
                end
            end

        	train_x(row, :) = vb(:); % Vectorise the visionbox voxels

            if strcmp(FRAMEWORK, 'NORMAL')  
                % For NORMAL, the ground truth is the spherical coordinates of the output directions
                train_y(row, 1) = ltrainrobot{i}.lrobot(j).next_th;
                train_y(row, 2) = ltrainrobot{i}.lrobot(j).next_phi;
            elseif strcmp(FRAMEWORK, 'PUFFER')
                % For PUFFER, the ground truth is the sampled probability matching the
                % uniformly distributed spherical directions
                train_y(row, :) = ltrainrobot{i}.lrobot(j).prob';
            else
                raise Exception(sprintf('FRAMEWORK: %s not defined\n', FRAMEWORK));
            end

            row = row + 1;
        end
    end

    if strcmp(FRAMEWORK, 'NORMAL')
        train_x(row:end, :) = []; % Delete the redundent rows not used because of the branching
        train_y(row:end, :) = [];
    end

    if CACHETRAINDATA % Cache the X and Y matrix
        save(fullfile(datadir, 'traincache.mat'), 'train_x', 'train_y');
    end

    clearvars sbj;
end

if DEBUG == true
    train_x = train_x(1 : 1000, :);
    train_y = train_y(1 : 1000, :);
end

% Train Neural Stalker Model 
if CACHETRAINMODEL && exist(fullfile(datadir, 'modelcache.mat')) % Skip Training
    if strcmp(FRAMEWORK, 'NORMAL')
        fmodel = load(fullfile(datadir, 'modelcache.mat'), 'rf_th', 'rf_phi');
        rf_th = fmodel.rf_th;
        rf_phi = fmodel.rf_phi;
    elseif strcmp(FRAMEWORK, 'PUFFER')        
        fmodel = load(fullfile(datadir, 'modelcache.mat'), 'nn');
        nn = fmodel.nn; 
    end
else
    if strcmp(FRAMEWORK, 'NORMAL')
        options = statset('UseParallel', 'Always');
        
        % When (2*pi > theta > pi) we invert the ground truth vector to its
        % inversed direction
        orith = train_y(:, 1);
        oriphi = train_y(:, 2);
        [gtx, gty, gtz] = sph2cart(orith, oriphi, ones(numel(orith, 1)));
        [invth, invphi] = cart2sph(-gtx, -gty, -gtz);
        gtth = orith;
        gtphi = oriphi;
        gtth(orith > pi) = invth(orith > pi);
        gtphi(orith > pi) = invphi(orith > pi);

        % Train Theta
        disp('Start to train RF THETA...');
        rf_th = TreeBagger(RF.NTREE, train_x, gtth,...
                                'Method', 'regression', 'NVarToSample',...
                                 RF.MTRY, 'NPrint', true,'Options',options); 

        % Train Phi
        disp('Start to train RF PHI...');
        rf_phi = TreeBagger(RF.NTREE, train_x, gtphi,... 
                                 'Method', 'regression', 'NVarToSample',...
                                  RF.MTRY, 'NPrint', true,'Options',options);

        if CACHETRAINMODEL 
            save(fullfile(datadir, 'modelcache.mat'), 'rf_th', 'rf_phi');
        end

    elseif strcmp(FRAMEWORK, 'PUFFER')
        % Remove the data which can be not fit into the batchsize
        ntrain = size(train_x, 1);
        nremainder = mod(ntrain, NN.BATCHSIZE);
        train_x = train_x(1 : end - nremainder, :);
        train_y = train_y(1 : end - nremainder, :);
        fprintf('ntrain after elliminating: %d\n', size(train_x, 1));

        nn = trainNN(train_x, train_y, NN);
        %nn = nnff(nn, train_x);
        %R = corr(nn.a{end}(:), train_y(:));
        %fprintf('The training correlations is %f\n', R);
        if CACHETRAINMODEL 
            save(fullfile(datadir, 'modelcache.mat'), 'nn');
        end
    end
end


% % Test RF by walking 
% Read in the test subject
if RUNTEST == false
    return
end

img3dctr = 1; % counter for img3d cell array
for i = nsbj - TESTSIZE + 1 : nsbj 
    fsbj = load(fullfile(datadir, strcmp(PREFIX, num2str(nsbj), '.mat')), 'sbj', 'img3d'); 
    ltestrobot{img3dctr} = fsbj.sbj;
    img3d{img3dctr} = fsbj.img3d;
    img3dctr = img3dctr + 1;
end


for r = 1 : numel(ltestrobot)
    figure
    hold on
    showbox(img3d{r}.original, 0.1);
 	testimg = img3d{r}.(NOISETYPE);
 
 	% Start Location
 	curnode.x = TESTSTARTLOC(r, 1);
 	curnode.y = TESTSTARTLOC(r, 2);
 	curnode.z = TESTSTARTLOC(r, 3);
    fprintf('Starting at (%f, %f, %f)', curnode.x, curnode.y, curnode.z);
 
 	% Start Direction
 	curnode.prev_th = TESTSTARTDIR(r, 1);
 	curnode.prev_phi = TESTSTARTDIR(r, 2);
    fprintf('Starting from direction (%f, %f)\n', curnode.prev_th, curnode.prev_phi);

    % Iterate each step of the prediction
    for s = 1 : NTESTSTEP 
        % Extract the vbox of the stalker
        vbox = extractbox(testimg, ltestrobot{r}.vboxsize,...
                          curnode.x, curnode.y, curnode.z,...
                          ltestrobot{r}.zerosize);

        th = rf_th.predict(vbox(:)');
        phi = rf_phi.predict(vbox(:)');

        % Calculate the cosine of the forward and backward
        % Larger cosine means smaller angle
        % Choose the direction with the small angle based on the assumption that turning angles larger than pi/2
        % is not common in bio systems
        dcosforward = sphveccos(th, phi, curnode.prev_th, curnode.prev_phi);
        dcosbackward = sphveccos(th+pi, -phi, curnode.prev_th, curnode.prev_phi);

        if dcosbackward > dcosforward
            [x, y, z] = sph2cart(th, phi);
            [th, phi] = cart2sph(-x, -y, -z);
        end

        % Move one step
        [dx, dy, dz] = sph2cart(th, phi, STEPSIZE);
        nextnode.x = curnode.x + dx;
        nextnode.y = curnode.y + dy;
        nextnode.z = curnode.z + dz;
        nextnode.prev_th = th;
        nextnode.prev_phi = phi;

        fprintf('Step %d: Move to direction (%f, %f) - (%f, %f, %f)\n', s, th, phi, dx, dy, dz);
        line([curnode.x, nextnode.x], [curnode.y, nextnode.y], [curnode.z, nextnode.z], 'Color','k', 'LineWidth',5);
        curnode = nextnode;
        drawnow
    end
    hold off
end

end


function nn = trainNN(trainx, trainy, NN)
% Wrapper function for training stacked denoising autoencoders and a softmax output NN

trainx = double(trainx);
trainy = double(trainy);

% Setup SAE
rng;
saearch = [size(trainx, 2), repmat(NN.NHNEURON, 1, NN.NHLAYER)];
sae = saesetup(saearch);

for i = 1 : NN.NHLAYER
    sae.ae{i}.activation_function = 'sigm';
    %sae.ae{i}.learnRate = '1';
    sae.ae{i}.inputZeroMaskedFraction = NN.ZEROMASK;
end

opts.numepochs = NN.SAEEPOCH;
opts.batchsize = NN.BATCHSIZE;
saetrain(sae, trainx, opts) % Train SDAE
%visualize(sae.ae{1}.W{1}')

% Use the SDAE to initialize a FFNN
nnarch = [saearch size(trainy, 2)];
nn = nnsetup(nnarch);
nn.activation_function = 'sigm';
%nn.learningRate = 1;

for i = 1 : NN.NHLAYER % Transfer weights from SDAE
    nn.W{i} = sae.ae{i}.W{1};
end

opts.numepochs = NN.NNEPOCH;
opts.batchsize = NN.BATCHSIZE;
% nn = nntrain(nn, trainx, trainy, opts);

end
