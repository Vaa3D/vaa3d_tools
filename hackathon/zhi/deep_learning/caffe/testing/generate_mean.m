config = struct();

config.base_address = '/local4/Data/IVSCC_test/comparison/Caffe_testing/train/';
config.target_file  = 'train.txt';
config.target_size  = [64 64];

config.result_file  = 'train_IVSCC_mean_64_64.mat';

%% Doing the job!

fid = fopen([config.base_address config.target_file]);

fileLines = textscan(fid, '%[^ ] %d', 'delimiter', '\n', 'BufSize', 100000);
fclose(fid);

files = fileLines{1, 1};
file_count = numel(files);
fprintf('Found %d files\n', file_count);

images = cell(file_count, 1);
fprintf('Processing the files ...'); tic;
parfor f = 1:file_count,
    fprintf('Doing %s\n', files{f});
    im = imread([config.base_address files{f}]);
    im = cat(3,im,im,im);
    im = imresize(im, config.target_size, 'bilinear');
    images{f} = im;
end
fprintf(' done in %.2fs\n', toc);

%% The rest of the job
fprintf('Calculating the mean\n');

images_reshape = cellfun(@(x) reshape(x, [1 config.target_size 3]), images, 'UniformOutput', false);
images_reshape = cell2mat(images_reshape);
image_mean_exp = mean(images_reshape);

image_mean = single(squeeze(image_mean_exp));

%% Saving the output
fprintf('Saving the file ...'); tic;
save([config.base_address config.result_file], 'image_mean');
fprintf(' done in %.2fs\n', toc);

%% clear up
clear image_mean images_reshape image_mean_exp images;