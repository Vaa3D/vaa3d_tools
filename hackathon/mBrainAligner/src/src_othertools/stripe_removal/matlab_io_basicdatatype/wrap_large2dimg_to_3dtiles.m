function [ img3d ] = wrap_large2dimg_to_3dtiles( img2d, tile_szx, tile_szy, fillvalue )
%a utility function to wrap a large 2D image to 3D stack, using the size
%specified in tile_szx and tile_szy
%
% The additional margin is filled using 'fillvalue', which is 0 by default
%
% by Hanchuan Peng
% 2013-02-21

sz = size(img2d);
if (length(sz)~=2 & length(sz)~=3),
    error('The input is not a 2D or 3D image');
    return;
end;

if nargin<4,
    fillvalue = 0;
end;

if nargin<3,
    tile_szy = 512;
end;

if nargin<2,
    tile_szx = 512;
end;

if (tile_szx<128 | tile_szy<128),
    error('It seems your tile size is too small... do nothing.');
    return;
end;

ntx = ceil(sz(1)/tile_szx);
nty = ceil(sz(2)/tile_szy);

% enlarge the margin if needed

if (ntx*tile_szx ~= sz(1))
    fprintf('Add margin for X\n');
    if (length(sz)==2)
        img2d(sz(1)+1:ntx*tile_szx, :) = fillvalue;
    else,
        img2d(sz(1)+1:ntx*tile_szx, :, :) = fillvalue;
    end;
end;

if (nty*tile_szy ~= sz(2))
    fprintf('Add margin for Y\n');
    if (length(sz)==2)
        img2d(:, sz(2)+1:nty*tile_szy) = fillvalue;
    else,
        img2d(:, sz(2)+1:nty*tile_szy, :) = fillvalue;
    end;
end;

% preset data for faster copying. This step can certainly be improved, for
% I simply use it here for simplicity (and thus use more memory)

img3d = repmat(img2d(1:tile_szx, 1:tile_szy, 1), [1 1 ntx*nty, sz(3)]); 

% now start to copy over data

k = 0;
for j=1:nty,
    for i=1:ntx,
        
        k=k+1;
        fprintf('Tile [%d, %d] with the total index number [%d / %d]\n', i, j, k, ntx*nty);
        
        sposx = (i-1)*tile_szx + 1;
        eposx = i*tile_szx;
        sposy = (j-1)*tile_szy + 1;
        eposy = j*tile_szy;
        
        if (length(sz)==2),
            img3d(:,:,k) = img2d(sposx:eposx, sposy:eposy);
        else,
            for n=1:sz(3),
                img3d(:,:,k, n) = img2d(sposx:eposx, sposy:eposy, n);
            end;
        end;
    end;
end;
    
return;

end

