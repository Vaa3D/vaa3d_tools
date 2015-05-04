function obj=exportstack(obj)
global self;
global ogh;
self=struct(obj);
[filename, pathname] = uiputfile('*.tif','Save Max Projection As',[ obj.state.rawImageName '_.tif']);
[name ext]=strtok(filename,'.');

for i=1:size(obj.data.ch(1).imageArray,3)
    % Are we processing dual channels?
    if (obj.state.display.ch1==1) && (obj.state.display.ch2==1)
        array(:,:,1)=(obj.data.ch(1).imageArray(:,:,i)-obj.state.display.lowpixelch1)*(16*4096/(obj.state.display.highpixelch1-obj.state.display.lowpixelch1));
        array(:,:,2)=(obj.data.ch(2).imageArray(:,:,i)-obj.state.display.lowpixelch1)*(16*4096/(obj.state.display.highpixelch1-obj.state.display.lowpixelch1));
        array(:,:,3)=zeros([size(obj.data.ch(1).imageArray,1) size(obj.data.ch(1).imageArray,2)]);
    else
        if (obj.state.display.ch1==1)
            array=obj.data.ch(1).imageArray(:,:,i);
        else if (obj.state.display.ch2==1)
                array=obj.data.ch(1).imageArray(:,:,i);
            end
        end
    end
    imwrite(array,[pathname name num2str(i) ext ]);
end
