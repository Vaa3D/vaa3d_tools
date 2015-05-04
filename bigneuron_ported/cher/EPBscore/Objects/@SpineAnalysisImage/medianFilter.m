function obj = MedianFilter(obj)

if (obj.state.display.ch1==1) & isfield(obj.data.ch(1),'imageArray')
    ch=1;
    if (obj.state.GUI==1)
        w = waitbar(0,'Filtering of Tif', 'Name', 'GaussianFiltering XY');
    end
    
    obj.data.ch(ch).imageArray=uint16(obj.data.ch(ch).imageArray);
    obj.data.ch(ch).gaussianArray=zeros([size(obj.data.ch(ch).imageArray) 3],'uint16');
    for i=1:size(obj.data.ch(ch).imageArray,3)
        obj.data.ch(ch).gaussianArray(:,:,i,4)=imgaussian(obj.data.ch(ch).imageArray(:,:,i)*16,[15 15],[51 51],0);
        obj.data.ch(ch).gaussianArray(:,:,i,3)=imgaussian(obj.data.ch(ch).imageArray(:,:,i)*16,[7 7],[31 31],0);
        obj.data.ch(ch).gaussianArray(:,:,i,2)=imgaussian(obj.data.ch(ch).imageArray(:,:,i)*16,[3 3],[21 21],0);
        obj.data.ch(ch).gaussianArray(:,:,i,1)=imgaussian(obj.data.ch(ch).imageArray(:,:,i)*16,[1 1],[7 7],0);

        if (obj.state.GUI==1)
            waitbar(i/size(obj.data.ch(ch).imageArray,3),w,['Filtering frame ' num2str(i) ' of ' num2str(size(obj.data.ch(ch).imageArray,3))]);
        end
    end
    obj.data.ch(ch).gaussianmedian=median(obj.data.ch(ch).gaussianArray(:));
    if (obj.state.GUI==1)
        close(w);
    end
    
    if (obj.state.GUI==1)
        w = waitbar(0,'Filtering of Tif', 'Name', 'MedianFiltering XY');
    end
    
    obj.data.ch(ch).filteredArray=zeros(size(obj.data.ch(ch).imageArray),'uint16');
    for i=1:size(obj.data.ch(ch).imageArray,3)
        obj.data.ch(ch).filteredArray(:,:,i)=medfilt2(obj.data.ch(ch).imageArray(:,:,i),[obj.parameters.threedma.medianneighborhood obj.parameters.threedma.medianneighborhood]);
        if (obj.state.GUI==1)
            waitbar(i/size(obj.data.ch(ch).imageArray,3),w,['Filtering frame ' num2str(i) ' of ' num2str(size(obj.data.ch(ch).imageArray,3))]);
        end
    end
    obj.data.ch(ch).imagemedian=mode(single(obj.data.ch(ch).filteredArray(:)));
    
    if (obj.state.GUI==1)
        close(w);
    end
    
    obj.data.ch(ch).imagestd=5;
    obj.state.display.lowpixelch1=40;
end

% Old obsolete code


% box filter

% gaussian filter
%     n=50;
%     s=30;
%     x=-1/2:1/(n-1):1/2;
%     [Y,X]=meshgrid(x,x);
%     f=single(exp(-(X.^2+Y.^2)/(2*s^2)));
%     f=f/sum(f(:));

%f=uint16(ones(50,50));
if (obj.state.display.ch2==1) && isfield(obj.data.ch(2),'imageArray')
    ch=2;
    if (obj.state.GUI==1)
        w = waitbar(0,'Filtering of Tif', 'Name', 'MedianFiltering XY');
    end

    obj.data.ch(ch).filteredArray=int16([]);
    obj.data.ch(ch).imagemedian=median(obj.data.ch(ch).imageArray(:));
     if 0 % obsolete method
        for z=1:1
            
            obj.data.ch(ch).filteredArray=int16([]);
            for i=1:size(obj.data.ch(ch).imageArray,3)
                
                IM=int16(obj.data.ch(ch).imageArray(:,:,i));
                IM=medfilt2(IM,[obj.parameters.threedma.medianneighborhood obj.parameters.threedma.medianneighborhood]);
                %         M=IM;
                %         pu=prctile(M(:),[85]);
                %         m=median(M(:));
                %         M(M>pu)=m;
                %         B=int16(fastrunmean(M,[51 51],'mean'));
                %         F=IM-B+30;
                %         % replace original to save memory space?
                %         if ~obj.state.display.keepintermediates
                %             obj.data.ch(ch).imageArray(:,:,i)=F;
                %         else
                %             obj.data.ch(ch).filteredArray(:,:,i)=F;
                %
                %         end
                %obj.data.ch(ch).filteredArray=fastrunmean(obj.data.ch(ch).filteredArray,[5 5 5],'mean');
                
                %obj.data.ch(ch).filteredArray=smooth3(single(obj.data.ch(ch).imageArray),'box',[5 5 3]);
                
                if (obj.state.GUI==1)
                    waitbar(i/size(obj.data.ch(ch).imageArray,3),w,['Filtering frame ' num2str(i) ' of ' num2str(size(obj.data.ch(ch).imageArray,3))]);
                end
                
            end
            if ~obj.state.display.keepintermediates
                obj.data.ch(ch).filteredArray=obj.data.ch(ch).imageArray;
                obj.data.ch(ch).imageArray=[];
            end
        end
        
    end
    for i=1:size(obj.data.ch(ch).imageArray,3)
        %            M=single(obj.data.ch(ch).imageArray(:,:,i));
        IM=int16(obj.data.ch(ch).imageArray(:,:,i));
        IM=medfilt2(IM,[obj.parameters.threedma.medianneighborhood obj.parameters.threedma.medianneighborhood]);
        M=IM;
        pu=prctile(M(:),[85]);
        m=median(M(:));
        M(M>pu)=m;
        B=int16(fastrunmean(M,[51 51],'mean'));
         F=IM-B+30;
        % replace original to save memory space?
        if ~obj.state.display.keepintermediates
            obj.data.ch(ch).imageArray(:,:,i)=F;
        else
            obj.data.ch(ch).filteredArray(:,:,i)=F;
        end
        if (obj.state.GUI==1)
            waitbar(i/size(obj.data.ch(ch).imageArray,3),w,['Filtering frame ' num2str(i) ' of ' num2str(size(obj.data.ch(ch).imageArray,3))]);
        end
        
        % do box filtering in the other dimension
       

    end
    obj.data.ch(ch).filteredArray=fastrunmean(obj.data.ch(ch).filteredArray,[5 5 5],'mean');
    obj.data.ch(ch).gaussianArray=smooth3(obj.data.ch(ch).imageArray,'gaussian',[7 7 3],2);

    obj.data.ch(ch).imagemedian=mode(obj.data.ch(ch).gaussianArray(:));
   
    if ~obj.state.display.keepintermediates
        obj.data.ch(ch).filteredArray=obj.data.ch(ch).imageArray;
        obj.data.ch(ch).imageArray=[];
    end
    if (obj.state.GUI==1)
        close(w);
    end
    obj.data.ch(ch).imagestd=5;
    obj.state.display.lowpixelch2=40;
    updateGUIbyglobal('self.state.display.lowpixelch2');
end
