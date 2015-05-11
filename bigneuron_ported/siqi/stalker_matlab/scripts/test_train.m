tic;
clc; clear all; close all;

% PARA
TESTSIZE        = 1;
TESTSTARTLOC    = [[89, 363, 30]];
TESTSTARTDIR    = [[-0.9278, 0.1916]];
NTESTSTEP       = 100;
STEPSIZE        = 1;
NOISETYPE       = 'original'; % The noise added to vision blocks: original, gauss, salt_pepper
NTREE           = 200;
MTRY            = 140;
CACHETRAIN      = true;
PREFIX          = 'OP_';
CACHETRAINDATA  = true;
CACHETRAINMODEL = true;
% ENDPARA

curdir = fileparts(mfilename('fullpath'));
datadir = fullfile(curdir, '..', 'data', 'input', 'preprocessed');
addpath(fullfile(curdir, '..', 'utils'));

nsbj = numel(dir([datadir, [filesep, '*.mat']]))-1;

% disp(['cur dir:', curdir]);
% disp(['data dir:', datadir]);
% disp(dir([datadir, [filesep, '*.mat']]));
% disp(sprintf('nsbj: %d\n', nsbj));

load(fullfile(datadir, 'gt_i.mat'), 'VBOXSIZE', 'ZERO_SIZE');

%     Read in the robots 
%     Only deal with low memory use
    for i = 1 : nsbj-TESTSIZE
        fsbj = load(fullfile(datadir, strcat(PREFIX, num2str(nsbj), '.mat')), 'sbj');   
        ltrainrobot{i} = fsbj.sbj;
    end

    %ltrainrobot = lsbj{1 : numel(lsbj)-TESTSIZE};
    %ltestrobot = lsbj{numel(lsbj)-TESTSIZE+1 : end};
    
    ltestrobot_counter = 1;
    for i = (nsbj - TESTSIZE + 1) : nsbj
        fsbj = load(fullfile(datadir, strcat(PREFIX, num2str(nsbj), '.mat')), 'sbj');   
        ltestrobot{ltestrobot_counter} = fsbj.sbj;
        ltestrobot_counter = ltestrobot_counter + 1;
    end
    
    
    % Count the train vbox
    ntrain = 0;
    for r = 1 : numel(ltrainrobot)
        ntrain = ntrain + numel(ltrainrobot{1,r}.lrobot);
    end

    % Count the test vbox
    ntest = 0;
    for r = 1 : numel(ltestrobot)
        ntest = ntest + numel(ltestrobot{1,r}.lrobot);
    end

    
    train_x = zeros(ntrain, VBOXSIZE^3);
    row = 1;
    for i = 1 : numel(ltrainrobot)
        for j = 1 : numel(ltrainrobot{1,i}.lrobot)
%             disp(sprintf('Reading train matrix row %d\n', row));
            vb = ltrainrobot{1,i}.lrobot(j).visionbox.(NOISETYPE);

            if ltrainrobot{i}.lrobot(j).fissure == 1 % Only use the vboxes not at branching locations
%                 disp('fissure')
                continue;
            end

        	train_x(row, :) = vb(:);
            train_y(row, 1) = ltrainrobot{1,i}.lrobot(j).next_th;
            train_y(row, 2) = ltrainrobot{1,i}.lrobot(j).next_phi;
            row = row + 1;
        end
    end

    train_x(row:end,:) = []; % Delete the redundent rows not used because of the branching
    train_y(row:end,:) = [];

clearvars ltrainrobot ltestrobot; 
    disp('Start to train RF...');
    options = statset('UseParallel', 'Always');
    
    orith = train_y(:, 1);
    oriphi = train_y(:, 2);
    [gtx, gty, gtz] = sph2cart(orith, oriphi, ones(numel(orith, 1)));
    [invth, invphi] = cart2sph(-gtx, -gty, -gtz);
    gtth = orith;
    gtphi = oriphi;
    orith > pi
    gtth(orith > pi) = invth(orith > pi);
    gtphi(orith > pi) = invphi(orith > pi);
   
%    tic
%     % Train Theta
%     rf_th = TreeBagger(NTREE, train_x, gtth,...
%                             'Method', 'regression', 'NVarToSample',...
%                              MTRY, 'NPrint', true,'Options',options); 
%     toc
%     % Train Phi
%     rf_phi = TreeBagger(NTREE, train_x, gtphi,... 
%                              'Method', 'regression', 'NVarToSample',...
%                               MTRY, 'NPrint', true,'Options',options);
