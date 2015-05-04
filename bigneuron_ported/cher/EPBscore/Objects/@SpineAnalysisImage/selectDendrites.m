function obj = selectDendrites(obj)
numstr=char(obj.state.display.editbox);
if (numstr)
    dendrites=[];
    rem=numstr;
    while(rem)
        [token,rem]=strtok(rem,[' ',',',';']);
        [den,ok]=str2num(token);
        if (ok) & (den>0)
            dendrites=[dendrites den];
        else
            if token(1)~='-'
                
                den=double(token(1))-64;
            else
                den=64-double(token(2));
            end
            dendrites=[dendrites den];
        end
    end
    newdendrites=[];
    if isfield(obj.data,'dendrites') && size(obj.data.dendrites,2)>0    
        for i=1:size(dendrites,2)
            if (abs(dendrites(i))>0) & (abs(dendrites(i))<=size(obj.data.dendrites,2))
                newdendrites=[newdendrites obj.data.dendrites(abs(dendrites(i)))];
                if dendrites(i)<0
                    temp=newdendrites(end).start;  
                    newdendrites(end).start=newdendrites(end).end;
                    newdendrites(end).end=temp;
                    newdendrites(end).voxel=fliplr(newdendrites(end).voxel);
                    
                    if isfield(newdendrites(end),'voxelmax')
                    newdendrites(end).voxelmax=fliplr(newdendrites(end).voxelmax);
                    newdendrites(end).dx=fliplr(newdendrites(end).dx);
                    newdendrites(end).dy=fliplr(newdendrites(end).dy);
                    newdendrites(end).maxint=fliplr(newdendrites(end).maxint);
                    newdendrites(end).maxintraw=fliplr(newdendrites(end).maxintraw);
                    newdendrites(end).medianfiltered=fliplr(newdendrites(end).medianfiltered);
                    newdendrites(end).length=max(newdendrites(end).length)-fliplr(newdendrites(end).length);
                    end
                    dendrites(i)=abs(dendrites(i));     
                end
            end
        end
%         for i=1:size(newdendrites,2)
%             newdendrites(i).index=i;
%         end
        %  [len,order]=sort([obj.data.dendrites.length]);   % sort dendrites according to length
        %  obj.data.dendrites=obj.data.dendrites(flipdim(order,2));
    end
    if isfield(obj.data.dendrites(dendrites(1)),'maxintcorrected')
        obj.data.dendrites(dendrites(1)).maxintcorrected=[];
        obj.data.dendrites(dendrites(1)).maxintcube=[];
        obj.data.dendrites(dendrites(1)).medianfiltered=[];
        obj.data.dendrites(dendrites(1)).meanbacksingle=[];
        obj.data.dendrites(dendrites(1)).maxintsingle=[];
        obj.data.dendrites(dendrites(1)).thres=[];
        obj.data.dendrites(dendrites(1)).meanback=[];
    end
    
    
    
    %obj.data.spines=[];
    % newspines=[];
    obj.data.dendrites=newdendrites;

    % clean up spines and marks
    
    try
        newspines=[];
    if isfield(obj.data,'spines') && size(obj.data.spines,2)>0
        for i=1:size(obj.data.spines,2)
            den_ids=[];
            if sum(abs(dendrites)==obj.data.spines(i).den_ind)>0
                findden=find([abs(dendrites)]==obj.data.spines(i).den_ind);
                obj.data.spines(i).den_ind=findden(1);
                newspines=[newspines obj.data.spines(i)];
            end
        end
        % sort according to dendrites
        [id,order]=sort([newspines.den_ind]);
        obj.data.spines=newspines(order);
    end        
    catch
        i
    end
    
    try
        newmarks=[];
    if isfield(obj.data,'marks') && size(obj.data.marks,2)>0
        for i=1:size(obj.data.marks,2)
            den_ids=[];
            if sum(abs(dendrites)==obj.data.marks(i).den_ind)>0
                findden=find([abs(dendrites)]==obj.data.marks(i).den_ind);
                obj.data.marks(i).den_ind=findden(1);
                newmarks=[newmarks obj.data.marks(i)];
            end
        end
        % sort according to dendrites
        [id,order]=sort([newmarks.den_ind]);
        obj.data.marks=newmarks(order);
    end        
    catch
        i
    end
    % try
    % if isfield(obj.data,'spines') && size(obj.data.spines,2)>0
    %     for i=1:size(obj.data.spines,2)
    %         den_ids=[];
    %         if sum(dendrites==obj.data.spines(i).den_ind)>0
    %             newspines=[newspines obj.data.spines(i)];
    %             findden=find([obj.data.dendrites.index]==obj.data.spines(i).den_ind);
    %             den_ids=[den_ids findden(1)];
    %         end
    %     end
    %     % sort according to dendrites
    %     [id,order]=sort(den_ids);
    %     obj.data.spines=newspines(order);
    % end        
    % catch
    %     i
    % end
end