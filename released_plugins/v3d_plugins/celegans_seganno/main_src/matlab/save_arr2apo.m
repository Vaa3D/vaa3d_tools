%save 3xN arr_pos to apo file
function save_arr2apo(arr_pos,arr_name,filename)

for i=1:size(arr_pos,2)
    cellarr{i}.n=i;
    cellarr{i}.orderinfo=num2str(i);
    cellarr{i}.name=arr_name{i};
    cellarr{i}.comment='';
    cellarr{i}.pixmax=0;
    cellarr{i}.intensity=79;
    cellarr{i}.sdev=18;
    cellarr{i}.volsize=2;
    cellarr{i}.mass=34223;
    color.r=255;color.g=0;color.b=0;
    cellarr{i}.color=color;
%     cellarr{i}.x=cellarr_save(1,i);
%     cellarr{i}.y=cellarr_save(2,i);
    cellarr{i}.y=arr_pos(1,i);
    cellarr{i}.x=arr_pos(2,i);
    cellarr{i}.z=arr_pos(3,i);
end

save_v3d_apo_file(cellarr,filename);
