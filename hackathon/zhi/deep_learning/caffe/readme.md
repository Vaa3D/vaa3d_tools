Deep learning for IVSCC data

Install deep learning framework Caffe using the instruction (http://caffe.berkeleyvision.org/installation.html)

Traning process using CaffeNet(a variant of AlexNet):
1. Prepare training data and validation data, saved like:
/path/to/imagenet/train/bkg/min_xy_313862167.jp2.tif_x30_y30_z1_bkg_x1440_y4559_z1_bkg.tif
/path/to/imagenet/val/min_xy_313862167.jp2.tif_x30_y30_z1_bkg_x3619_y2754_z1_bkg.tif
The training and validation input are described in ./training/train.txt and ./training/val.txt as text listing all the files and their labels. 

2. Assume Caffe is installed in CAFFE_ROOT. Use CAFFE_ROOT/examples/imagenet/create_imagenet.sh to generate the leveldbs:
CAFFE_ROOT/examples/imagenet/ilsvrc12_train_leveldb
CAFFE_ROOT/examples/imagenet/ilsvrc12_val_leveldb 
For our case, we set “RESIZE=true” to resize all images to 64x64, and set the paths to the train and val dirs.

3. Cacualte image mean using CAFFE_ROOT/examples/imagenet/make_imagenet_mean.sh to generate the mean file:
CAFFE_ROOT/ilsvrc12/imagenet_mean.binaryproto

4. Under CAFFE_ROOT foler, train the data the command line: ./build/tools/caffe train --solver=models/bvlc_reference_caffenet/solver.prototxt. Two example files are here:
./training/solver.prototxt
./training/train_val.prototxt

5. The trained CaffeNet mode will be saved in:
CAFFE_ROOT/models/bvlc_reference_caffenet/bvlc_reference_caffenet.caffemodel


Classification process using Caffe Matlab inderface:
1. Install MATLAB Caffe by running "make all matcaffe" under CAFFE_ROOT.

2. Use "./testing/generate_mean.m" to generate mean file for Matlabe use.

3. Under CAFFE_ROOT folder, use "./testing/classification_IVSCC.m" as the function for classfication. Two exmaple files are here:
./testing/IVSCC_classfi_all.m
./testing/IVSCC_classfi_all_bkg.m
  
