function obj = Binarize(obj)
% if threshold is 0 then guess an intelligent threshold of 200 
if obj.state.display.binarythreshold==0
            obj.state.display.binarythreshold=70;
end
if (obj.state.display.ch1==1) && isfield(obj.data.ch(1),'filteredArray') && size(obj.data.ch(1).filteredArray,2)>0
    ch=1;
    if ~obj.state.display.keepintermediates
        
        obj.data.ch(ch).filteredArray=obj.data.ch(ch).filteredArray>obj.state.display.binarythreshold;
        obj.data.ch(ch).binaryArray=obj.data.ch(ch).filteredArray;
        obj.data.ch(ch).filteredArray=[];
    else
        obj.data.ch(ch).binaryArray=obj.data.ch(ch).filteredArray>obj.state.display.binarythreshold;
    end
    obj.data.ch(ch).binaryArray=uint8(obj.data.ch(ch).binaryArray);
    % get rid small elements
    obj.data.ch(ch).binaryArray=bwareaopen(obj.data.ch(ch).binaryArray,10000);
end

if (obj.state.display.ch2==1) && isfield(obj.data.ch(2),'filteredArray') && size(obj.data.ch(2).filteredArray,2)>0
    ch=2;
    if ~obj.state.display.keepintermediates
        obj.data.ch(ch).filteredArray=obj.data.ch(ch).filteredArray>obj.state.display.binarythreshold;
        obj.data.ch(ch).binaryArray=obj.data.ch(ch).filteredArray;
        obj.data.ch(ch).filteredArray=[];
    else
        obj.data.ch(ch).binaryArray=obj.data.ch(ch).filteredArray>obj.state.display.binarythreshold;
    end
    obj.data.ch(ch).binaryArray=uint8(obj.data.ch(ch).binaryArray);
    % get rid small elements
    obj.data.ch(ch).binaryArray=bwareaopen(obj.data.ch(ch).binaryArray,10000);
end
