% function save_centralbrain(In_Filename, Out_Filename)
% by Benny Lam Sep 22, 2008
%
% Function to save the central brain after cutting off the optic lobes as
% the third channel mask.

function save_centralbrain(In_Filename, Out_Filename)

aa = loadRaw2Stack(In_Filename);

sz = size(aa);

for z=1:sz(3)
    for y=1:sz(2)
        for x=1:sz(1)
            if (aa(x,y,z,3)==254 || aa(x,y,z,3)==255)
                aa(x,y,z,1:2)=0;
            end
        end
    end
end

saveStack2File_c(aa(:,:,:,1:2),Out_Filename);