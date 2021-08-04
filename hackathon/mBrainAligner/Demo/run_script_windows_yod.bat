::<<global_registration.exe>> parameter
:: -f fixed_image
:: -m moving_image
:: -c recentered_image_pad_scr_image (if you select "rpm")
:: -d moving_image_threshold (if less than the given threshold,then the pixel value = 0. The default is 30)
:: -p <registration_methods>，a:affine, r:rpm，f:ffd, example:a+f,r+f (It is recommended that you only need to select one option in "affine" and "rpm")
:: -o the result save path  example "Registration_sample_data/result/fMOST/"

::<<local_registration.exe>> parameter
:: -p <Algorithm parameter file>, If the user uses the registration sample data, the interface needs to provide the user with four modes of registration (fMOST,LSFM,VISoR,MRI), 
::                                and the corresponding parameter file is (the file name is fixed) :fMOST_config.txt,LSFM_config.txt,VISoR_config.txt,MRI_config.txt
::                                If the user is registering with private data, we need to generate "...txt" file (see fmost_config.txt file).
:: -s <global_result_image>  The file name is fixed. If the result save path is "Registration_sample_data/result/fMOST/", 
::                           then "-s" is "Registration_sample_data/result/fMOST/global.v3draw".
:: -m <fmost_segmentation_result> 
:: -l <landmarks_file_path> example: "high_landmarks.txt" "middle_landmarks.txt" "low_landmarks.txt" 
:: -o the result save path example "Registration_sample_data/result/fMOST/"

:: registration your own data
mBrainAligner_data\3rdparty_windows\global_registration.exe -f mBrainAligner_data/data/average_template_25_u8_xpad.v3draw -c mBrainAligner_data/data/recentered_image_pad_scr.v3draw ^
^ -m ***.v3draw  -p r+f+n -o Registration_sample_data/result/yod/ -d 15

mBrainAligner_data\3rdparty_windows\local_registration.exe -p Registration_sample_data/Parameter/yod_config.txt -s Registration_sample_data/result/yod/global.v3draw ^
^ -l mBrainAligner_data/data/density_landmarks/low_landmarks.marker  -g mBrainAligner_data/data/ -o Registration_sample_data/result/yod/

pause