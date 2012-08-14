#!/bin/bash
#This script is generated automatically by genscript_1.m

#PS_detection---------------------------------------------------------
#larva
#/groups/peng/home/qul/myhome/work/v3d_external/bin/vaa3d -x principal_skeleton -f detect -i /media/OYEN_BLACK/vnc/tmp/fly_L3_larva/GL_10F10_051109A_MIP.tif /media/OYEN_BLACK/vnc/tmp/fly_L3_larva/larva_principalskeleton_ini.marker /media/OYEN_BLACK/vnc/tmp/fly_L3_larva/larva_principalskeleton.domain -o /media/OYEN_BLACK/vnc/tmp/output.marker /media/OYEN_BLACK/vnc/tmp/output.swc -p 2 1 500 0.01 0.5 7 0 0
#vnc (old)
#/groups/peng/home/qul/myhome/work/v3d_external/bin/vaa3d -x principal_skeleton -f detect -i /media/OYEN_BLACK/vnc/tmp/fly_adult_VNC/a150_4-23-07_L18_mip.tif /media/OYEN_BLACK/vnc/tmp/fly_adult_VNC/vnc_principalskeleton_ini.marker /media/OYEN_BLACK/vnc/tmp/fly_adult_VNC/vnc_principalskeleton.domain -o /media/OYEN_BLACK/vnc/tmp/output.marker /media/OYEN_BLACK/vnc/tmp/output.swc -p 0 1 500 0.01 0.5 7 0 0



#vnc (new) - long-nerrow noise
#/groups/peng/home/qul/myhome/work/v3d_external/bin/vaa3d -x principal_skeleton -f detect -i /media/OYEN_BLACK/vnc/tmp/GMR_9C12_AE_01_26-fA01v_C071116_20071121233548750_rigid.raw /media/OYEN_BLACK/vnc/results/target/ps_ini3.marker /media/OYEN_BLACK/vnc/results/target/vnc_principalskeleton3.domain -o /media/OYEN_BLACK/vnc/tmp/output.marker /media/OYEN_BLACK/vnc/tmp/output.swc -p 1 1 500 0.01 1 15 1 1

#vnc (new) - big-boundary noise
/groups/peng/home/qul/myhome/work/v3d_external/bin/vaa3d -x principal_skeleton -f detect -i /media/OYEN_BLACK/vnc/tmp/GMR_10G02_AE_01_04-fA01v_C080404_20080404225043781_rigid.raw /media/OYEN_BLACK/vnc/results/target/ps_ini3.marker /media/OYEN_BLACK/vnc/results/target/vnc_principalskeleton3.domain -o /media/OYEN_BLACK/vnc/tmp/output.marker /media/OYEN_BLACK/vnc/tmp/output.swc -p 1 1 500 0.01 1 15 1 1


#PS_warp---------------------------------------------------------
#vnc (new) - long-nerrow noise
#/groups/peng/home/qul/myhome/work/v3d_external/bin/vaa3d -x principal_skeleton -f warp -i /media/OYEN_BLACK/vnc/tmp/GMR_9C12_AE_01_26-fA01v_C071116_20071121233548750_rigid.raw /media/OYEN_BLACK/vnc/tmp/output.marker /media/OYEN_BLACK/vnc/results/target/flyVNCtemplate2.raw /media/OYEN_BLACK/vnc/results/target/ps_ini3.marker /media/OYEN_BLACK/vnc/results/target/vnc_principalskeleton3.domain -o /media/OYEN_BLACK/vnc/tmp/output.raw -p 2.0 5 200 1

#vnc (new) - big-boundary noise
/groups/peng/home/qul/myhome/work/v3d_external/bin/vaa3d -x principal_skeleton -f warp -i /media/OYEN_BLACK/vnc/tmp/GMR_10G02_AE_01_04-fA01v_C080404_20080404225043781_rigid.raw /media/OYEN_BLACK/vnc/tmp/output.marker /media/OYEN_BLACK/vnc/results/target/flyVNCtemplate2.raw /media/OYEN_BLACK/vnc/results/target/ps_ini3.marker /media/OYEN_BLACK/vnc/results/target/vnc_principalskeleton3.domain -o /media/OYEN_BLACK/vnc/tmp/output.raw -p 2.0 5 200 1
