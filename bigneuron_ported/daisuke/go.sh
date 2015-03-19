#!/bin/bash +x
export OMP_NUM_THREADS=1

STARTTIME=$(date +'%s')
PLUGIN_NAME=neuron_tracing/Vaa3D_Neuron2_Tuned
#/usr/bin/time -f "%U sec %M KB" ./v3d_external/bin/vaa3d -x ${PLUGIN_NAME} -f app2 -i ./myneurons/0969_seg.tif -p NULL 0 10 0
/usr/bin/time -f "%U sec %M KB" ./v3d_external/bin/vaa3d -x ${PLUGIN_NAME} -f app2 -i ./myneurons/0965_seg.tif -p NULL 0 10 0

#/usr/bin/time -f "%U sec %M KB" ./v3d_external/bin/vaa3d -x ${PLUGIN_NAME} -f app2 -i ./testdata/02013.THMARCM-573_seg.lsm.tif.c3.v3draw.uint8.v3draw
#/usr/bin/time -f "%U sec %M KB" ./v3d_external/bin/vaa3d -x ${PLUGIN_NAME} -f app2 -i ./testdata/02012.THMARCM-523M_seg1.lsm.tif.c3.v3draw.uint8.v3draw
#/usr/bin/time -f "%U sec %M KB" ./v3d_external/bin/vaa3d -x ${PLUGIN_NAME} -f app2 -i ./testdata/02011.THMARCM-514M_seg1.lsm.tif.c3.v3draw.uint8.v3draw
#/usr/bin/time -f "%U sec %M KB" ./v3d_external/bin/vaa3d -x ${PLUGIN_NAME} -f app2 -i ./testdata/02010.THMARCM-432M_seg2.lsm.tif.c3.v3draw.uint8.v3draw


ENDTIME=$(date +'%s')
echo "TOTAL : $(($ENDTIME - $STARTTIME)) sec"
