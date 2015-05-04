function displayProjectionImage(obj)
if isfield(obj.gh.projectionGUI,'Figure') && ishandle(obj.gh.projectionGUI.Figure)
    figure(obj.gh.projectionGUI.Figure);
    hold off;
    if isfield(obj.state.display,'colormap') && ~isempty(obj.state.display.colormap)
        colormap(obj.state.display.colormap);
    else
        colormap(gray);
    end
    cla;
    if (obj.state.display.displayraw==1)
        if (obj.state.display.ch1==1) && (obj.state.display.ch2==1) && isfield(obj.data.ch(1),'imageArray') && isfield(obj.data.ch(2),'imageArray') && ~isempty(obj.data.ch(1).imageArray) && ~isempty(obj.data.ch(2).imageArray)
            % dual color plots
            red=collapse(obj.data.ch(2).imageArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz),'XY');
            red=single(red);
            red=(red-obj.state.display.lowpixelch2)/(obj.state.display.highpixelch2-obj.state.display.lowpixelch2);
            red(red>1)=1;
            red(red<0)=0;
            green=collapse(obj.data.ch(1).imageArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz),'XY');
            green=single(green);
            green=(green-obj.state.display.lowpixelch1)/(obj.state.display.highpixelch1-obj.state.display.lowpixelch1);
            green(green>1)=1;
            green(green<0)=0;
            blue=zeros(size(red));
            combine=cat(3,red,green,blue);
            h=image(combine);
        else
            if (obj.state.display.ch1==1) && isfield(obj.data.ch(1),'imageArray') && ~isempty(obj.data.ch(1).imageArray)
                h=imagesc(collapse(obj.data.ch(1).imageArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz),'XY'),[obj.state.display.lowpixelch1 obj.state.display.highpixelch1]);
            end
            if (obj.state.display.ch2==1) && isfield(obj.data.ch(2),'imageArray')  && ~isempty(obj.data.ch(2).imageArray)
                h=imagesc(collapse(obj.data.ch(2).imageArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz),'XY'),[obj.state.display.lowpixelch2 obj.state.display.highpixelch2]);
            end
        end
        if exist('h','var')
            set(h,'ButtonDownFcn','spineanalysisimageGUI(''projectionimageaxes_ButtonDownFcn'',gcbo,[],guidata(gcbo))');
        end
    elseif (obj.state.display.displayfiltered==1)
        if (obj.state.display.ch1==1) && (obj.state.display.ch2==1) && isfield(obj.data.ch(1),'filteredArray') && isfield(obj.data.ch(2),'filteredArray') && ~isempty(obj.data.ch(1).filteredArray) && ~isempty(obj.data.ch(2).filteredArray)
            % dual color plots
            red=collapse(obj.data.ch(2).filteredArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz),'XY');
            red=single(red);
            red=(red-obj.state.display.lowpixelch2)/(obj.state.display.highpixelch2-obj.state.display.lowpixelch2);
            red(red>1)=1;
            red(red<0)=0;
            green=collapse(obj.data.ch(1).filteredArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz),'XY');
            green=single(green);
            green=(green-obj.state.display.lowpixelch1)/(obj.state.display.highpixelch1-obj.state.display.lowpixelch1);
            green(green>1)=1;
            green(green<0)=0;
            blue=zeros(size(red));
            combine=cat(3,red,green,blue);
            h=image(combine);
        else
            if (obj.state.display.ch1==1) && isfield(obj.data.ch(1),'filteredArray') && ~isempty(obj.data.ch(1).filteredArray)
%                h=imagesc(collapse(obj.data.ch(1).filteredArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz),'XY'),[obj.state.display.lowpixelch1 obj.state.display.highpixelch1]);
                num=obj.state.display.gaussian;
                h=imagesc(collapse(obj.data.ch(1).gaussianArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz,num),'XY'),[obj.state.display.lowpixelch1*16 obj.state.display.highpixelch1*16]);
 
            end
            if (obj.state.display.ch2==1) && isfield(obj.data.ch(2),'filteredArray') && ~isempty(obj.data.ch(2).filteredArray)
                h=imagesc(collapse(obj.data.ch(2).filteredArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz),'XY'),[obj.state.display.lowpixelch2 obj.state.display.highpixelch2]);
            end
        end
        if exist('h','var')
            set(h,'ButtonDownFcn','spineanalysisimageGUI(''projectionimageaxes_ButtonDownFcn'',gcbo,[],guidata(gcbo))');
        end
    elseif (obj.state.display.displaybinary==1)
        
        
         if (obj.state.display.ch2==1) && isfield(obj.data.ch(2),'binaryArray') && ~isempty(obj.data.ch(2).binaryArray)
                h=imagesc(collapse(obj.data.ch(2).binaryArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz),'XY'),[0 1]);
         elseif (obj.state.display.ch1==1) && isfield(obj.data.ch(1),'binaryArray') && ~isempty(obj.data.ch(1).binaryArray)
                h=imagesc(collapse(obj.data.ch(1).binaryArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz),'XY'),[0 1]);
         end
            
%         if (obj.state.display.ch1==1) && (obj.state.display.ch2==1) && isfield(obj.data.ch(1),'binaryArray') && isfield(obj.data.ch(2),'binaryArray') && length(obj.data.ch(1).binaryArray)>0 && length(obj.data.ch(2).binaryArray)>0
%             % dual color plots
%             red=collapse(obj.data.ch(2).binaryArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz),'XY');
%             red=single(red);
%             green=collapse(obj.data.ch(1).binaryArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz),'XY');
%             green=single(green);
%             blue=zeros(size(red));
%             combine=cat(3,red,green,blue);
%             h=image(combine);
%         else
%             if (obj.state.display.ch1==1) && isfield(obj.data.ch(1),'binaryArray') && length(obj.data.ch(1).binaryArray)>0
%                 h=imagesc(collapse(obj.data.ch(1).binaryArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz),'XY'),[0 1]);
%             end
%             if (obj.state.display.ch2==1) && isfield(obj.data.ch(2),'binaryArray') && length(obj.data.ch(2).binaryArray)>0
%                 h=imagesc(collapse(obj.data.ch(2).binaryArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz),'XY'),[0 1]);
%             end
%         end
        if exist('h','var')
            set(h,'ButtonDownFcn','spineanalysisimageGUI(''projectionimageaxes_ButtonDownFcn'',gcbo,[],guidata(gcbo))');
        end
    end;
    if (obj.state.display.displayMA)
        hold on;
        if (obj.state.display.ch1==1) && isfield(obj.data,'maset') && ~isempty(obj.data.maset)
            ch=1;
            h=plot(double(obj.data.maset(2,:)),double(obj.data.maset(1,:)),'r.');
            set(h,'LineWidth',5);
            set(h,'ButtonDownFcn','spineanalysisimageGUI(''projectionimageaxes_ButtonDownFcn'',gcbo,[],guidata(gcbo))');
            h=plot(double(obj.data.trimset(2,:)),double(obj.data.trimset(1,:)),'b.');
            set(h,'LineWidth',0.1);
            set(h,'ButtonDownFcn','spineanalysisimageGUI(''projectionimageaxes_ButtonDownFcn'',gcbo,[],guidata(gcbo))');
        end
        if (obj.state.display.ch2==1) && isfield(obj.data,'maset') && ~isempty(obj.data.maset)
            ch=2;
            h=plot(double(obj.data.maset(2,:)),double(obj.data.maset(1,:)),'r.');
            set(h,'LineWidth',5);
            set(h,'ButtonDownFcn','spineanalysisimageGUI(''projectionimageaxes_ButtonDownFcn'',gcbo,[],guidata(gcbo))');
            h=plot(double(obj.data.trimset(2,:)),double(obj.data.trimset(1,:)),'b.');
            set(h,'LineWidth',0.1);
            set(h,'ButtonDownFcn','spineanalysisimageGUI(''projectionimageaxes_ButtonDownFcn'',gcbo,[],guidata(gcbo))');
        end
    end
    if (obj.state.display.displaydendrites)
        hold on;
        if isfield(obj.data,'dendrites') && ~isempty(obj.data.dendrites)
            for i=1:size(obj.data.dendrites,2)
                h=plot(double(obj.data.dendrites(i).voxel(2,:)),double(obj.data.dendrites(i).voxel(1,:)),'.');
                set(h,'MarkerSize',1);
                set(h,'ButtonDownFcn','spineanalysisimageGUI(''projectionimageaxes_ButtonDownFcn'',gcbo,[],guidata(gcbo))');
                set(h,'color',hsv2rgb([i/size(obj.data.dendrites,2) 1.0 1.0]));
                h=text(double(obj.data.dendrites(i).voxel(2,floor(size(obj.data.dendrites(i).voxel,1)/2))),double(obj.data.dendrites(i).voxel(1,floor(size(obj.data.dendrites(i).voxel,1)/2))),char(64+i));
                set(h,'Color',hsv2rgb([i/size(obj.data.dendrites,2) 1.0 1.0]));
            end
        end
    end
    if isfield(obj.data,'ROI') && ~isempty(obj.data.ROI)
        hold on;
        for i=1:size(obj.data.dendrites,2)
            line(obj.data.ROI.xv,obj.data.ROI.yv);
        end
    end
    try
        ch=1;
    if (obj.state.display.displayspines)
        if isfield(obj.data,'spines') && ~isempty(obj.data.spines)
            hold on;
            for i=1:size(obj.data.spines,2)
                if ((obj.data.spines(i).edittype~=3) && ((obj.data.spines(i).edittype~=1) || (obj.state.display.displayrejectedspine)))
                    %colorvec=[1 0 0];
                    obj.state.display.displayentirespine=0;
                    if obj.state.display.displayentirespine==1
                        h=plot(double(obj.data.spines(i).voxel(2,3:end)),double(obj.data.ch(ch).spines(i).voxel(1,3:end)),'.');
                        if obj.data.spines(i).edittype==1
                            colorvec=[0.5 0.5 0.5];
                        else
                            colorvec=obj.data.spines(i).color;
                        end
                    else
                        if sum(obj.data.spines(i).voxel(1:2,1)-obj.data.spines(i).voxel(1:2,2)==[0 0]')==2
                            h=plot(obj.data.spines(i).voxel(2,1),obj.data.spines(i).voxel(1,1),'.');
                            set(h,'MarkerSize',6);
                        else
                            h=plot(obj.data.spines(i).voxel(2,1:2),obj.data.spines(i).voxel(1,1:2));
                            set(h,'LineWidth',3);
                        end
                        if obj.data.spines(i).edittype==1
                            colorvec=[0 1 0];
                        elseif obj.data.spines(i).edittype==2
                            colorvec=[0 0 1];
                        else
                            colorvec=[1 0 0];
                        end
                    end
                    set(h,'Color',colorvec);
                    set(h,'ButtonDownFcn','spineanalysisimageGUI(''projectionimageaxes_ButtonDownFcn'',gcbo,[],guidata(gcbo))');
                    if obj.data.spines(i).len<0.01    % right on the axon?
                        ind1=min(obj.data.spines(i).dendis+10,size(obj.data.dendrites(obj.data.spines(i).den_ind).voxel,2));
                        ind2=max(obj.data.spines(i).dendis-10,1);
                        dx=mean(obj.data.dendrites(obj.data.spines(i).den_ind).dx(ind2:ind1));
                        dy=mean(obj.data.dendrites(obj.data.spines(i).den_ind).dy(ind2:ind1));
                        %h=text(double(obj.data.spines(i).voxel(2,1))-dy*15-5,double(obj.data.spines(i).voxel(1,1))+dx*20-8,int2str(obj.data.spines(i).label));
                        h=text(double(obj.data.spines(i).voxel(2,1)),double(obj.data.spines(i).voxel(1,1))-10,int2str(obj.data.spines(i).label));
                        set(h,'Color',colorvec);
                    else
                        direction=[double(obj.data.spines(i).voxel(2,2))-double(obj.data.spines(i).voxel(2,1)) double(obj.data.spines(i).voxel(1,2))-double(obj.data.spines(i).voxel(1,1))];
                        direction=direction/sqrt(sum(direction.*direction));
                        co=[double(obj.data.spines(i).voxel(2,2)) double(obj.data.spines(i).voxel(1,2))]+direction*10-[3 3];

                        h=text(co(1),co(2),int2str(obj.data.spines(i).label));
                        %   h=text(double(obj.data.spines(i).voxel(2,2))*2.5-double(obj.data.spines(i).voxel(2,1))*1.5-3,double(obj.data.spines(i).voxel(1,2))*2.5-double(obj.data.spines(i).voxel(1,1))*1.5-3,int2str(obj.data.spines(i).label));
                        set(h,'Color',[0 1 0]);
                    end

                    set(h,'ButtonDownFcn','spineanalysisimageGUI(''projectionimageaxes_ButtonDownFcn'',gcbo,[],guidata(gcbo))');
                end
            end
        end
    end
    catch ME
        display 'error displaying spines'
        disp(lasterr)
    end
    if (obj.state.display.displaymarks) && isfield(obj.data,'marks')
        hold on;
        for i=1:size(obj.data.marks,2)
            colorvec=[0 1 0];
            h=plot(obj.data.marks(i).voxel(2,1),obj.data.marks(i).voxel(1,1),'+');
            set(h,'MarkerSize',6);
            set(h,'Color',colorvec);
            set(h,'ButtonDownFcn','spineanalysisimageGUI(''projectionimageaxes_ButtonDownFcn'',gcbo,[],guidata(gcbo))');
            ind1=min(obj.data.marks(i).dendis+10,size(obj.data.dendrites(obj.data.marks(i).den_ind).voxel,2));
            ind2=max(obj.data.marks(i).dendis-10,1);
            dx=mean(obj.data.dendrites(obj.data.marks(i).den_ind).dx(ind2:ind1));
            dy=mean(obj.data.dendrites(obj.data.marks(i).den_ind).dy(ind2:ind1));
            %h=text(double(obj.data.marks(i).voxel(2,1))-dy*15-5,double(obj.data.marks(i).voxel(1,1))+dx*20-8,int2str(obj.data.marks(i).label));
            h=text(double(obj.data.marks(i).voxel(2,1)),double(obj.data.marks(i).voxel(1,1))-10,int2str(obj.data.marks(i).label));
            set(h,'Color',colorvec);
            set(h,'ButtonDownFcn','spineanalysisimageGUI(''projectionimageaxes_ButtonDownFcn'',gcbo,[],guidata(gcbo))');
        end
    end
    %     if (obj.state.display.displaymarks)
    %         hold on;
    %         for i=1:size(obj.marks,1)
    %             h=plot(obj.marks(i,1)+1,obj.marks(i,2)+1,'r+');
    %             set(h,'ButtonDownFcn','SpineAnalysisImage(''projectionimageaxes_ButtonDownFcn'',gcbo,[],guidata(gcbo))');
    %             h=text(obj.marks(i,1)+10,obj.marks(i,2)+10,int2str(i));
    %             set(h,'color',[1 0 0]);
    %         end
    %     end
    figure(obj.gh.mainGUI.Figure);
end
