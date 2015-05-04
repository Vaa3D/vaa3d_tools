function obj = CorrectDendrites(obj)


for i=1:size(obj.data.dendrites,2)
    obj.data.dendrites(i).index=i;   % assign ID to dendrite
    
    
    
    
    %calculate axon length
    obj.parameters.yspacing=0.083;
    obj.parameters.xspacing=0.083;
    obj.parameters.zspacing=1;
    
    voxel=double(obj.data.dendrites(i).voxel);
    
    if (size(voxel,2)>10)
        % smoothing again to futher suppress jumps
        voxel(1,:)=medfilt1pad(voxel(1,:),10);
        voxel(2,:)=medfilt1pad(voxel(2,:),10);
        %        voxel(1,:)=smooth(voxel(1,:),10)';
        %        voxel(2,:)=smooth(voxel(2,:),10)';
    else
        voxel(1,:)=medfilt1pad(voxel(1,:),size(voxel,2));
        voxel(2,:)=medfilt1pad(voxel(2,:),size(voxel,2));
        %       voxel(1,:)=smooth(voxel(1,:),size(voxel,2))';
        %       voxel(2,:)=smooth(voxel(2,:),size(voxel,2))';
    end
    if (size(voxel,2)>100)
        voxel(3,:)=medfilt1pad(voxel(3,:),100);
        %      voxel(3,:)=smooth(voxel(3,:),100)';
    else
        voxel(3,:)=medfilt1(voxel(3,:),size(voxel,2));
        %     voxel(3,:)=smooth(voxel(3,:),size(voxel,2))';
    end
    
    voxelzs=[];
    for k=1:50
        obj.data.dendrites(i).length=[];
        % sample points in Z at 50 point intervals
        voxelz=[];
        startl=voxel(3,1);
        startpos=1;
        endpos=startpos+k;
        if (endpos<size(voxel,2)+1)
            for j=1:floor((size(voxel,2)-endpos-1)/50)+2
                if (j>1)
                    endpos=min(size(voxel,2),startpos+50);
                end
                lengtht=(voxel(3,endpos)-voxel(3,startpos));
                if (lengtht<0.0001) % if multiple points are essentially at the same place near the end of the axon backbone
                    voxelz=[voxelz startl*ones(1,endpos-startpos)];
                else
                    voxelz=[voxelz startl:lengtht/(endpos-startpos):(startl+lengtht*(endpos-startpos-1)/(endpos-startpos))];
                end
                %                size([startl:length/(endpos-startpos):(startl+length*(endpos-startpos)/(endpos-startpos+1))])
                oldstartpos=startpos;
                oldstartl=startl;
                startpos=endpos;
                startl=startl+lengtht;
            end
            voxelz=[voxelz startl];
            voxelzs=[voxelzs;voxelz];
        end
        
        
    end
    voxel(3,:)=mean(voxelzs,1);
    
    
    lengths=[];
    for k=1:20
        % smaller steps in X,Y
        onelength=[];
        startpos=1;
        startl=0;
        endpos=startpos+k;
        if (endpos<size(voxel,2)+1)
            for j=1:floor((size(voxel,2)-endpos-1)/20)+2
                if (j>1)
                    endpos=min(size(voxel,2),startpos+20);
                end
                
                
                lengtht=sqrt(obj.parameters.yspacing*obj.parameters.yspacing*(voxel(1,endpos)-voxel(1,startpos))*(voxel(1,endpos)-voxel(1,startpos))+ ...
                    obj.parameters.xspacing*obj.parameters.xspacing*(voxel(2,endpos)-voxel(2,startpos))*(voxel(2,endpos)-voxel(2,startpos))+ ...
                    obj.parameters.zspacing*obj.parameters.zspacing*(voxel(3,endpos)-voxel(3,startpos))*(voxel(3,endpos)-voxel(3,startpos)));
                if (lengtht<0.00001) % if multiple points are essentially at the same place near the end of the axon backbone
                    onelength=[onelength startl*ones(1,endpos-startpos)];
                else
                    onelength=[onelength startl:lengtht/(endpos-startpos):(startl+lengtht*(endpos-startpos-1)/(endpos-startpos))];
                end
                %                size([startl:length/(endpos-startpos):(startl+length*(endpos-startpos)/(endpos-startpos+1))])
                oldstartpos=startpos;
                oldstartl=startl;
                startpos=endpos;
                startl=startl+lengtht;
            end
            onelength=[onelength startl];
            lengths=[lengths; onelength];
        end
        
    end
    obj.data.dendrites(i).length=mean(lengths,1);
    
    %obj.data.dendrites(i).length=[0
    %cumsum(sqrt(obj.parameters.yspacing*obj.parameters.yspacing*diff(voxel(1,:)).*diff(voxel(1,:))+obj.parameters.xspacing*obj.parameters.xspacing*diff(voxel(2,:)).*diff(voxel(2,:))+obj.parameters.zspacing*obj.parameters.zspacing*diff(voxel(3,:)).*diff(voxel(3,:))))];
    
    %calculate length only in XY
    XYlengths=[];
    for k=1:20
        % smaller steps in X,Y
        onelength=[];
        startpos=1;
        startl=0;
        endpos=startpos+k;
        if (endpos<size(voxel,2)+1)
            for j=1:floor((size(voxel,2)-endpos-1)/20)+2
                if (j>1)
                    endpos=min(size(voxel,2),startpos+20);
                end
                
                
                lengtht=sqrt(obj.parameters.yspacing*obj.parameters.yspacing*(voxel(1,endpos)-voxel(1,startpos))*(voxel(1,endpos)-voxel(1,startpos))+ ...
                    obj.parameters.xspacing*obj.parameters.xspacing*(voxel(2,endpos)-voxel(2,startpos))*(voxel(2,endpos)-voxel(2,startpos)));
                if (lengtht<0.00001) % if multiple points are essentially at the same place near the end of the axon backbone
                    onelength=[onelength startl*ones(1,endpos-startpos)];
                else
                    onelength=[onelength startl:lengtht/(endpos-startpos):(startl+lengtht*(endpos-startpos-1)/(endpos-startpos))];
                end
                %                size([startl:length/(endpos-startpos):(startl+length*(endpos-startpos)/(endpos-startpos+1))])
                oldstartpos=startpos;
                oldstartl=startl;
                startpos=endpos;
                startl=startl+lengtht;
            end
            onelength=[onelength startl];
            lengths=[lengths; onelength];
        end
        
    end
    obj.data.dendrites(i).XYlength=mean(lengths,1);
    
    
    
    
    % estimate the median to get the approximate backbone value
    
    
    
    
    % reanalyze for meanback
    try
        for jj=1:4
            if jj<4 || exist('obj.data.dendrites(i).maxint3')
                
                switch jj
                    case 1
                        maxint0=obj.data.dendrites(i).maxint;
                    case 2
                        maxint0=obj.data.dendrites(i).maxint1;
                    case 3
                        maxint0=obj.data.dendrites(i).maxint2;
                    case 4
                        maxint0=obj.data.dendrites(i).maxint3;
                end
                m=double(median(maxint0));
%                 step=round(m/100);
%                 if step<1
%                     step=1;
%                 end
%                 
%                 [histogram]=histc(maxint0(maxint0<m*3),(0:step:m*3)');
%                 
%                 [t,maxind]=max(smooth(histogram,m*2/step,'rloess'));
%                 
%                 %interate to find self-consistent solution.
%                 for z=1:4
%                     [t,maxind]=max(smooth(histogram,min(length(histogram)-1,maxind*2/step),'rloess'));
%                 end
                
%                maxind=maxind*step;
                maxind=m;
                switch jj
                    case 1
                        obj.data.dendrites(i).meanback=maxind;
                    case 2
                        obj.data.dendrites(i).meanback1=maxind;
                    case 3
                        obj.data.dendrites(i).meanback2=maxind;
                    case 4
                        obj.data.dendrites(i).meanback3=maxind;
                end
            end
        end
        
    catch ME
        ME
    end
    
    
    
    medianmaxint=maxind;
    maxint=obj.data.dendrites(i).maxint;
    left=obj.data.dendrites(i).maxint(maxint<medianmaxint);
    cutthres=std([left-medianmaxint medianmaxint-left]);
    maxintcut=obj.data.dendrites(i).maxint;
    maxintcut(obj.data.dendrites(i).maxint>medianmaxint+cutthres*1)=deal(medianmaxint+cutthres*1);
    ns=obj.parameters.threedma.backbonemedian;
    if (size(maxintcut,2)>ns)
        medianfiltered=medfilt1([fliplr(maxintcut(1:ns)) maxintcut fliplr(maxintcut(end-ns+1:end))],ns);
        medianfiltered=medianfiltered(ns+1:end-ns);
    else
        medianfiltered=medfilt1([ones(1,ns)*maxintcut(1) maxintcut ones(1,ns)*maxintcut(end)],ns);
        medianfiltered=medianfiltered(ns+1:end-ns);
    end
    obj.data.dendrites(i).medianfiltered=medianfiltered;
end

if isfield(obj.data,'marks')
    for i=1:size(obj.data.marks,2)
        dendrite=double(obj.data.dendrites(obj.data.marks(i).den_ind).voxel);
        spine=double(obj.data.marks(i).voxel);
        dis=sqrt((dendrite(1,:)-spine(1,1)).*(dendrite(1,:)-spine(1,1))+(dendrite(2,:)-spine(2,1)).*(dendrite(2,:)-spine(2,1))+(dendrite(3,:)-spine(3,1)).*(dendrite(3,:)-spine(3,1)));
        [a,ind]=min(dis);
        obj.data.marks(i).dendis=ind;
        obj.data.marks(i).voxel(:,1)=uint16(dendrite(1:3,ind));
    end
end