function changefilenames(oldstring, newstring)
a=dir; 
for i=1:size(a,1)
k = strfind(a(i).name,oldstring);
if k
    newname=a(i).name;
    newname=[newname(1:k-1) newstring newname(k+length(oldstring):end)];
    movefile(a(i).name,newname);
end
end