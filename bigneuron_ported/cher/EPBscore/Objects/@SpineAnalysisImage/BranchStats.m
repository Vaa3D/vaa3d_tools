function obj = ShiftBranchMaxObj(obj,restricted) 
 % the C program uses indices starting from 0. This needs to be converted
       % to MATLAB convention which starts from 1
       %[voxelgaussian, obj.data.dendrites(i).voxelmaxgaussian, obj.data.dendrites(i).maxintrawgaussian, dx, dy]=shiftDendriteMax(double(obj.data.dendrites(i).voxel),obj.data.gaussianArray);
       %[voxel, voxelmax, maxint, dx, dy]=shiftDendriteMax(double(obj.data.dendrites(i).voxel),obj.data.imageArray);
       %if 
       stats=[];
       for j=1:4
           obj=ShiftBranchMaxObj(obj,restricted);
           obj=CorrectDendrites(obj);
           for i=1:size(obj.data.dendrites,2)
               vec=double(obj.data.dendrites(i).voxel(1:3,1))-double(obj.data.dendrites(i).voxel(1:3,end));
               vec=vec.*[0.083 0.083 1]';
               maxint=double(obj.data.dendrites(i).maxint);
               meanback=double(obj.data.dendrites(i).meanback);
               maxint=maxint/meanback;
               
               stats(j,1,i)=max(obj.data.dendrites(i).length);
               stats(j,2,i)=sqrt(sum(vec.*vec));
               stats(j,3,i)=max(maxint(end-5:end));
           end
       end
       for i=1:size(obj.data.dendrites,2)
           stats(j+1,:,i)=mean(stats(1:j,:,i));
       end
       stats