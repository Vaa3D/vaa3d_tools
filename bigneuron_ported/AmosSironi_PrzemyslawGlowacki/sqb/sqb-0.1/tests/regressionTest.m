MEX_PATH = '../build';

addpath(MEX_PATH);

file_data = load('../datasets/airquality.mat');
Data = [file_data.wind, file_data.temperature, file_data.radiation];
Labels = file_data.ozone;

test.X = Data(1:2:end,:);
test.Y = Labels(1:2:end,:);

train.X = Data(2:2:end,:);
train.Y = Labels(2:2:end,:);

opts = [];
opts.loss = 'squaredloss'; % can be logloss or exploss

% this has to be not too high (max 1.0)
opts.shrinkageFactor = 0.01;
opts.subsamplingFactor = 0.2;
opts.maxTreeDepth = uint32(2);  % this was the default before customization
opts.randSeed = uint32(rand()*1000);

numIters = 600;
tic;
model = SQBMatrixTrain(single(train.X), train.Y, uint32(numIters), opts);
toc

%% Test model

pred = SQBMatrixPredict( model, single(test.X) );
outErr = sum((pred - test.Y).^2);

fprintf('Prediction error (MSE): %f\n', outErr / length(pred));