#A shell program to strip IVSCC image names of their special characters (;, (, )) so they can run in Vaa3D

if [ ! $# -ge 1 ]; then
   echo "sh clean_image_names.sh <input folder> <output folder>"
   exit
else 
   if [ ! -d $1 ]; then
     echo "cannot find input folder"
   fi
fi

if [ ! $# -ge 2 ]; then
   echo "sh clean_image_names.sh <input folder> <output folder>"
   exit
fi

in_folder=$1
out_folder=$2

if [ ! -d $out_folder ]; then
   mkdir $out_folder
fi

for str in $(ls $in_folder); 
   do cp $in_folder/$str $out_folder/${str//[;()]/}
done

