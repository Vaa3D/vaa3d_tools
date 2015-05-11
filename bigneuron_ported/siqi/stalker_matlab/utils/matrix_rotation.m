function [x_dir_list, y_dir_list, z_dir_list,mg_list,voxel_vec_list]=matrix_rotation(x_dir,y_dir,z_dir,mg,voxel_box)

%replace y axis with z axis
y_z_voxel_box=permute(voxel_box,[3 2 1]);
voxel_vec_list=y_z_voxel_box(:)';
x_dir_list=x_dir;
y_dir_list=z_dir;
z_dir_list=y_dir;

%replace x axis with z axis
x_z_voxel_box=permute(voxel_box,[1 3 2]);
voxel_vec_list=[voxel_vec_list;x_z_voxel_box(:)'];
x_dir_list=[x_dir_list;z_dir];
y_dir_list=[y_dir_list;y_dir];
z_dir_list=[z_dir_list;x_dir];

voxel_vec_list=[voxel_vec_list;voxel_box(:)'];
x_dir_list=[x_dir_list;x_dir];
y_dir_list=[y_dir_list;y_dir];
z_dir_list=[z_dir_list;z_dir];

%voxel_vec_list=voxel_vec_list';
mg_list=[mg;mg;mg;];
end