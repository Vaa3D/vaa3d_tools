clear all;
close all;

filename = dir('/local4/Data/IVSCC_test/comparison/Caffe_testing/prediction/bkg/*.mat');
ana = [];
fore_index_all=[];
bkg_index_all=[];
for i = 2:length(filename)
    s_name = [filename(i).name(1:end-8),'.mat']
    load(['/local4/Data/IVSCC_test/comparison/Caffe_testing/prediction/singal/',s_name]);
    acc_fore=sum(predi_index==2)/length(predi_index);
    fore_index_all=[fore_index_all;predi_index];
    load(['/local4/Data/IVSCC_test/comparison/Caffe_testing/prediction/bkg/',filename(i).name]);
    acc_bkg=sum(predi_index==1)/length(predi_index);
    bkg_index_all=[bkg_index_all;predi_index];
    ana = [ana;length(predi_index) acc_fore acc_bkg];
end
acc_fore_all = sum(fore_index_all==2)/length(fore_index_all)
acc_bkg_all = sum(predi_index==1)/length(predi_index)