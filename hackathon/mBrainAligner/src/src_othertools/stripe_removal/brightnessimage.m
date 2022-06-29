% This program is used for brightness optimization.

function b = brightnessimage(image)

v_sort = sort(image(:));
v_len = length(v_sort);
clipmin = v_sort(int32(v_len*0.01));
clipmax = v_sort(int32(v_len*0.99));

fillminvalueidx = image<clipmin;
fillmaxvalueidx = image>clipmax;

x2=clipmin; y2=0;
x1=clipmax; y1=255;
image = (image-x2).*((y1-y2)/(x1-x2))+y2;

image(fillmaxvalueidx)=255;
image(fillminvalueidx)=0;

b=uint8(image);
end
