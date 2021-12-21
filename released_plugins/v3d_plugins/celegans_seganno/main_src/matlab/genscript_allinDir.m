%
% revised by Hanchuan Peng, 20110524 for testing on pengh-wm1
%

%generate the script for all images in current directory
%for standford_326 data
clc
clear all
close all


%%
%dir_input='/groups/peng/home/qul/myhome/work/0.data/celegans_anno/stanford_326/stanford_326_extracted';
%dir_output='/groups/peng/home/qul/myhome/work/0.data/celegans_anno/stanford_326/stanford_326_extracted_result';

dir_input='/opt/home/hanchuan/Desktop/SRS_data/Cell2009_175';
dir_output='/opt/home/hanchuan/Desktop/SRS_testres';

%generate the script file for every valid file in the given dir
rootdir=dir(dir_input);

n=0;
for i=1:length(rootdir)
    if(strfind(rootdir(i).name,'_crop_straight.raw'))
        n=n+1;
        fprintf('[%d]: %s\n',n,rootdir(i).name);
        
        filefullname=[dir_input,'/',rootdir(i).name];
        
        genscript_1(n,filefullname,dir_output);
                
    end
end

%%
% %generate the script the submit all scripts 
% allin1_scriptname='alljob.sh';
% 
% fid=fopen(allin1_scriptname,'w');
% 
% %print header
% fprintf(fid,'#!/bin/sh\n');
% fprintf(fid,'#This script is generated automatically by genscript_allinDir\n');
% fprintf(fid,'#Example:\n');
% fprintf(fid,'#ssh login2\n');
% fprintf(fid,'#http://wiki/wiki/display/ScientificComputing/New+JFRC+Compute+Cluster\n');
% fprintf(fid,'#sh alljob.sh\n\n');
% 
% for i=1:n
% %     fprintf(fid,'qsub -V -e /groups/peng/home/qul/work/Rex/align/error -o /groups/peng/home/qul/work/Rex/align/out /groups/peng/home/qul/work/Rex/align/scripts/job_%d.sh\n',i);
%     fprintf(fid,'sh job_%d.sh\n',i);
% end
% 
% fclose(fid);


%%
%generate the script the submit all scripts 
n_script=10;
n_jobinscript=floor(n/n_script);

for k=1:n_script
    allin1_scriptname=['alljob_',num2str(k),'.sh'];

    fid=fopen(allin1_scriptname,'w');

    %print header
    fprintf(fid,'#!/bin/sh\n');
    fprintf(fid,'#This script is generated automatically by genscript_allinDir\n');
    fprintf(fid,'#Example:\n');
    fprintf(fid,'#ssh login2\n');
    fprintf(fid,'#http://wiki/wiki/display/ScientificComputing/New+JFRC+Compute+Cluster\n');
    fprintf(fid,'#sh alljob.sh\n\n');

    if(k~=n_script)
        for i=((k-1)*n_jobinscript+1):k*n_jobinscript
        %     fprintf(fid,'qsub -V -e /groups/peng/home/qul/work/Rex/align/error -o /groups/peng/home/qul/work/Rex/align/out /groups/peng/home/qul/work/Rex/align/scripts/job_%d.sh\n',i);
            fprintf(fid,'sh job_%d.sh\n',i);
        end
    else
         for i=((k-1)*n_jobinscript+1):n
        %     fprintf(fid,'qsub -V -e /groups/peng/home/qul/work/Rex/align/error -o /groups/peng/home/qul/work/Rex/align/out /groups/peng/home/qul/work/Rex/align/scripts/job_%d.sh\n',i);
            fprintf(fid,'sh job_%d.sh\n',i);
        end
   end

    fclose(fid);
end
