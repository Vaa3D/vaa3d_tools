function val = get(a,prop_name)
% GET Get asset properties from the specified object
% and return the value
val=eval(['a.' prop_name ';']);