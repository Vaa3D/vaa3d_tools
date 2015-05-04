function updateGUIbyglobalAll(globalName)
% given the name of a global variable, find and update the GUI that
% contains it

[topName, structName, fieldName]=structNameParts(globalName);
eval(['global ' topName ';']);
names=fieldnames(eval(globalName));
for i=1:size(names,1)
    updateGUIbyglobal([globalName '.' names{i}]);
end
