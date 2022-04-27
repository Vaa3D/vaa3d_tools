::<<global_registration.exe>> config
:: -f fixed_image
:: -m moving_image
:: -c recentered_image_pad_scr_image (if you select "rpm")
:: -d moving_image_threshold (if less than the given threshold,then the pixel value = 0. The default is 30)
:: -l the x-axis/y-axis/z-axispadding size of the target image
:: -p <registration_methods>，a:affine, r:rpm，f:ffd, example:a+f,r+f (It is recommended that you only need to select one option in "affine" and "rpm")
:: -o the result save path
:: -u <gpu mode>,0:gpu_off,1:gpu_on

::<<local_registration.exe>> config
:: -p <Algorithm config file>, If the user uses the registration sample data, the interface needs to provide the user with four modes of registration (fMOST,LSFM,VISoR,MRI), 
::                                and the corresponding config file is (the file name is fixed) :fMOST_config.txt,LSFM_config.txt,VISoR_config.txt,MRI_config.txt
::                                If the user is registering with private data, we need to generate "...txt" file (see fmost_config.txt file).
:: -s <global_result_image>  The file name is fixed. If the result save path is "result/fMOST/", 
::                           then "-s" is "result/fMOST/global.v3draw".
:: -m <fmost_segmentation_result> 
:: -l <landmarks_file_path> example: "high_landmarks.txt" "middle_landmarks.txt" "low_landmarks.txt" 
:: -o the result save path
:: -u <gpu mode>,0:gpu_off,1:gpu_on  If your runtime environment does not have GPU, please select "0"

:: registration fMOST sample data
..\binary\win64_bin\global_registration.exe -f target/25um/  -m subject/fMOST_18458_raw.v3draw  -p r+f+n -o result/fMOST/ -d 1 -l 30+20+20 -u 1
..\binary\win64_bin\local_registration.exe -p config/fMOST_config.txt -s result/fMOST/global.v3draw -m subject/fMOST_segmentation_50um/ -l target/25um/target_landmarks/low_landmarks.marker  -g target/25um/ -o result/fMOST/ -u 1  

..\binary\win64_bin\global_registration.exe -f target/25um/  -m subject/18052_red_mm_RSA_x2.nii.gz  -p r+f+n -o result/fMOST_nii/ -d 1 -l 20+0+0 -u 1
..\binary\win64_bin\local_registration.exe -p config/fMOST_withou_seg_config.txt -s result/fMOST_nii/global.nii.gz -l target/25um/target_landmarks/low_landmarks.marker  -g target/25um/ -o result/fMOST_nii/ -u 1  


:: registration LSFM sample data
..\binary\win64_bin\global_registration.exe -f target/25um/ -m subject/LSFM_raw.v3draw  -p r+f+n -o result/LSFM/ -d 70 -l 30+30+30 -u 1
..\binary\win64_bin\local_registration.exe -p config/LSFM_config.txt -s result/LSFM/global.v3draw -l target/25um/target_landmarks/low_landmarks.marker  -g target/25um/ -o result/LSFM/ -u 1

:: registration MRI sample data
..\binary\win64_bin\global_registration.exe -f target/25um/ -m subject/MRI_raw.v3draw  -p r+f+n -o result/MRI/ -d 20 -l 30+30+30 -u 1
..\binary\win64_bin\local_registration.exe -p config/MRI_config.txt -s result/MRI/global.v3draw -l target/25um/target_landmarks/low_landmarks.marker  -g target/25um/ -o result/MRI/ -u 1

:: registration VISoR sample data
..\binary\win64_bin\global_registration.exe -f target/25um/ -m subject/VISoR_raw.v3draw  -p r+f+n -o result/VISoR/ -d 20 -l 30+30+30 -u 1
..\binary\win64_bin\local_registration.exe -p config/VISoR_config.txt -s result/VISoR/global.v3draw -l target/25um/target_landmarks/low_landmarks.marker  -g target/25um/ -o result/VISoR/ -u 1

:: registration LSFM half brain sample data
..\binary\win64_bin\global_registration.exe -f target/half/  -m subject/LSFM_half_raw.v3draw  -p r+f -o result/LSFM_half/ -d 70 -l 20+30+30 -u 1
..\binary\win64_bin\local_registration.exe -p config/LSFM_half_config.txt -s result/LSFM_half/global.v3draw -l target/half/target_landmarks/high_landmarks_half.marker  -g target/half/ -o result/LSFM_half/ -u 1 

pause
