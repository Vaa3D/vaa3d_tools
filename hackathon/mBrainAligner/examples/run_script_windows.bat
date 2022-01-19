::<<global_registration.exe>> config
:: -f <input_fixed_image> .raw/.tif/.lsm file containing 3D template/target stack to which a subject image will be warped/mapped.        
::                                     The .raw file is in V3D's RAW format.
:: -m <input_moving_image> .raw/.tif/.lsm file containing 3D template/target stack to which a subject image will be warped/mapped.        
::                                     The .raw file is in V3D's RAW format.
:: -c <fixed_image_mask> The foreground mask of fixed image(When you select rpm(-p r) in the global registration, this option must be used).
:: -d <moving_image_threshold> This option can remove image artifacts(if less than the given threshold,then the pixel value = 0, the default is 30).
:: -p <global_registration_methods>，a:affine, r:rpm，f:ffd, example:a+f,r+f (It is recommended that you only need to select one option in "affine" and "rpm")
:: -o <Results_save_path> Global registration result will saved here.

::<<local_registration.exe>> config
:: -p <algorithm config file>, If the user uses the registration sample data, the interface needs to provide the user with four modes of registration (fMOST,LSFM,VISoR,MRI), 
::                                and the corresponding config file is (the file name is fixed) :fMOST_config.txt,LSFM_config.txt,VISoR_config.txt,MRI_config.txt
::                                If the user is registering with your own data, we need to generate "**.txt" file (see fMOST_config.txt file).
:: -s <input_moving_image> .raw/.tif/.lsm file containing 3D template/target stack to which a subject image will be warped/mapped. The .raw file is in V3D's RAW format.       
::                          The input in the example is the result of global registration, so this option is set to "result/fMOST/global.v3draw".
:: -m <fMOST_segmentation_data> If the "Select_modal" is set to 0 in the fMOST_config.txt,, this option must be used. 
:: -l <landmarks_file_path> example: "high_landmarks.txt" "middle_landmarks.txt" "low_landmarks.txt" 
:: -o <Results_save_path> Local registration result will saved here.

:: registration fMOST sample data
..\binary\win64_bin\global_registration.exe -f target/CCF_u8_xpad.v3draw -c target/CCF_mask.v3draw ^
^ -m subject/fMOST_18458_raw.v3draw  -p r+f+n -o result/fMOST/ -d 1
..\binary\win64_bin\local_registration.exe -p config/fMOST_config.txt -s result/fMOST/global.v3draw ^
^ -m subject/fMOST_segmentation/ -l target/target_landmarks/low_landmarks.marker  -g target/ -o result/fMOST/


:: registration LSFM sample data
..\binary\win64_bin\global_registration.exe -f target/CCF_u8_xpad.v3draw -c target/CCF_mask.v3draw ^
^ -m subject/LSFM_raw.v3draw  -p r+f+n -o result/LSFM/ -d 70
..\binary\win64_bin\local_registration.exe -p config/LSFM_config.txt -s result/LSFM/global.v3draw ^
^ -l target/target_landmarks/low_landmarks.marker  -g target/ -o result/LSFM/


:: registration MRI sample data
..\binary\win64_bin\global_registration.exe -f target/CCF_u8_xpad.v3draw -c target/CCF_mask.v3draw ^
^ -m subject/MRI_raw.v3draw  -p r+f+n -o result/MRI/ -d 20
..\binary\win64_bin\local_registration.exe -p config/MRI_config.txt -s result/MRI/global.v3draw ^
^ -l target/target_landmarks/low_landmarks.marker  -g target/ -o result/MRI/


:: registration VISoR sample data
..\binary\win64_bin\global_registration.exe -f target/CCF_u8_xpad.v3draw -c target/CCF_mask.v3draw ^
^ -m subject/VISoR_raw.v3draw  -p r+f+n -o result/VISoR/ -d 20
..\binary\win64_bin\local_registration.exe -p config/VISoR_config.txt -s result/VISoR/global.v3draw ^
^ -l target/target_landmarks/low_landmarks.marker  -g target/ -o result/VISoR/

pause
