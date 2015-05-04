function obj=opentwolaserimage(obj)
% get the name of the image
[fname, pname] = uigetfile({'*.tif'}, 'Choose image to load');
    [redname, p]= uigetfile({'*.tif'}, 'Choose Red Channel image to load');
    if (redname & fname)
        obj=openImageByNameGUI(obj,[pname fname],[p redname]);
    end