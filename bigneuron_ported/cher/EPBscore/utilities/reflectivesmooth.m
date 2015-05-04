function out=reflectivesmooth(in,span)
[maxval,maxdim]=max(size(in));
if max(size(in)>span)
    out=smooth(cat(maxdim,flipdim(in(1:span),maxdim), in, flipdim(in(end-span+1:end),maxdim)),span);
    %out=smooth(cat(maxdim,flipdim(in(1:span),maxdim),flipdim(in(end-span+1:end),maxdim)),span);
    if (maxdim==2)
        out=out';
    end
    out=double(out(span+1:end-span));
end
   
%                 maxintcube=smooth([fliplr(maxintcube(1:14)) maxintcube fliplr(maxintcube(end-13:end))],7)';       % smooth the skeleton with reflecting boundary condition
%                 maxintcube=double(maxintcube(15:end-14));