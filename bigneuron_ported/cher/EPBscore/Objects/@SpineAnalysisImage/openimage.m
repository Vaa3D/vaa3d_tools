function obj=openimage(obj)
% get the name of the image
[fname, pname] = uigetfile({'*.tif'}, 'Choose image to load');
if (fname)
        obj.data=[];
        obj=openImageByNameGUI(obj,[pname fname]);
end