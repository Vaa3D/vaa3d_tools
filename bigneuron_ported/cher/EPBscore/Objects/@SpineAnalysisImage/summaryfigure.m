function summaryfigure(obj)
figure;
hold off;
try
    count=0;
    for axonnumber=1:size(obj.data.dendrites,2)
        try
            dendrite=obj.data.dendrites(axonnumber);
            if (count<=4) & (max(size(obj.data.dendrites(axonnumber).maxint))>50)        
                maxint=double(obj.data.dendrites(axonnumber).maxint);
                meanback=double(obj.data.dendrites(axonnumber).meanback);
                maxint=maxint/meanback;
                maxintraw=double(obj.data.dendrites(axonnumber).maxintraw)/meanback;
     %                 posx=(axonnumber*2-2);
%                 posy=floor(posx/4);
%                 posx=posx-posy*4;
                  posx=axonnumber-1;
%                 subplot('position',[0.05+0.25*posx 0.33-0.25*posy 0.15 0.17]);
%                 hold off;
%                 [histogram,bins]=hist(maxint,200);
%                 plot(bins,histogram);
%                 axis([min(bins)-(max(bins)-min(bins))*0.05 max(bins)+(max(bins)-min(bins))*0.05 0 max(histogram+10)]);
%                 hold on;
%                 stem(1, max(histogram),'r');
%                 stem(median(maxint),max(histogram),'b');
%                 if size(obj.data.spines,1)>0
%                     spines=sum([obj.data.spines.den_ind]==axonnumber);
%                 end                
%                xlabel(['M' num2str(meanback)]);
                
                %subplot('position',[0.3+0.25*posx 0.33-0.25*posy 0.15 0.17]); 
                subplot('position',[0.05+0.5*posx 0.1 0.4 0.4]); 
               
                hold off;
                hold on;
%                plot(dendrite.length,maxintraw,'k');
                h=plot(dendrite.length,maxint,'r');
               % set(h,'Color',hsv2rgb([axonnumber/size(obj.data.dendrites,2) 1.0 1.0]),'Tag',[obj.state.rawImageName '_axonnumber#' num2str(axonnumber)]);
                set(h,'Tag',[obj.state.rawImageName '_axonnumber#' num2str(axonnumber)]); 
                hold on;  
                maxmaxint=max(maxint);
                minmaxint=min(maxint);
                if  isfield(obj.data.dendrites(axonnumber),'maxintdual') 
                    maxintdual=double(obj.data.dendrites(axonnumber).maxintdual)/meanback;      
                      
                    maxmaxint=max(max(maxint),max(maxintdual));
                     minmaxint=min(min(maxint),min(maxintdual));
                     h=plot(obj.data.dendrites(axonnumber).length,maxintdual,'g');
                end
                %plot(obj.data.dendrites(axonnumber).length,double(obj.data.dendrites(axonnumber).thres)*ones(size(maxint)),'r');
                %hold on;
 %               plot(dendrite.length,dendrite.medianfiltered/meanback,'k');
                
                axis([0 max(obj.data.dendrites(axonnumber).length) minmaxint-(maxmaxint-minmaxint)*0.05 maxmaxint+(maxmaxint-minmaxint)*0.25]);
 
                if size(obj.data.spines,1)>0
                %axis([0 max(dendrite.length) min(maxint)-(max(maxint)-min(maxint))*0.05 max(maxint)+(max(maxint)-min(maxint))*0.25+0.05]);
                ind=find([obj.data.spines.den_ind]==axonnumber);
                peaks=[obj.data.spines(ind).dendis];
                peakdistance=[obj.data.dendrites(axonnumber).length(peaks)];
                for i=1:size(peaks,2)
                    if (obj.data.spines(ind(i)).edittype==1)
                        colorvec=[0 1 0];
                    elseif (obj.data.spines(ind(i)).edittype==2)
                        colorvec=[0 0 1];
                    else    
                        colorvec=[1 0 0];
                    end      
                    if (obj.data.spines(ind(i)).len==0)
                        h=text(peakdistance(i),0.1*(max(maxint)-min(maxint))+maxint(peaks(i)),num2str(obj.data.spines(ind(i)).label)); 
                        set(h,'Color',colorvec);
                    else
                        h=plot(peakdistance(i),obj.data.spines(ind(i)).intensity/meanback,'r+');
                        set(h,'Color',colorvec);
                        h=text(peakdistance(i),0.1*(max(maxint)-min(maxint))+obj.data.spines(ind(i)).intensity/obj.data.dendrites(axonnumber).meanback,num2str(obj.data.spines(ind(i)).label));       
                        set(h,'Color',colorvec);
                    end
                    
                end
                %plot(peakdistance,([obj.data.spines(ind).intensity]-dendrite.shiftraw)/(dendrite.meanback*dendrite.stretchraw),'r.');
                  
                %h=plot(peakdistance,maxint(peaks),'.');
                xlabel(['M-' num2str(meanback) ' I-' num2str(obj.data.dendrites(axonnumber).length(end)/sum(([obj.data.spines.den_ind]==axonnumber) & ([obj.data.spines.len]==0))) ...
                        ' T-' num2str(obj.data.dendrites(axonnumber).length(end)/sum(([obj.data.spines.den_ind]==axonnumber) & ([obj.data.spines.len]>0)))]);
            end
                 count=count+1;
            end
        catch
            disp(lasterr);
        end
    end
    
%     if isfield(obj.state.display,'colormap')
%         colormap(flipud(obj.state.display.colormap));
%     end
    subplot('position',[0.05 0.6 0.4 0.4]);

    if (obj.state.display.ch1==1) && (obj.state.display.ch2==1) && isfield(obj.data.ch(1),'imageArray') && isfield(obj.data.ch(2),'imageArray') && length(obj.data.ch(1).imageArray)>0 && length(obj.data.ch(2).imageArray)>0
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
        if (obj.state.display.ch1==1) && isfield(obj.data.ch(1),'imageArray') && length(obj.data.ch(1).imageArray)>0
            h=imagesc(collapse(obj.data.ch(1).imageArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz),'XY'),[obj.state.display.lowpixelch1 obj.state.display.highpixelch1]);
        end
        if (obj.state.display.ch2==1) && isfield(obj.data.ch(2),'imageArray')  && length(obj.data.ch(2).imageArray)>0
            h=imagesc(collapse(obj.data.ch(2).imageArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz),'XY'),[obj.state.display.lowpixelch2 obj.state.display.highpixelch2]);
        end
    end
    xlabel(obj.state.rawImageName);

    subplot('position',[0.55 0.6 0.4 0.4]);
    if (obj.state.display.ch1==1) && (obj.state.display.ch2==1) && isfield(obj.data.ch(1),'filteredArray') && isfield(obj.data.ch(2),'filteredArray') && length(obj.data.ch(1).filteredArray)>0 && length(obj.data.ch(2).filteredArray)>0
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
         if isfield(obj.data.ch(1),'ROI') & length(obj.data.ch(1).ROI)
                xlabel(['meanimageback ' num2str( obj.data.ch(1).imagemedian) ' ' num2str( obj.data.ch(2).imagemedian) ' ROI ' num2str(obj.data.ch(1).ROI.meanintensity) ' ' num2str(obj.data.ch(2).ROI.meanintensity)]);
            else
                xlabel(['meanimageback ' num2str( obj.data.ch(1).imagemedian) ' ' num2str(obj.data.ch(2).imagemedian)]);
            end    
    else
        if (obj.state.display.ch1==1) && isfield(obj.data.ch(1),'filteredArray') && length(obj.data.ch(1).filteredArray)>0
            h=imagesc(collapse(obj.data.ch(1).filteredArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz),'XY'),[obj.state.display.lowpixelch1 obj.state.display.highpixelch1]);
            if isfield(obj.data.ch(1),'ROI') & length(obj.data.ch(1).ROI)
                xlabel(['meanimageback ' num2str( obj.data.ch(1).imagemedian) ' ROI ' num2str(obj.data.ch(1).ROI.meanintensity)]);
            else
                xlabel(['meanimageback ' num2str( obj.data.ch(1).imagemedian)]);
            end           
        end
        if (obj.state.display.ch2==1) && isfield(obj.data.ch(2),'filteredArray') && length(obj.data.ch(2).filteredArray)>0
            h=imagesc(collapse(obj.data.ch(2).filteredArray(:,:,obj.state.ROI.minz:obj.state.ROI.maxz),'XY'),[obj.state.display.lowpixelch2 obj.state.display.highpixelch2]);
            if isfield(obj.data.ch(2),'ROI') & length(obj.data.ch(2).ROI)
                xlabel(['meanimageback ' num2str( obj.data.ch(2).imagemedian) ' ROI ' num2str(obj.data.ch(2).ROI.meanintensity)]);
            else
                xlabel(['meanimageback ' num2str( obj.data.ch(2).imagemedian)]);
            end
        end
    end
    
    hold on;
    for i=1:size(obj.data.dendrites,2)
        h=plot(double(obj.data.dendrites(i).voxel(2,:)),double(obj.data.dendrites(i).voxel(1,:)));
        set(h,'LineWidth',2);
        set(h,'ButtonDownFcn','spineanalysisimageGUI(''projectionimageaxes_ButtonDownFcn'',gcbo,[],guidata(gcbo))');
        set(h,'color',hsv2rgb([i/size(obj.data.dendrites,2) 0.8 0.8]),'Tag',[obj.state.rawImageName '_axonnumber#' num2str(i)]);
        h=text(double(obj.data.dendrites(i).voxel(2,floor(size(obj.data.dendrites(i).voxel,1)/2))),double(obj.data.dendrites(i).voxel(1,floor(size(obj.data.dendrites(i).voxel,1)/2))),char(64+i));
        set(h,'Color',[0 1 0]);
    end
    for i=1:size(obj.data.spines,2)
        if ((obj.data.spines(i).edittype~=3) & ((obj.data.spines(i).edittype~=1) | (obj.state.display.displayrejectedspine)))
            colorvec=[1 0 0];
            if obj.state.display.displayentirespine==1 
                h=plot(double(obj.data.spines(i).voxel(2,3:end)),double(obj.data.spines(i).voxel(1,3:end)),'.');
                if obj.data.spines(i).edittype==1
                    colorvec=[0.5 0.5 0.5];
                else
                    colorvec=[0.5 0.5 0.5];
                end
            else
                if sum(obj.data.spines(i).voxel(1:2,1)-obj.data.spines(i).voxel(1:2,2)==[0 0]')==2
                    h=plot(obj.data.spines(i).voxel(2,1),obj.data.spines(i).voxel(1,1),'.');
                    set(h,'MarkerSize',6);
                    set(h,'Color',[1 0 0],'Tag',[obj.state.rawImageName '_spinenumber#' num2str(obj.data.spines(i).label) '_axonnumber#' num2str(obj.data.spines(i).den_ind)]);
                else                    
                    h=plot(obj.data.spines(i).voxel(2,1:2),obj.data.spines(i).voxel(1,1:2));
                    set(h,'LineWidth',3);
                    set(h,'Color',[0.5 0.5 0.5]);
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
            if obj.data.spines(i).len<0.5    % right on the axon?
                ind1=min(obj.data.spines(i).dendis+10,size(obj.data.dendrites(obj.data.spines(i).den_ind).voxel,2));
                ind2=max(obj.data.spines(i).dendis-10,1);
                dx=mean(obj.data.dendrites(obj.data.spines(i).den_ind).dx(ind2:ind1));
                dy=mean(obj.data.dendrites(obj.data.spines(i).den_ind).dy(ind2:ind1));
                h=text(double(obj.data.spines(i).voxel(2,1))-dy*15-5,double(obj.data.spines(i).voxel(1,1))+dx*20-8,int2str(obj.data.spines(i).label));
            else
                h=text(double(obj.data.spines(i).voxel(2,2))*2.5-double(obj.data.spines(i).voxel(2,1))*1.5-3,double(obj.data.spines(i).voxel(1,2))*2.5-double(obj.data.spines(i).voxel(1,1))*1.5-3,int2str(obj.data.spines(i).label));
            end
            set(h,'Color',[1 0 0]);
        end
    end
catch
    disp(lasterr);
end
