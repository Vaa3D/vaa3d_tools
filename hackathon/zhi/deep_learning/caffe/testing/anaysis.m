clear all;
close all;

filename = dir('/local4/Data/IVSCC_test/comparison/Caffe_testing_3nd/train_package_4th/prediction/all*');
neuroname = dir('/local4/Data/IVSCC_test/comparison/3D_training_patches_mip/*_z15');

ana = [];
fore_index_all=[];
bkg_index_all=[];
for i = 1:length(filename)
    i
    load(['/local4/Data/IVSCC_test/comparison/Caffe_testing_3nd/train_package_4th/prediction/',filename(i).name]);
    tifname = dir(['/local4/Data/IVSCC_test/comparison/3D_training_patches_mip/',neuroname(i).name,'/*.tif']);

    for j = 1:length(predi_index)
       if( predi_index(j)==1)           
           copyfile(['/local4/Data/IVSCC_test/comparison/3D_training_patches_mip/',neuroname(i).name,'/',tifname(j).name],['/local4/Data/IVSCC_test/comparison/3D_training_patches_mip_false_nagtive/',neuroname(i).name,'/',tifname(j).name])
       end      
        
    end
%     acc_fore=sum(predi_index==2)/length(predi_index);
%     num_fore = length(predi_index);
%     ana = [ana; num_fore acc_fore];

%     load(['/local4/Data/IVSCC_test/comparison/Caffe_testing_3nd/train_package_4th/prediction_390000/',filename(i).name,'_foreground.mat']);
%     acc_fore=sum(predi_index==2)/length(predi_index);
%     num_fore = length(predi_index);
%     
%     load(['/local4/Data/IVSCC_test/comparison/Caffe_testing_3nd/train_package_4th/prediction_390000/',filename(i).name,'_background.mat']);
%     acc_bkg=sum(predi_index==1)/length(predi_index);
%     num_bkg = length(predi_index);
%     ana = [ana; num_fore acc_fore num_bkg acc_bkg]; 

end

acc_fore_all = sum(ana(:,1).*ana(:,2))/sum(ana(:,1))
%acc_bkg_all = sum(ana(:,3).*ana(:,4))/sum(ana(:,3))
