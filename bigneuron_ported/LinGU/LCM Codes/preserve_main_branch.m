function p90 = preserve_main_branch(p90)


sz_I = size(p90);

CC = bwconncomp(p90 > 0,6);

numPixels = cellfun(@numel,CC.PixelIdxList);

[biggest,idx] = max(numPixels);

p90 = zeros(sz_I);

p90(CC.PixelIdxList{idx}) = 1;

end
