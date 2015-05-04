function obj=exportmaxprojection(obj)
global self;
global ogh;
self=struct(obj);
[filename, pathname] = uiputfile('*.tif','Save Max Projection As',[ obj.state.rawImageName 'max.tif']);

% Are we processing dual channels?
if (obj.state.display.ch1==1) && (obj.state.display.ch2==1)
    array(:,:,1)=(collapse(obj.data.ch(1).imageArray,'XY')-obj.state.display.lowpixelch1)*(16*4096/(obj.state.display.highpixelch1-obj.state.display.lowpixelch1));
    array(:,:,2)=(collapse(obj.data.ch(2).imageArray,'XY')-obj.state.display.lowpixelch1)*(16*4096/(obj.state.display.highpixelch1-obj.state.display.lowpixelch1));
    array(:,:,3)=zeros([size(obj.data.ch(1).imageArray,1) size(obj.data.ch(1).imageArray,2)]);
else
    if (obj.state.display.ch1==1)
         array=collapse(obj.data.ch(1).imageArray,'XY');
    else if (obj.state.display.ch2==1)
         array=collapse(obj.data.ch(2).imageArray,'XY');
        end
    end
end
imwrite(array,[pathname filename]);
