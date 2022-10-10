# <<swc_registration.exe>> config
# <the path to input file>
# -C <input_CCF_marker> input _RPM_tar.marker file full name.
# -M <input_Manual_marker>	input _RPM_sub.marker file full name.
# -o <input_original_swc>	input original swc file full name.
# -T <input_tar_auto_marker>  input ori_local_registered_tar.marker file full name.
# -S <input_brain_auto_marker>     input ori_local_registered_sub.marker file full name.
# -d <input_FFD_grid>  input _FFD_grid.swc file full name. If FFD is included in the brain registration, add it.
# <the size to resample/stripmove brain image>
# -x <resample image size_x>   original image sample reduction x.
# -y <resample image size_y>   original image sample reduction y.
# -z <resample image size_z>   original image sample reduction z.
# <the size to template brain image,default [528 320 456], is "CCF_u8_xpad.v3draw", an Allen CCF brain>
# -a <average_template image size_x>   input average_template brain image size_x.
# -b <average_template image size_y>   input average_template brain image size_y.
# -c <average_template image size_z>   input average_template brain image size_z.
# <the path to output swc file>
# -r <out_resample_swc>    output resample_swc file full name.
# -g <out_RPM_swc>     output RPM_swc file full name.
# -f <out_FFD_swc>		output FFD_swc file full name.
# -s <out_stps_swc>     output stps_swc file full name.


../binary/linux_bin/swc_registration -C sample_data/raw_RPM_tar.marker -M sample_data/raw_RPM_sub.marker -o sample_data/original_data1.swc -T sample_data/ori_local_registered_tar.marker -S sample_data/ori_local_registered_sub.marker -d sample_data/raw_FFD_grid.swc -x 64 -y 64 -z 16 -a 528 -b 320 -c 456 -r result/resample_data.swc -g result/global_data.swc -f result/FFD_data.swc -s result/local_registered_data.swc

pause
