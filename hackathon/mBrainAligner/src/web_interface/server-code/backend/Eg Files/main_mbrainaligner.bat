:: -f fixed_image
:: -m moving_image
:: -c recentered_image_pad_scr_image (if you select "rpm")
:: -th moving_image_threshold (if less than the given threshold,then the pixel value = 0. The default is 30)
:: -p <registration_methods>，a:affine, r:rpm，s:ssd, example:a+s,r+s (It is recommended that you only need to select one option in "affine" and "rpm")
:: -o the result save path

mBrainAligner_data\3rdparty\global_registration.exe -f mBrainAligner_data/data/average_template_25_u8_xpad.v3draw -c mBrainAligner_data/data/recentered_image_pad_scr.v3draw -m mBrainAligner_data/Registration_data/18458.v3draw  -p r+s -o mBrainAligner_data/result/ -th 30

:: -p <Algorithm parameter file>, If the user uses the registration sample data, the interface needs to provide the user with four modes of registration (fMOST,LSFM,VISoR,MRI), 
::                                and the corresponding parameter file is (the file name is fixed) :fMOST_config.txt,LSFM_config.txt,VISoR_config.txt,MRI_config.txt
::                                If the user is registering with private data, we need to generate "...txt" file (see fmost_config.txt file).
:: -s <global_result_image>  The file name is fixed. If the result save path is "mBrainAligner_data/result/", then "-s" is "result/global.v3draw".
:: -m <fmost_segmentation_result> 
:: -l <landmarks_file_path> example: "high_landmarks.txt" "middle_landmarks.txt" "low_landmarks.txt" 
:: -o the result save path

mBrainAligner_data\3rdparty\local_registration.exe -p mBrainAligner_data/data/fMOST_config.txt -s mBrainAligner_data/result/global.v3draw -m mBrainAligner_data/seg/ -l mBrainAligner_data/data/high_landmarks.marker  -g mBrainAligner_data/data/ -o mBrainAligner_data/result/
pause