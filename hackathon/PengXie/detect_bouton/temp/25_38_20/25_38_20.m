im_pth='25_38_20/Images/';
im_id='25_38_20.tif'; 
trace_pth='25_38_20/Traces/';
profile_file='25_38_20/Profiles'; 
results_file='25_38_20/Results'; 
ch=1;

[AxonMat, Profile] = cmd_BoutonAnalyzer(im_pth, im_id, trace_pth, profile_file, results_file, ch);

