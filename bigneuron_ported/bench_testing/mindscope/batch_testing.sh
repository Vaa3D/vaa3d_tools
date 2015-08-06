#!/bin/bash
cd /data/mat/zhi/BigNeuron/first2000/images/
for filename in `ls -d *`
do
	/local1/work/vaa3d_tools/hackathon/zhi/neuron_comparison_script/batch_reconstruction.test /data/mat/zhi/BigNeuron/Vaa3D_BigNeuron_version1/vaa3d "/data/mat/zhi/BigNeuron/first2000/images/"$filename"/"$filename "/data/mat/zhi/BigNeuron/first2000/reconstructions/"$filename /data/mat/zhi/BigNeuron/Vaa3D_BigNeuron_version1
	  		
done

