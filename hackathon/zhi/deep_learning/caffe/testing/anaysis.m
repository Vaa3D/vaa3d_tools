clear all;
close all;

filename = dir('/local4/Data/IVSCC_test/comparison/Caffe_testing_2nd/prediction/signal/*.mat');
ana = [];
fore_index_all=[];
bkg_index_all=[];
for i = 1:166%length(filename)
    i
    load(['/local4/Data/IVSCC_test/comparison/Caffe_testing_2nd/prediction/signal/',filename(i).name]);
    acc_fore=sum(predi_index==2)/length(predi_index);
    fore_index_all=[fore_index_all;predi_index];
    
    load(['/local4/Data/IVSCC_test/comparison/Caffe_testing_2nd/prediction/bkg/',filename(i).name]);
    bkg_fore=sum(predi_index==1)/length(predi_index);
    bkg_index_all=[bkg_index_all;predi_index];
    ana = [ana;length(predi_index) acc_fore bkg_fore];
%     ana = [ana;length(predi_index)];

end

acc_fore_all = sum(fore_index_all==2)/length(fore_index_all)
acc_bkg_all = sum(bkg_index_all==1)/length(bkg_index_all)
