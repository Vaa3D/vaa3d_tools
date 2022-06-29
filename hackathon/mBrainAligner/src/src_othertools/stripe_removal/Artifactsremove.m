% This program is used to remove artifact.Set a threshold and remove the
% gray value less than the threshold.

function A = Artifactsremove(image,Artifact)
%clc
%clear all
%close all
% Artifact=30;
[row,col,depth]=size(image);
for k=1:depth
    Image=image(:,:,k);                
    for i=1:row
        for j=1:col
            if Image(i,j)<=Artifact
                Image(i,j)=0;
            end
        end
    end
    A(:,:,k)=uint8(Image);
end
end