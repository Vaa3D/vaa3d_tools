% revised by Hanchuan Peng, 20110524 for testing on pengh-wm1

%generate the script for celegans cell annotation
%for standford_326 data

function genscript_1(n,filefullname,dir_output)

%fixed files

%filename_atlas_input='/groups/peng/home/qul/myhome/work/0.data/celegans_anno/atlas_related/atlas.apo';

DATAHOMEDIR = '/opt/home/hanchuan/Desktop/SRS_data/celegans_atlas_533cells/';
PROGRAMHOMEDIR = '/opt/home/hanchuan/work/v3d_internal/qu_lei/celegans_seganno/';

filename_atlas_input=[DATAHOMEDIR 'atlas.apo'];

% filename_celloi='/groups/peng/home/qul/myhome/work/0.data/celegans_anno/atlas_related/celloi_muscle.txt';

%filename_celloi='/groups/peng/home/qul/myhome/work/0.data/celegans_anno/atlas_related/celloi_muscle_noSPH.txt';
%filename_aliaslist='/groups/peng/home/qul/myhome/work/0.data/celegans_anno/atlas_related/aliaslist.txt';

filename_celloi=[DATAHOMEDIR '/celloi_muscle.txt'];
filename_aliaslist=[DATAHOMEDIR '/aliaslist.txt'];

%find filename without dir and suffix
ind_dir=strfind(filefullname,'/');
ind_suffix=strfind(filefullname,'_crop_straight.raw');
filename_nodirsuffix=filefullname(ind_dir(end)+1:ind_suffix(end)-1);


%for annotation
filename_img=filefullname;
filename_atlas_output=[dir_output,'/',filename_nodirsuffix,'_anno_muscle.apo'];
filename_info_output=[dir_output,'/',filename_nodirsuffix,'_anno_info.txt'];
filename_seg_label = [dir_output,'/',filename_nodirsuffix,'_seglabel.raw'];

%for analysis
filename_manual_input=[filefullname(1:ind_dir(end)),'/',filename_nodirsuffix,'.ano.ano.apo'];
filename_analysis_output=[dir_output,'/',filename_nodirsuffix,'_anno_analysis.txt'];

%%
%generate the script file
scriptname=['job_',num2str(n),'.sh'];

fid=fopen(scriptname,'w');
fprintf(fid,'#!/bin/bash\n');
fprintf(fid,'#This script is generated automatically by genscript_1.m\n\n');

%do annotation
fprintf(fid,[PROGRAMHOMEDIR 'main_atlasguided_stranno  ']);
fprintf(fid,'-t %s  ',filename_img);
fprintf(fid,'-a %s  ',filename_atlas_input);
fprintf(fid,'-i %s  ',filename_celloi);
fprintf(fid,'-o %s  ',filename_atlas_output);
fprintf(fid,'-I %s  ',filename_info_output);
fprintf(fid,'-s %s  ',filename_seg_label);
fprintf(fid,'-m -1  ');
fprintf(fid,'-c 2  ');  %red:1, green:2, blue:3
fprintf(fid,'-f 3  ');
fprintf(fid,'-d 3 \n\n');

%do anno analysis
fprintf(fid,[PROGRAMHOMEDIR 'main_annoresultanalysis  ']);
fprintf(fid,'-a %s  ',filename_atlas_output);
fprintf(fid,'-m %s  ',filename_manual_input);
fprintf(fid,'-l %s  ',filename_aliaslist);
fprintf(fid,'-d 10  ');
fprintf(fid,'-o %s  ',filename_analysis_output);


fclose(fid);
