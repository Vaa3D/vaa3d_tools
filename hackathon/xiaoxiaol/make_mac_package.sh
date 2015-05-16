#!/bin/bash

cd ~/work/src/vaa3d
svn up

cd ~/work/bin/vaa3d
cmake .
make -j8

cd ~/work/v3d/v3d_external
svn up
cd ~/work/v3d/vaa3d_tools
svn up

cd ~/work/v3d/v3d_external
./build.macx -B -m -j5
# run big neuron tracing
# run zhi's tracing


cd ~/work/bin/vaa3d/v3d/Mac_Fat/Vaa3d.app/Contents/MacOS
rm -rf plugins
cp -r  ~/work/v3d/v3d_external/bin/plugins ./

cd ~/work/bin/vaa3d/
make package
rm v3d/Mac_Fat/Vaa3d.app/Contents/MacOS/plugins/image_stitching/istitch/libimageStitch.dylib
rm v3d/Mac_Fat/Vaa3d.app/Contents/MacOS/plugins/image_segmentation/lobeseger/liblobeseg.dylib
rm v3d/Mac_Fat/Vaa3d.app/Contents/MacOS/plugins/refExtract/librefExtract.dylib
rm v3d/Mac_Fat/Vaa3d.app/Contents/MacOS/plugins/image_registration/SSD_registration/libplugin_PQ_imagereg.dylib
rm v3d/Mac_Fat/Vaa3d.app/Contents/MacOS/plugins/image_blending/blend_multiscanstacks/libblend_multiscanstacks.dylib
rm v3d/Mac_Fat/Vaa3d.app/Contents/MacOS/plugins/FlyWorkstation_utilities/clonalSelect/libclonalselect.dylib
rm v3d/Mac_Fat/Vaa3d.app/Contents/MacOS/plugins/image_stitching/ifusion/libifusion.dylib

#otool -L /Users/xiaoxiaoliu/work/bin/vaa3d/v3d/Mac_Fat/Vaa3d.app/Contents/MacOS/plugins/image_filters/anisotropic_filter/anisodiff_littlequick/libanisodiff_littlequick_debug.dylib


# copy vaa3d executable and plugins to the template package folder



