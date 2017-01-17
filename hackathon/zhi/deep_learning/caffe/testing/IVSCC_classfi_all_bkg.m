clear all;
close all;

folderfile = dir('/local4/Data/IVSCC_test/comparison/released_training_sets_bgk/');

for i = 13:length(folderfile)-1
    filename = dir(['/local4/Data/IVSCC_test/comparison/released_training_sets_bgk/',folderfile(i).name,'/*.tif']);
    predi_index = zeros(length(filename),1);
    for j = 1:length(filename)
        fprintf('folder %d, file %d\n', i,j);
        im = imread(['/local4/Data/IVSCC_test/comparison/released_training_sets_bgk/',folderfile(i).name,'/',filename(j).name]);
        im = cat(3,im,im,im);
        [a, b] = classification_IVSCC(im,1);
        predi_index(j) = b; 
    end
    save(['/local4/Data/IVSCC_test/comparison/Caffe_testing/prediction/bkg/',folderfile(i).name,'.mat'], 'predi_index');

end