function a=copystruct(a,b)
names=fieldnames(b);
for i=1:size(names,1)
    a.(char(names(i)))=b.(char(names(i)));
end