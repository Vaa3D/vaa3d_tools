function f=parentFigure(h)
% Find the parent figure of a handle
f=h;
while get(f,'Parent')~=0
    f=get(f,'Parent');
end