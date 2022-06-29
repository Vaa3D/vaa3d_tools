function save_v3d_apo_file(m_struct, filename)
%function save_v3d_apo_file(m_struct, filename)
%
% Save the .apo point cloud format data file used in V3D
% 
% m_struct will consist of point cloud coordinates and other information
% (e.g. name/comments/types/shape)
%
% V3D website: see software page of http://penglab.janelia.org
%
% by Hanchuan Peng
% 20090724


save_v3d_pointcloud_file(m_struct, filename);
