function displayProfile(obj)
global maxint;
global maxintraw;
if isfield(obj.gh.profileGUI,'Figure') && ishandle(obj.gh.profileGUI.Figure)
    figure(obj.gh.profileGUI.Figure);
    axonnumber=obj.state.display.axonnumber;
    if axonnumber>0 && size(obj.data.dendrites,2)>=axonnumber && isfield(obj.data.dendrites(axonnumber),'maxint') && ~isempty(obj.data.dendrites(axonnumber).maxint)
        maxint=double(obj.data.dendrites(axonnumber).maxint);
        meanback=double(obj.data.dendrites(axonnumber).meanback);
        maxint=maxint/meanback;
        maxintraw=double(obj.data.dendrites(axonnumber).maxintraw)/meanback;
%         if  isfield(obj.data.dendrites(axonnumber),'maxintdual') 
%             maxintdual=double(obj.data.dendrites(axonnumber).maxintdual)/meanback;      
%         end
        % hack by sen to maintain backward compatibility
        if ~isfield(obj.state.display,'gaussian')
            obj.state.display.gaussian=1;
        end
        num=obj.state.display.gaussian;
        if num==1
            maxint2=double(obj.data.dendrites(axonnumber).maxint)/obj.data.dendrites(axonnumber).meanback;
            meanback2=obj.data.dendrites(axonnumber).meanback;
        else if num==2
                if isfield(obj.data.dendrites(axonnumber),'maxint1')
                    maxint2=double(obj.data.dendrites(axonnumber).maxint1)/obj.data.dendrites(axonnumber).meanback1;
                    meanback2=obj.data.dendrites(axonnumber).meanback1;
                end
            else if num==3
                    maxint2=double(obj.data.dendrites(axonnumber).maxint2)/obj.data.dendrites(axonnumber).meanback2;
                    meanback2=obj.data.dendrites(axonnumber).meanback2;
                else if num==4
                          maxint2=double(obj.data.dendrites(axonnumber).maxint3)/obj.data.dendrites(axonnumber).meanback3;
                          meanback2=obj.data.dendrites(axonnumber).meanback3;
                    end
                end
            end
        end
        
        subplot('position',[0.0 0.35 0.48 0.65]);
        hold off;
        [histogram,bins]=hist(maxint2,1000);
        bar(bins,histogram);
        axis([min(bins)-(max(bins)-min(bins))*0.05 max(bins)+(max(bins)-min(bins))*0.05 0 max(histogram+10)]);
        hold on;
        stem(1, max(histogram),'r');
        %vector distance
        %obj.data.dendrites(axonnumber).voxel(1:3,1)
        %obj.data.dendrites(axonnumber).voxel(1:3,end)
        %vec=double(obj.data.dendrites(axonnumber).voxel(1:3,1))-double(obj.data.dendrites(axonnumber).voxel(1:3,end));
        %vec=vec.*[0.083 0.083 1]';
        xlabel(['length-' num2str(max(obj.data.dendrites(axonnumber).length)) ' maxstack-' num2str(max(obj.data.ch(1).imageArray(:)))]);
        
        subplot('position',[0.50 0.35 0.48 0.65]);
        cla;
        hold off;
        plot(obj.data.dendrites(axonnumber).length,maxintraw,'b');
        hold on;
        plot(obj.data.dendrites(axonnumber).length,maxint,'r');
        if isfield (obj.data.dendrites(axonnumber),'maxint2')
           plot(obj.data.dendrites(axonnumber).length,maxint2,'c');
        end
     
        maxmaxint=max(maxint);
        minmaxint=min(maxint);
%         if  isfield(obj.data.dendrites(axonnumber),'maxintdual') 
%             h=plot(obj.data.dendrites(axonnumber).length,maxintdual,'g');
%             maxmaxint=max(maxmaxint,max(maxintdual));
%             minmaxint=min(minmaxint,min(maxintdual));
%         end
        %        set(h,'Color',hsv2rgb([axonnumber/size(obj.data.dendrites,2) 1.0 1.0]));
        %        plot(obj.data.dendrites(axonnumber).length,(double(obj.data.dendrites(axonnumber).maxintraw)-obj.data.dendrites(axonnumber).shiftraw)/(meanback*obj.data.dendrites(axonnumber).stretchraw));
        %        plot(obj.data.dendrites(axonnumber).length,double(obj.data.dendrites(axonnumber).thres)*ones(size(maxint)),'r');
        %        hold on;
        plot(obj.data.dendrites(axonnumber).length,obj.data.dendrites(axonnumber).medianfiltered/meanback,'k');
        plot([0 obj.data.dendrites(axonnumber).length(end)],[1 1],'g');
       
        axis([0 max(obj.data.dendrites(axonnumber).length) minmaxint-(maxmaxint-minmaxint)*0.05 maxmaxint+(maxmaxint-minmaxint)*0.25]);
          if num==1
            maxint2=double(obj.data.dendrites(axonnumber).maxint)/obj.data.dendrites(axonnumber).meanback;
        else if num==2
                if isfield(obj.data.dendrites(axonnumber),'maxint1')
                    maxint2=double(obj.data.dendrites(axonnumber).maxint1)/obj.data.dendrites(axonnumber).meanback1;
                end
            else if num==3
                    maxint2=double(obj.data.dendrites(axonnumber).maxint2)/obj.data.dendrites(axonnumber).meanback2;
                else if num==4
                          maxint2=double(obj.data.dendrites(axonnumber).maxint3)/obj.data.dendrites(axonnumber).meanback3;
                    end
                end
            end
        end
        
        
        xlabel(['meanback-' num2str(meanback) '   median' num2str(median(double(obj.data.dendrites(axonnumber).maxint))) ' meanbgauss '  num2str(meanback2) ]);
        
%          if isfield(obj.data.dendrites(axonnumber),'XYlength') 
%              content=[max(obj.data.dendrites(axonnumber).length) sqrt(sum(vec.*vec)) max(obj.data.dendrites(axonnumber).XYlength) sqrt(sum(vec(1:2).*vec(1:2))) max(maxint(end-5:end)) max(maxint(end-5:end))*meanback];
%              dec=2;
%              ff=ceil(log10(max(abs(content))))+dec+3;
%             % clipboard('copy',sprintf(['%#',num2str(ff),'.',num2str(dec),'f\t'],content)); 
%          end
         if (max(size(obj.data.spines))>0)
            ind=find([obj.data.spines.den_ind]==axonnumber);
            if (max(size(ind))>0)
                peaks=[obj.data.spines(ind).dendis];
                goodpeaks=[];
                goodpeaksraw=[];
                goodpeaksdual=[];
                goodpeaksdualraw=[];
               
                peakdistance=obj.data.dendrites(axonnumber).length(peaks);
                for i=1:size(peaks,2)
                    if (obj.data.spines(ind(i)).edittype==1)
                        colorvec=[0 1 0];
                    elseif (obj.data.spines(ind(i)).edittype==2)
                        colorvec=[0 0 1];
                    else    
                        colorvec=[1 0 0];
                        if (obj.data.spines(ind(i)).len==0)
                            goodpeaks=[goodpeaks double(maxint(peaks(i)))];
                            goodpeaksraw=[goodpeaksraw double(obj.data.dendrites(axonnumber).maxint(peaks(i)))];
                            if  isfield(obj.data.dendrites(axonnumber),'maxintdual')
                                goodpeaksdual=[goodpeaksdual double(maxintdual(peaks(i)))];
                                goodpeaksdualraw=[goodpeaksdualraw double(obj.data.dendrites(axonnumber).maxintdual(peaks(i)))];
                            end
                        else
                            goodpeaks=[goodpeaks double(obj.data.spines(ind(i)).intensity)/double(obj.data.dendrites(axonnumber).meanback)];
                        end
                    end      
                    if (obj.data.spines(ind(i)).len==0)
                        h=text(peakdistance(i),0.1*(max(maxint)-min(maxint))+maxint(peaks(i)),num2str(obj.data.spines(ind(i)).label)); 
                        set(h,'Color',colorvec);
                    else
                        h=plot(peakdistance(i),double(obj.data.spines(ind(i)).intensity)/double(meanback),'r+');
                        set(h,'Color',colorvec);
                        h=text(peakdistance(i),double(0.1*(max(maxint)-min(maxint))+(obj.data.spines(ind(i)).intensity/obj.data.dendrites(axonnumber).meanback)),num2str(obj.data.spines(ind(i)).label));       
                        set(h,'Color',colorvec);
                    end
                    
                end
          %      clipboard('copy',printmatrix([goodpeaks;goodpeaksdual;goodpeaksraw;goodpeaksdualraw]'));
            end       
        end
    else
        clf;
    end
end
figure(obj.gh.mainGUI.Figure);