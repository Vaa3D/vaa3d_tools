#！/bin/bash
#<<global_registration >> parameter
# -f fixed_image
# -m moving_image
# -c recentered_image_pad_scr_image (if you select "rpm")
# -d moving_image_threshold (if less than the given threshold,then the pixel value = 0. The default is 30)
# -p <registration_methods>，a:affine, r:rpm，s:ssd, example:a+s,r+s (It is recommended that you only need to select one option in "affine" and "rpm")
# -o the result save path

#<<local_registration >> parameter
# -p <Algorithm parameter file>, If the user uses the registration sample data, the interface needs to provide the user with four modes of registration (fMOST,LSFM,VISoR,MRI), 
#                                and the corresponding parameter file is (the file name is fixed) :fMOST_config.txt,LSFM_config.txt,VISoR_config.txt,MRI_config.txt
#                                If the user is registering with private data, we need to generate "...txt" file (see fmost_config.txt file).
# -s <global_result_image>  The file name is fixed. If the result save path is "mBrainAligner_data/result/", then "-s" is "result/global.v3draw".
# -m <fmost_segmentation_result> 
# -l <landmarks_file_path> example: "high_landmarks.txt" "middle_landmarks.txt" "low_landmarks.txt" 
# -o the result save path

# registration fMOST sample data
mBrainAligner_data/3rdparty_linux/global_registration  -f mBrainAligner_data/data/average_template_25_u8_xpad.v3draw -c mBrainAligner_data/data/recentered_image_pad_scr.v3draw 
-m Registration_data/fMOST_18458_raw.v3draw  -p r+f+n -o mBrainAligner_data/result/fMOST/ -d 1

mBrainAligner_data/3rdparty_linux/local_registration  -p Registration_data/Parameter/fMOST_config.txt -s mBrainAligner_data/result/fMOST/global.v3draw 
-m Registration_data/fMOST_segmentation/ -l mBrainAligner_data/data/density_landmarks/low_landmarks.marker  -g mBrainAligner_data/data/ -o mBrainAligner_data/result/fMOST/


# registration LSFM sample data
mBrainAligner_data/3rdparty_linux/global_registration  -f mBrainAligner_data/data/average_template_25_u8_xpad.v3draw -c mBrainAligner_data/data/recentered_image_pad_scr.v3draw 
-m Registration_data/LSFM_raw.v3draw  -p r+f+n -o mBrainAligner_data/result/LSFM/ -d 70

mBrainAligner_data/3rdparty_linux/local_registration  -p Registration_data/Parameter/LSFM_config.txt -s mBrainAligner_data/result/LSFM/global.v3draw 
-l mBrainAligner_data/data/density_landmarks/low_landmarks.marker  -g mBrainAligner_data/data/ -o mBrainAligner_data/result/LSFM/


# registration MRI sample data
mBrainAligner_data/3rdparty_linux/global_registration  -f mBrainAligner_data/data/average_template_25_u8_xpad.v3draw -c mBrainAligner_data/data/recentered_image_pad_scr.v3draw 
-m Registration_data/MRI_raw.v3draw  -p r+f+n -o mBrainAligner_data/result/MRI/ -d 20

mBrainAligner_data\3rdparty_linux\local_registration  -p Registration_data/Parameter/MRI_config.txt -s mBrainAligner_data/result/MRI/global.v3draw 
-l mBrainAligner_data/data/density_landmarks/low_landmarks.marker  -g mBrainAligner_data/data/ -o mBrainAligner_data/result/MRI/


# registration VISoR sample data
mBrainAligner_data/3rdparty_linux/global_registration  -f mBrainAligner_data/data/average_template_25_u8_xpad.v3draw -c mBrainAligner_data/data/recentered_image_pad_scr.v3draw 
-m Registration_data/VISoR_raw.v3draw  -p r+f+n -o mBrainAligner_data/result/VISoR/ -d 20

mBrainAligner_data/3rdparty_linux/local_registration  -p Registration_data/Parameter/VISoR_config.txt -s mBrainAligner_data/result/VISoR/global.v3draw 
-l mBrainAligner_data/data/density_landmarks/low_landmarks.marker  -g mBrainAligner_data/data/ -o mBrainAligner_data/result/VISoR/

pause