function [labelsT,ims] = mex_superpixel_test(filename,step)

  im      = imread(filename);
  labelsT = mex_superpixel(filename,step)';
  ims = imseg(im,labelsT);
  imagesc(ims);
