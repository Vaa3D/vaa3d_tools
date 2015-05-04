function obj = ShiftBranchMaxObj(obj,restricted) 
try
   for i=1:size(obj.data.dendrites,2)
       % the C program uses indices starting from 0. This needs to be converted
       % to MATLAB convention which starts from 1
       %[voxelgaussian, obj.data.dendrites(i).voxelmaxgaussian, obj.data.dendrites(i).maxintrawgaussian, dx, dy]=shiftDendriteMax(double(obj.data.dendrites(i).voxel),obj.data.gaussianArray);
       %[voxel, voxelmax, maxint, dx, dy]=shiftDendriteMax(double(obj.data.dendrites(i).voxel),obj.data.imageArray);
       %if 
       voxel=double(obj.data.dendrites(i).voxel);
       for j=1:3
           [voxel, voxelmax, maxintrawcorrected, dx, dy]=shiftBranchMax(voxel,obj.data.imageArray,restricted);
           voxel(:,1:3)=double(obj.data.dendrites(i).voxel(:,1:3));
           [maxint,voxelmax1,cube]=getbackboneintensity(voxel,obj.data.imageArray,dx,dy,0,1,0);
           maxint=reflectivesmooth(double(maxint),7);
       end
       endint=maxint(max(1,end-25):end)
       maxInd=find(imregionalmax(endint));
       maxIndold=maxInd;
       % is the local maximal >20%
       for j=1:size(maxInd,2)
           if (endint(maxInd(j))<15+max(min(endint(max(1,maxInd(j)-5):maxInd(j))),min(endint(maxInd(j):min(size(endint,2),maxInd(j)+5)))))
               maxInd(j)=0;
           end
       end    
       if (sum(maxInd)==0)
           m=maxIndold(end);
       else
        maxInd=maxInd(maxInd>0);      
        m=maxInd(end);
    end
 %      [ind,m]=max(maxint(max(1,end-25):end));
           voxel=floor(voxel(:,1:max(1,end-25)+m-1));
           dx=dx(:,1:max(1,end-25)+m-1);
           dy=dy(:,1:max(1,end-25)+m-1);
       [maxint,voxelmax1,cube]=getbackboneintensity(voxel,obj.data.imageArray,dx,dy,0,1,0);
     

       obj.data.dendrites(i).voxel=uint16(floor(voxel));
       voxelmax(:,1)=double(voxel(:,1));
       obj.data.dendrites(i).voxelmax=voxelmax;
       maxint=double(maxint);
       maxint=maxint-obj.data.imagemedian;
       obj.data.dendrites(i).maxintraw=maxint;
       obj.data.dendrites(i).maxint=reflectivesmooth(maxint,7);
       obj.data.dendrites(i).dx=dx;
       obj.data.dendrites(i).dy=dy;
       obj.data.dendrites(i).start=double(obj.data.dendrites(i).voxel(1:3,1))';
       obj.data.dendrites(i).end=double(obj.data.dendrites(i).voxel(1:3,end))';
       obj.data.dendrites(i).sizeofaxon=size(voxel,2);
       
       % median filtering
       % cut off tall peaks before filtering
       medianmaxint=median(maxint);
       left=maxint(maxint<medianmaxint);
       cutthres=std([left-medianmaxint medianmaxint-left]);
       maxintcut=maxint;
       maxintcut(maxint>medianmaxint+cutthres*1)=deal(medianmaxint+cutthres*1); 
       ns=obj.parameters.threedma.backbonemedian;
       if (size(maxintcut,2)>ns)
           medianfiltered=medfilt1([fliplr(maxintcut(1:ns)) maxintcut fliplr(maxintcut(end-ns+1:end))],ns);
           medianfiltered=medianfiltered(ns+1:end-ns);
       else
           medianfiltered=medfilt1([ones(1,ns)*maxintcut(1) maxintcut ones(1,ns)*maxintcut(end)],ns);
           medianfiltered=medianfiltered(ns+1:end-ns);
       end
       obj.data.dendrites(i).meanback=median(maxint);
       obj.data.dendrites(i).medianfiltered=medianfiltered;
   end 
catch
    disp(lasterr);
    disp(i);
end