function obj = ShiftDendriteMaxObj(obj,restricted)
try
    %     if (obj.state.display.ch1==1) && isfield(obj.data.ch(1),'filteredArray')
    %         ch=1;
    %         chdual=2;
    %     end
    %
    %     if (obj.state.display.ch2==1) && isfield(obj.data.ch(2),'filteredArray')
    %         ch=2;
    %         chdual=1;
    %     end
    
    ch=1;
    for i=1:size(obj.data.dendrites,2)
        if isempty(obj.data.dendrites(i).voxel)
            obj.data.dendrites(i)=[];
            break;
        end
        voxel=obj.data.dendrites(i).voxel;
        if ~restricted
            [voxel, dx, dy]=shiftDendriteMax(double(voxel),obj.data.ch(ch).gaussianArray(:,:,:,1),restricted);%,double(obj.data.dendrites(i).voxel));
            
            %      else
            %           voxel=obj.data.dendrites(i).voxel;
            
        end
        oldvoxel=voxel;
        for j=1:10
            [voxel, dx, dy]=shiftDendriteMax(voxel,obj.data.ch(ch).gaussianArray(:,:,:,1),1);%,oldvoxel);
            %[voxel, dx, dy]=shiftDendriteMax(voxel,obj.data.ch(ch).gaussianArray(:,:,:,1),1,oldvoxel)
        end
        [maxint_t,cube_t]=getbackboneintensity(voxel,obj.data.ch(ch).gaussianArray,dx,dy,0,1,0);
        [maxint,cube]=getbackboneintensity(voxel,obj.data.ch(ch).gaussianArray(:,:,:,1),dx,dy,0,1,0);
        [maxint1,cube1]=getbackboneintensity(voxel,obj.data.ch(ch).gaussianArray(:,:,:,2),dx,dy,0,1,0);
        [maxint2,cube2]=getbackboneintensity(voxel,obj.data.ch(ch).gaussianArray(:,:,:,3),dx,dy,0,1,0);
        try
            [maxint3,cube3]=getbackboneintensity(voxel,obj.data.ch(ch).gaussianArray(:,:,:,4),dx,dy,0,1,0);
        catch
        end
        aa=obj.data.ch(ch).gaussianArray(:,:,:,1);
        medianimage=double(median(aa(:)))/16;
        
        obj.data.dendrites(i).voxel=voxel;
        obj.data.dendrites(i).voxelmax=voxel;
        maxint=double(maxint)/16-medianimage;
        obj.data.dendrites(i).maxintraw=maxint;
        obj.data.dendrites(i).maxint=maxint;
        obj.data.dendrites(i).maxint1=maxint1/16-medianimage;
        obj.data.dendrites(i).maxint2=maxint2/16-medianimage;
        try
            obj.data.dendrites(i).maxint3=maxint3/16-medianimage;
        catch
        end
        obj.data.dendrites(i).cube=cube;
        obj.data.dendrites(i).cube1=cube1;
        obj.data.dendrites(i).cube2=cube2;
        try
            obj.data.dendrites(i).cube3=cube3;
        catch
        end
        obj.data.dendrites(i).dx=dx;
        obj.data.dendrites(i).dy=dy;
        obj.data.dendrites(i).start=double(obj.data.dendrites(i).voxel(1:3,1))';
        obj.data.dendrites(i).end=double(obj.data.dendrites(i).voxel(1:3,end))';
        obj.data.dendrites(i).sizeofaxon=size(voxel,2);
        
        % obselete code for dealing with second color
        %        if (size(obj.data.ch,2)>1) && (obj.state.display.ch2==1) && (obj.state.display.ch1==1)
        %           if (obj.parameters.threedma.shiftmaxuseraw)
        %             [voxel, voxelmax, maxintfilteredcorrected, dx, dy]=shiftDendriteMax(double(obj.data.dendrites(i).voxel),obj.data.ch(chdual).imageArray,restricted);
        %             [maxint,voxelmax1,cube]=getbackboneintensity(voxel,obj.data.ch(ch).imageArray,dx,dy,0,1,0);
        %           else
        %             [voxel, voxelmax, maxintfilteredcorrected, dx, dy]=shiftDendriteMax(double(obj.data.dendrites(i).voxel),obj.data.ch(chdual).gaussianArray,restricted);
        %             [maxint,voxelmax1,cube]=getbackboneintensity(voxel,obj.data.ch(ch).gaussianArray,dx,dy,0,1,0);
        %           end
        %        obj.data.dendrites(i).voxeldual=voxeldual;
        %        obj.data.dendrites(i).voxelmaxdual=voxelmaxdual;
        %        obj.data.dendrites(i).maxintrawdual=double(maxintdual)-double(obj.
        %        data.ch(chdual).imagemedian);
        %        obj.data.dendrites(i).maxintdual=reflectivesmooth(obj.data.dendrites(i).maxintrawdual,7);
        %        end
        
        % median filtering
        % cut off tall peaks before filtering
        
        
        % try this for now
        % use an interative process to find the median
        % the criterion is that
        % median=median(maxint(maxint<2*median)
        
    end
    for i=1:size(obj.data.dendrites,2)    
        % correct spines
        
        if (isfield(obj.data,'spines') && max(size(obj.data.spines))>0)
            for j=1:size(obj.data.spines,2)
                den_ind=obj.data.spines(j).den_ind;
                
                %Calculate distances
                dendrite=double(obj.data.dendrites(den_ind).voxel);
                spine=double(obj.data.spines(j).voxel);
                dis=sqrt((dendrite(1,:)-spine(1,2)).*(dendrite(1,:)-spine(1,2))+(dendrite(2,:)-spine(2,2)).*(dendrite(2,:)-spine(2,2))+(dendrite(3,:)-spine(3,2)).*(dendrite(3,:)-spine(3,2)));
                [t,ind]=min(dis);
                dendrite=double(obj.data.dendrites(den_ind).voxel);
                obj.data.spines(j).den_ind=den_ind;
                if (obj.data.spines(j).len==0)
                    maxint=obj.data.dendrites(den_ind).maxint;
                    [t,maxind]=max(double(maxint(max(1,ind-5):min(ind+5,length(maxint)))));
                    ind=ind+maxind-6;
                    ind=min(max(1,ind),length(maxint));                    
                    obj.data.spines(j).dendis=ind;
                    
                    obj.data.spines(j).voxel(1:3,1)=uint16(dendrite(1:3,ind));
                    
                    obj.data.spines(j).dendis=ind;
                    obj.data.spines(j).intensity=obj.data.dendrites(den_ind).maxint(ind);
                else
                    obj.data.spines(j).voxel(:,1)=uint16(dendrite(1:3,ind));
                    spine=double(obj.data.spines(j).voxel);
                    obj.parameters.xspacing=0.083;
                    obj.parameters.yspacing=0.083;
                    obj.parameters.zspacing=1;
                    xs= obj.parameters.xspacing;
                    ys=obj.parameters.yspacing;
                    zs=obj.parameters.zspacing;
                    dis=sqrt((dendrite(1,:)-spine(1,2)).*(dendrite(1,:)-spine(1,2))*ys*ys+(dendrite(2,:)-spine(2,2)).*(dendrite(2,:)-spine(2,2))*xs*xs+(dendrite(3,:)-spine(3,2)).*(dendrite(3,:)-spine(3,2))*zs*zs);
                    [a,ind]=min(dis);
                    
                    obj.data.spines(j).dendis=ind;
                    obj.data.spines(j).len=a;
                end
            end
            obj.data.slicespines={};            % Take care of slice spines for display
            for i=1:size(obj.data.ch(1).imageArray,3)
                ind=1;
                %   obj.data.slicespines(i)=struct([]);
                for j=1:size(obj.data.spines,2)
                    pos=find(obj.data.spines(j).voxel(3,:)==i-1);
                    if (pos)
                        obj.data.slicespines(i).spines(ind)=struct('voxel',[obj.data.spines(j).voxel(1,pos);obj.data.spines(j).voxel(2,pos)],'index',j);
                        ind=ind+1;
                    end
                end
            end
        end
        
        % correct marks
        if (isfield(obj.data,'marks') && max(size(obj.data.marks))>0)
            for j=1:size(obj.data.marks,2)
                den_ind=obj.data.marks(j).den_ind;
                
                dendrite=double(obj.data.dendrites(den_ind).voxel);
                spine=double(obj.data.marks(j).voxel);
                try % sometimes bug due to old versions
                    dis=sqrt((dendrite(1,:)-spine(1,2)).*(dendrite(1,:)-spine(1,2))+(dendrite(2,:)-spine(2,2)).*(dendrite(2,:)-spine(2,2))+(dendrite(3,:)-spine(3,2)).*(dendrite(3,:)-spine(3,2)));
                catch ME
                    dis=sqrt((dendrite(1,:)-spine(1,1)).*(dendrite(1,:)-spine(1,1))+(dendrite(2,:)-spine(2,1)).*(dendrite(2,:)-spine(2,1))+(dendrite(3,:)-spine(3,1)).*(dendrite(3,:)-spine(3,1)));
                end
                [t,ind]=min(dis);
                
                obj.data.marks(j).dendis=ind;
                obj.data.marks(j).voxel=uint16(dendrite(1:3,ind));
            end
        end
    end
catch ME
    disp(ME);
    disp(ME.stack(1));
    disp(i);
    
end