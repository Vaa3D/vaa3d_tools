function obj = selectDendrites(obj)
numstr=char(obj.state.display.editbox);
if (numstr)
dendrites=[];
rem=numstr;
while(rem)
    [token,rem]=strtok(rem,[' ',',',';']);
    den=str2num(token);
    if (den>0)
        dendrites=[dendrites den];
    end
end
newdendrites=[];
if isfield(obj.data,'dendrites') && size(obj.data.dendrites,2)>0    
    for i=1:size(obj.data.dendrites,2)
        if sum(dendrites==obj.data.dendrites(i).index)>0
            newdendrites=[newdendrites obj.data.dendrites(i)];
        end
    end
    obj.data.dendrites=newdendrites;
  %  [len,order]=sort([obj.data.dendrites.length]);   % sort dendrites according to length
  %  obj.data.dendrites=obj.data.dendrites(flipdim(order,2));
end
newspines=[];
if isfield(obj.data,'spines') && size(obj.data.spines,2)>0
    for i=1:size(obj.data.spines,2)
        den_ids=[];
        if sum(dendrites==obj.data.spines(i).den_ind)>0
            newspines=[newspines obj.data.spines(i)];
            findden=find([obj.data.dendrites.index]==obj.data.spines(i).den_ind);
            den_ids=[den_ids findden(1)];
        end
    end
    % sort according to dendrites
    [id,order]=sort(den_ids);
    obj.data.spines=newspines(order);
end        
if isfield(obj.data,'dendrites') && size(obj.data.dendrites,2)>0   
    for i=1:size(obj.data.dendrites,2)
        obj.data.dendrites(i).index=i;  
    end
end
end
obj=CorrectDendrites(obj);
obj.data.spines=[];