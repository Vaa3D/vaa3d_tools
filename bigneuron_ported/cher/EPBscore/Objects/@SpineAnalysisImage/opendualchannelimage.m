function obj=opendualchannelimage(obj)
% get the name of the image
[fname, pname] = uigetfile({'*.tif'}, 'Choose image to load');
    if (fname)
        obj=openImageByNameGUI(obj,[pname fname],'dualchannel');
    end