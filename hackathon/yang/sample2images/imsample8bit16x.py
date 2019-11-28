
#
# python imsample.py input.tif output.tif
#


import sys
import skimage
import numpy

# read input tiff file
im = skimage.io.imread(sys.argv[1], plugin='tifffile');

# downsample 16x in x and y
imds = skimage.transform.downscale_local_mean(im, (16,16))

# convert float64 to uint8
imdsint = numpy.array(imds, dtype='uint8')

# save downsampled image to lzw (compress=5) tiff file
skimage.io.imsave(sys.argv[2], imdsint, compress=5, plugin='tifffile')



