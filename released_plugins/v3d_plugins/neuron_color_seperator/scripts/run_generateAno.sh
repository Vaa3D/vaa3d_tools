#Hanbo Chen
#2015.7
#generate ano file for proofreading
#each ano file links the maximum project or raw image as well as all seperation results
while read sub
do
	fname_raw=${sub##*/}
	fname=${fname_raw}.img.mip.tiff
	echo RAWIMG=${fname} > ${curpwd}/${fname_raw}.ano
	ls ${fname_raw}.extract* | while read fname_ext
	do
		echo RAWIMG=${fname_ext} >> ${curpwd}/${fname_raw}.ano
	done	
done < ../list_rawimage.txt
#generate the list of files to proofread, it is just a list of ano files
ls *.ano > list_proofread.txt