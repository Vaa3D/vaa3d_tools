function obj=openimagebyname(obj,filepath,redfilepath)
[path,name,ext] = fileparts([filepath]);
cd(path);
imageArray=opentif(filepath);  % get the image
obj.state.rawImagePath=path;
obj.state.rawImageName=name;
if exist('redfilepath','var')
    if strcmp(redfilepath,'dualchannel')
        % dual channels
        obj.data.ch(1).imageArray=imageArray(:,:,(1:size(imageArray,3)/2)*2-1);
        obj.data.ch(2).imageArray=imageArray(:,:,(1:size(imageArray,3)/2)*2);
        clear imageArray;
    else
        % contrast adjust images acquired with two lasers
        % needs some more work

        obj.data.ch(1).imageArray=imageArray(:,:,(1:size(imageArray,3)/2)*2-1);
        obj.data.ch(2).imageArray=imageArray(:,:,(1:size(imageArray,3)/2)*2);

        imageArray=opentif(redfilepath);  % get the image acquired with the other laser
        obj.data.redImageArray=imageArray(:,:,(1:size(imageArray,3)/2)*2);
        obj.data.redDualArray=imageArray(:,:,(1:size(imageArray,3)/2)*2-1);
        obj.state.redImagePath=path;
        obj.state.redImageName=name;
        filtered=[];
        for i=1:size(obj.data.ch(1).imageArray,3)
            filtered(:,:,i)=medfilt2(obj.data.ch(1).imageArray(:,:,i),[7 7]);
        end
        filteredred=[];
        for i=1:size(obj.data.ch(1).imageArray,3)
            filteredred(:,:,i)=medfilt2(obj.data.redImageArray(:,:,i),[7 7]);
        end
        %
        % low=prctile(obj.data.ch(2).imageArray(filtered>50),20);
        % high=prctile(obj.data.ch(2).imageArray(filtered>50),100);
        % percentiles=[];
        % for i=low:(high-low)/9:high
        % percentiles=[percentiles sum(obj.data.ch(2).imageArray(:)>i)/length(obj.data.ch(2).imageArray(:)>i)];
        % end
        % percentiles=100*(1-percentiles);
        % get the linear alignment between various images based on brighest pixels
        intensities=[];
        percentiles=[99 99.7 99.9 99.95 99.99 99.999 100];
        % for i=percentiles
        %     intensities=[intensities;prctile(obj.data.redImageArray(:),i) prctile(obj.data.ch(1).imageArray(:),i) prctile(obj.data.ch(2).imageArray(:),i)];
        % end

        for i=percentiles
            intensities=[intensities;prctile(filtered(:),i) prctile(filteredred(:),i)];
        end

        ftobj=fittype('c*x');
        fittedmodel=fit(intensities(:,2),intensities(:,1),ftobj);
        obj.data.ch(1).imageArray=obj.data.redImageArray*fittedmodel.c/1.5;

        maxz=min( size(obj.data.ch(1).imageArray,3), size(obj.data.ch(2).imageArray,3));
        obj.data.ch(1).imageArray=obj.data.ch(1).imageArray(:,:,1:maxz);
        obj.data.ch(2).imageArray=obj.data.ch(2).imageArray(:,:,1:maxz);
    end
    % Set Image related properties
    obj.state.ROI.maxz=size(obj.data.ch(1).imageArray,3);
    obj.state.ROI.minz=1;
    obj.state.display.currentz=1;
    obj.state.display.lowpixel=double(min(min(min(obj.data.ch(1).imageArray))));
    obj.state.display.highpixel=double(max(max(max(obj.data.ch(1).imageArray))));

    obj.state.display.lowpixeldual=double(min(min(min(obj.data.ch(2).imageArray))));
    obj.state.display.highpixeldual=double(max(max(max(obj.data.ch(2).imageArray))));
else
    obj.data.ch(1).imageArray=imageArray;
    % Set Image related properties
    obj.state.ROI.maxz=size(obj.data.ch(1).imageArray,3);
    obj.state.ROI.minz=1;
    obj.state.display.currentz=1;
    obj.state.display.lowpixel=double(min(min(min(obj.data.ch(1).imageArray))));
    obj.state.display.highpixel=double(max(max(max(obj.data.ch(1).imageArray))));

end
