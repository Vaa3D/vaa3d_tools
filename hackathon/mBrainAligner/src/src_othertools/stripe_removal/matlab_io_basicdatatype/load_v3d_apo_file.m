function m_struct = load_v3d_apo_file(filename)
%function m_struct = load_v3d_apo_file(filename)
%
% Load the .apo point cloud format data file used in V3D
% 
% m_struct will consist of point cloud coordinates and other information
% (e.g. name/comments/types/shape)
%
% V3D website: see software page of http://penglab.janelia.org
%
% by Hanchuan Peng
% 20090724


m_struct = load_v3d_pointcloud_file(filename);
