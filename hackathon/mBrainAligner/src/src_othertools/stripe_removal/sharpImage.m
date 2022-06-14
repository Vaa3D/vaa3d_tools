% This program is used for image sharpening.

function p = sharpImage(image)
    [row,col,depth]=size(image);
    for k=1:depth
        Image=im2double(image(:,:,k));
        sigma=10;
        result=imgaussfilt(Image,sigma);
        blurImage = 2*Image-result;
        p(:,:,k)=uint8(blurImage*255);
    end
end