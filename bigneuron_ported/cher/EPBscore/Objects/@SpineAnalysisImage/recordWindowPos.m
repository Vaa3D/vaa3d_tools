function obj=recordWindowPos(obj)
if ~isfield(obj.state.display,'windowpos')
    obj.state.display.windowpos={};
end
names=fieldnames(obj.gh);
for i=1:size(names,1)
    handles=eval(['obj.gh.',char(names(i))]);
    if (isfield(handles,'Figure')) && isfield(eval(['obj.gh.',char(names(i))]),'Figure')
        set(handles.Figure,'Units','pixels');
        pos=get(handles.Figure,'Position');
        eval(['obj.state.display.windowpos.',char(names(i)),'=pos;']);
    end
end
