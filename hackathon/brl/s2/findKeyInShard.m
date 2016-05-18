
function out = findKeyInShard(stateShard,keystring,indexedValue)
elementList = [];
out = {};
outputIndex = 0;
for i=1:numel(stateShard)
    if sum(strfind(stateShard{i}.Attributes.key,keystring))>0
        elementList = [elementList,i];
        if isfield(stateShard{i}, 'IndexedValue')
            if isempty(indexedValue)
                ['key ', keystring,' requires indexed value']
                out = [];
                return
            else
                for j = 1:numel(stateShard{i}.IndexedValue)
                    if sum(strfind(stateShard{i}.IndexedValue{j}.Attributes.index, indexedValue))>0
                        outputIndex= outputIndex+1;
                        out{outputIndex}= stateShard{i}.IndexedValue{j}.Attributes.value;
                    end
                end
            end
        else
            outputIndex= outputIndex+1;
            out{outputIndex}= stateShard{i}.Attributes.value;
            
        end
    else
        
    end
end
if numel(out) ==0
    ['key ', keystring,' not found!']
    out = [];
    return
end

if numel(out)>1
    ['key ', keystring,' returned multiple values!']
end
