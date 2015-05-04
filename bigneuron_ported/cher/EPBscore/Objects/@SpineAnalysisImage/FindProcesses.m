function obj = FindProcesses(obj)
if (obj.state.display.ch1==1) && isfield(obj.data.ch(1),'binaryArray')
    ch=1;
end

if (obj.state.display.ch2==1) && isfield(obj.data.ch(2),'binaryArray')
    ch=2;
end



    obj.data.burnArray=findprocess('burn',{uint8(~obj.data.ch(ch).binaryArray)});
    obj.data.burnArray=obj.data.burnArray{1};
        
    obj.data.maset=findprocess('thin',{obj.data.burnArray(:,:,max(obj.state.ROI.minz,1):min(obj.state.ROI.maxz,size(obj.data.burnArray,3)))});
    obj.data.maset=obj.data.maset{1};
    % trimset parameters burncube maset
    % LL_PATH_TOL,BL_PATH_TOL,usefast,ntol,do_cmrg
    obj.data.trimset=findprocess('trim',{obj.data.burnArray(:,:,max(obj.state.ROI.minz,1):min(obj.state.ROI.maxz,size(obj.data.burnArray,3))), obj.data.maset, uint8(obj.parameters.threedma.isolatedpathtol), uint8(obj.parameters.threedma.branchpathtol), ...
                uint8(1),uint8(obj.parameters.threedma.looptol),uint8(1)});
    obj.data.trimset=obj.data.trimset{1};
    % dendrites parameters burncube maset max_den ntol do_cmrg brk_den
    % dp_tol um_x um_y um_z loop_tol
    dendrites=findprocess('dendrite',{obj.data.burnArray(:,:,max(obj.state.ROI.minz,1):min(obj.state.ROI.maxz,size(obj.data.burnArray,3))), obj.data.trimset, uint8(obj.parameters.threedma.maxdendritenum), uint8(obj.parameters.threedma.looptol),uint8(1), uint8(0), ...
        obj.parameters.threedma.maxkinkangle, obj.parameters.xspacing, obj.parameters.yspacing, obj.parameters.zspacing, obj.parameters.threedma.maxlooplength});
    dendrites=dendrites{1};
        
    

    
    % the C program uses indices starting from 0. This needs to be converted
    % to MATLAB convention which starts from 1
    [x,y,z]=ind2sub(size(obj.data.burnArray(:,:,max(obj.state.ROI.minz,1):min(obj.state.ROI.maxz,size(obj.data.burnArray,3)))),obj.data.maset);    
    z=z+max(obj.state.ROI.minz,1)-1; % get back the correct z value
    obj.data.maset=uint16([x';y';z';uint32(obj.data.burnArray(sub2ind(size(obj.data.burnArray),x,y,z))')]);
    
    [x,y,z]=ind2sub(size(obj.data.burnArray(:,:,max(obj.state.ROI.minz,1):min(obj.state.ROI.maxz,size(obj.data.burnArray,3)))),obj.data.trimset);    
    z=z+max(obj.state.ROI.minz,1)-1; % get back the correct z value
    obj.data.trimset=uint16([x';y';z';uint32(obj.data.burnArray(sub2ind(size(obj.data.burnArray),x,y,z))')]);
    
    
    obj.data.maset=double(obj.data.maset)+[ones(1,size(obj.data.maset,2));ones(1,size(obj.data.maset,2));ones(1,size(obj.data.maset,2));zeros(1,size(obj.data.maset,2))]; 
    obj.data.trimset=double(obj.data.trimset)+[ones(1,size(obj.data.trimset,2));ones(1,size(obj.data.trimset,2));ones(1,size(obj.data.trimset,2));zeros(1,size(obj.data.trimset,2))];    
    try
        obj.data.dendrites=struct('voxel',[]);
        for i=1:size(dendrites,1)
            [x,y,z]=ind2sub(size(obj.data.burnArray(:,:,max(obj.state.ROI.minz,1):min(obj.state.ROI.maxz,size(obj.data.burnArray,3))))+2,dendrites{i});
            z=z+max(obj.state.ROI.minz,1)-1; % get back the correct z value
            obj.data.dendrites(i)=struct('voxel',uint16([x'-1;y'-1;z'-1;uint32(obj.data.burnArray(sub2ind(size(obj.data.burnArray),x-1,y-1,z-1))')]));
        end

        for i=1:size(obj.data.dendrites,2)
            obj.data.dendrites(i).start=obj.data.dendrites(i).voxel(1:3,1);
            obj.data.dendrites(i).end=obj.data.dendrites(i).voxel(1:3,end);


            % order neurties so it goes from left to right top to bottom
            if (obj.data.dendrites(i).end(1)-obj.data.dendrites(i).start(1)-obj.data.dendrites(i).start(2)+obj.data.dendrites(i).end(2)<0)
                temp=obj.data.dendrites(i).start;
                obj.data.dendrites(i).start=obj.data.dendrites(i).end;
                obj.data.dendrites(i).end=temp;
                obj.data.dendrites(i).voxel=fliplr(obj.data.dendrites(i).voxel);
            end
            obj.data.dendrites(i).totalpixel=size(obj.data.dendrites(i).voxel,2);
        end
        [len,order]=sort([obj.data.dendrites.totalpixel]);   % sort dendrites according to length
        obj.data.dendrites=obj.data.dendrites(flipdim(order,2));
        if ~obj.state.display.keepintermediates
            obj.data.burnArray=[];
        end

    catch
        disp(lasterr);
        disp(i);
    end
end
