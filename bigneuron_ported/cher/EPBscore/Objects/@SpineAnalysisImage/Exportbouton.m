function obj = segmentAxonObj(obj) 
try
   %create segmentation array
   obj.data.segmentation=zeros(size(obj.data.binaryArray));
   for i=1:size(obj.data.dendrites,2)
       % the C program uses indices starting from 0. This needs to be converted
       % to MATLAB convention which starts from 1
       %[voxelgaussian, obj.data.dendrites(i).voxelmaxgaussian, obj.data.dendrites(i).maxintrawgaussian, dx, dy]=shiftDendriteMax(double(obj.data.dendrites(i).voxel),obj.data.gaussianArray);
       %[voxel, voxelmax, maxint, dx, dy]=shiftDendriteMax(double(obj.data.dendrites(i).voxel),obj.data.imageArray);
       %if 
       obj.data.segmentation=segmentAxon(double(obj.data.dendrites(i).voxel),obj.data.filteredArray,obj.data.segmentation);
   end 
catch
    disp(lasterr);
    disp(i);
end