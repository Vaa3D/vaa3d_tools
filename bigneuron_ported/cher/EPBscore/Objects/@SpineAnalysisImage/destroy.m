function destroy(obj)
% unregister obj
global objects;

% prompt to save data

% close all windows
names=fieldnames(obj.gh);
for i=1:size(names,1)
    handles=eval(['obj.gh.',char(names(i))]);
    if (isfield(handles,'Figure'))
       try
           delete(handles.Figure);
       catch 
       end
    end
end

if (size(objects,2)>1)
    objects([objects.index]==obj.ID)=[];
else
    objects=[];
end
ogh=[];
self=[];
