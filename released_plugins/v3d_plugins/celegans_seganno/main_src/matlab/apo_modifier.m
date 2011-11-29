%apo modifier
clc
clear all
close all

ratio=4;
foldername_output='/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/result_fullresolution/';

%read atlas apo file
for i=1:9
    i
    filename_apo=['noaffine',num2str(i),'.apo'];
    apo_atlas=load_v3d_apo_file(filename_apo);
    
    for j=1:length(apo_atlas)
        %rescale and revert xy axis
        tmp=apo_atlas{j}.x;
        apo_atlas{j}.x=apo_atlas{j}.y*ratio;
        apo_atlas{j}.y=tmp*ratio;
        apo_atlas{j}.z=apo_atlas{j}.z*ratio;
        apo_atlas{j}.volsize=apo_atlas{j}.volsize*40;
    end
    
%     filename_apo_output=['/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/result_fullresolution/noaffine',num2str(i),'.apo'];
    switch i
    case 1  
        filename_apo_output=[foldername_output,'cnd1threeL1_1213061.apo'];
    case 2
        filename_apo_output=[foldername_output,'cnd1threeL1_1228061.apo'];
    case 3
        filename_apo_output=[foldername_output,'cnd1threeL1_1229061.apo'];  %target
    case 4
        filename_apo_output=[foldername_output,'cnd1threeL1_1229062.apo'];
    case 5
        filename_apo_output=[foldername_output,'cnd1threeL1_1229063.apo'];
    case 6
        filename_apo_output=[foldername_output,'jkk1SD1439L1_0904072.apo'];
    case 7
        filename_apo_output=[foldername_output,'jkk1SD1439L1_0904073.apo'];
    case 8
        filename_apo_output=[foldername_output,'jkk1SD1439L1_0904074.apo'];
    case 9
        filename_apo_output=[foldername_output,'jkk1SD1439L1_0904075.apo'];
    end

    save_v3d_apo_file(apo_atlas,filename_apo_output);
end
