function classificationTest(MEX_PATH)

if nargin < 1
    MEX_PATH = '../build';
end

addpath(MEX_PATH);

file_data = load('../datasets/ionosphere.txt');
Data = file_data(:,1:end-1)';
Labels = file_data(:, end)';
Labels = Labels*2 - 1;

X = Data';
Y = Labels';

numTrain = round(length(Y)/2);

train.X = X(1:numTrain,:);
train.Y = Y(1:numTrain);

test.X = X((numTrain+1):end,:);
test.Y = Y((numTrain+1):end);

opts = [];
opts.loss = 'exploss'; % can be logloss or exploss

% gradient boost options
opts.shrinkageFactor = 0.1;
opts.subsamplingFactor = 0.5;
opts.maxTreeDepth = uint32(2);  % this was the default before customization
opts.randSeed = uint32(rand()*1000);

numIters = 200;
tic;
model = SQBMatrixTrain(single(train.X), train.Y, uint32(numIters), opts);
toc

pred = SQBMatrixPredict( model, single(test.X) );

err = sum( (pred > 0) ~= (test.Y > 0))/length(pred);
fprintf('Prediction error: %f\n', err);