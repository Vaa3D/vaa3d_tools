# In this folder you find a vaa3d plugin implementing the method described in [1] (./SQBTree_plugin)
# and a plugin using the the method [1] in combination with the Minimum Spanning Tree (MST) algorithm to reconstruct a neuron in an 3D image (./RegMST_tracing_plugin)
#
# The plugin under ./RegMST_tracing_plugin will be used in the BigNeuron bench-testing [2]
 

 ***1. compile the two plugins ***
cd ./SQBTree_plugin
qmake && make
cd ../RegMST_tracing_plugin
qmake && make
cd ..

 *** run the tracing plugin on a small image ***
cd ../../../v3d_external/bin

./vaa3d -x RegMST -f tracing_func -i ../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/cropped_OP_9.tif -p 1 2 ../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/trained_models/OPF_debug/Regressor_ac_0.cfg ../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/trained_models/OPF_debug/Regressor_ac_1.cfg 21 

 *** visualize results ***
load in vaaa3d the original image : ../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/cropped_OP_9.tif
          and  the reconstruction swc file : ../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/cropped_OP_9.tif_tubularity.v3draw_RegMST_Tracing.swc


[1] A. Sironi, E. Türetken, V. Lepetit and P. Fua. Multiscale Centerline Detection, submitted to IEEE Transactions on Pattern Analysis and Machine Intelligence.
[2] http://alleninstitute.org/bigneuron/about/