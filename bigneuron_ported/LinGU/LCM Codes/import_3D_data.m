function [I,gt] = import_3D_data(I_fn)

% import the 3D data from the given filename


if(strcmp(I_fn(end - 2:end),'mat'))

    load(I_fn);
    
    if(~exist('I'))
        
        I = img;
        
    end
    
%     exist
    
    gt = zeros(size(I));
    
%     gt = gt';

elseif(strcmp(I_fn(end - 2:end),'raw'))
    

    I = load_v3d_raw_img_file(I_fn);

    gt = zeros(size(I));
    
elseif(strcmp(I_fn(end - 3:end),'nrrd'))
    
    I = nrrdread(I_fn);
    
    gt = zeros(size(I));
    

end