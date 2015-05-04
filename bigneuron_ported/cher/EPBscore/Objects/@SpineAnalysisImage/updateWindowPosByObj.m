function obj=updateWindowPosByObj(obj)
if isfield(obj.state.display,'windowpos')
   names=fieldnames(obj.state.display.windowpos);
   for i=1:size(names,1)
       if isfield(obj.gh,names(i)) && isfield(eval(['obj.gh.',char(names(i))]),'Figure')
           handle=eval(['obj.gh.',char(names(i)),'.Figure']);
           set(handle,'Units','pixels','Position',eval(['obj.state.display.windowpos.',char(names(i))]));
       end
   end
end