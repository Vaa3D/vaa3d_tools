:: <<swc_registration.exe>> config
:: <the path to input file>
:: -C <input_CCF_marker> input _RPM_tar.marker file full name.
:: -M <input_Manual_marker>	input _RPM_sub.marker file full name.
:: -o <input_original_swc>	input original swc file full name.
:: -T <input_tar_auto_marker>  input ori_local_registered_tar.marker file full name.
:: -S <input_brain_auto_marker>     input ori_local_registered_sub.marker file full name.
:: -d <input_FFD_grid>  input _FFD_grid.swc file full name. If FFD is included in the brain registration, add it.
:: <the size to original brain image>
:: -X <original image size_x>   input original brain image size_x.
:: -Y <original image size_y>   input original brain image size_y.
:: -Z <original image size_z>   input original brain image size_z.
:: <the size to resample/stripmove brain image>
:: -x <resample image size_x>   input resample brain image size_x.
:: -y <resample image size_y>   input resample brain image size_y.
:: -z <resample image size_z>   input resample brain image size_z.
:: <the size to template brain image,default [568 320 456], is "average_template_25_u8_xpad.v3draw", an Allen CCF brain>
:: -a <average_template image size_x>   input average_template brain image size_x.
:: -b <average_template image size_y>   input average_template brain image size_y.
:: -c <average_template image size_z>   input average_template brain image size_z.
:: -p <pading x>       input template brain xpad, if you don't want to keep it, default 20.
:: <the path to output swc file>
:: -r <out_resample_swc>    output resample_swc file full name.
:: -f <out_affine_swc>     output affine_swc file full name.
:: -s <out_stps_swc>     output stps_swc file full name.


swc_registration.exe -C ./sample_data\raw_RPM_tar.marker -M ./sample_data\raw_RPM_sub.marker -o ./sample_data\original_data.swc -T ./sample_data\ori_local_registered_tar.marker -S ./sample_data\ori_local_registered_sub.marker -d ./sample_data\raw_FFD_grid.swc -X 37184 -Y 54592 -Z 9952 -x 581 -y 853 -z 622 -a 568 -b 320 -c 456 -p 20 -r ./result\resample_data.swc -f ./result\global_data.swc -s ./result\local_registered_data.swc

pause
