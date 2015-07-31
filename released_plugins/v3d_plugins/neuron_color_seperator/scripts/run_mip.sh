#Hanbo Chen
#2015.7
#script to generate mip for original brainbow data
for i in *.conf; 
do 
	fname_raw=${i%.extract.conf}
	fname_raw=${fname_raw##*/}
	echo ${fname_raw}
	fname_img=$(ls ../../raw/*/${fname_raw})
	echo ${fname_img}	
	#this will keep the 1st 3 channel and convert the data to UCHAR if it is not
	v3d -x save_channel -f save_RGB_channel -i ${fname_img} -o ~/tmp.v3draw	
	v3d -x mipZ -f mip_zslices -i ~/tmp.v3draw -p 1:1:e -o mip/${fname_raw}.img.mip.tiff 
done
