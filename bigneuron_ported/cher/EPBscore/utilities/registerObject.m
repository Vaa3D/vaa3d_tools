function h=registerObject(o)
% Register Objects in the global array
global objects
if size(objects,1)<1
    o=set(o,'ID',1);
    objects=[struct('index',1,'object',o,'class',class(o))];
    h=['objects([objects.index]==1).object'];
else
    index=objects(end).index;
    o=set(o,'ID',index+1);
    objects=[objects struct('index',index+1,'object',o,'class',class(o))];
    h=['objects([objects.index]==' num2str(index+1) ').object'];
end