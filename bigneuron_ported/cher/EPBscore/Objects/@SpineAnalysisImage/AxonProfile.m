function obj = AxonProfile(obj)
if (obj.state.display.ch1==1) && isfield(obj.data.ch(1),'filteredArray')
    ch=1;
end

if (obj.state.display.ch2==1) && isfield(obj.data.ch(2),'filteredArray')
    ch=2;
end


if isfield(obj.data,'dendrites')
    if isfield(obj.data,'spines') && (size(obj.data.spines,1)>0)
        obj.data.spines=obj.data.spines([obj.data.spines.len]>0);
        for i=1:size(obj.data.spines,2)
            obj.data.spines(i).label=i;
            % recalculate intensities for terminal boutons
            
%             spine=obj.data.spines(i).voxel;
%             dendrite=double(obj.data.dendrites(obj.data.spines(i).den_ind).voxel);
%             
%             
%             
%             left=max(spine(2,2)-7,1);
%             right=min(spine(2,2)+7,size(obj.data.imageArray,1));
%             up=max(spine(1,2)-7,1);
%             down=min(spine(1,2)+7,size(obj.data.imageArray,2));
%             top=max(spine(3,2)-3,1);
%             bottom=min(spine(3,2)+3,size(obj.data.imageArray,3)); 
%             [maxval,maxindz]=max(obj.data.imageArray(up:down,left:right,top:bottom),[],3);
%             [maxval,maxindy]=max(maxval,[],1);
%             [maxval,maxindx]=max(maxval,[],2);
%             maxindy=maxindy(maxindx);
%             maxindz=maxindz(maxindy,maxindx);
%             maxindx=min(max(spine(2,2)-8+maxindx,1),size(obj.data.imageArray,1));
%             maxindy=min(max(spine(1,2)-8+maxindy,1),size(obj.data.imageArray,2));
%             maxindz=min(max(spine(3,2)-4+maxindz,1),size(obj.data.imageArray,3));
%             
%             
%             dis=sqrt((dendrite(1,:)-spine(1,1)).*(dendrite(1,:)-spine(1,1))+(dendrite(2,:)-spine(2,1)).*(dendrite(2,:)-spine(2,1))+(dendrite(3,:)-spine(3,1)).*(dendrite(3,:)-spine(3,1)));
%             [a,ind]=min(dis);
%             spine=[dendrite(1:3,ind)';maxindy maxindx maxindz;maxindy maxindx maxindz]';
%             
%             obj.parameters.xspacing=0.083;
%             obj.parameters.yspacing=0.083;
%             obj.parameters.zspacing=1;
%             xs=obj.parameters.xspacing;
%             ys=obj.parameters.yspacing;
%             zs=obj.parameters.zspacing;
%             obj.data.spines(i).voxel=spine;
%             a=sqrt((dendrite(1,ind)-spine(1,2)).*(dendrite(1,ind)-spine(1,2))*xs*xs+(dendrite(2,ind)-spine(2,2)).*(dendrite(2,ind)-spine(2,2))*ys*ys+(dendrite(3,ind)-spine(3,2)).*(dendrite(3,ind)-spine(3,2))*zs*zs);
%             
%             obj.data.spines(i).dendis=ind;
%             obj.data.spines(i).len=a;
%             obj.data.spines(i).label=i;
%             obj.data.spines(i).intensity=(double(obj.data.imageArray(spine(1,2),spine(2,2),spine(3,2)))-obj.data.imagemedian);         
%             obj.data.spines(i).intensitygaussian=[];            
            
            
            
            
            
            %             [maxval,maxindz]=max(obj.data.gaussianArray(up:down,left:right,top:bottom),[],3);
            %             [maxval,maxindy]=max(maxval,[],1);
            %             [maxval,maxindx]=max(maxval,[],2);
            %             maxindy=maxindy(maxindx);
            %             maxindz=maxindz(maxindx,maxindy);
            %             maxindx=min(max(spine(2,2)-8+maxindx,1),size(obj.data.filteredArray,1));
            %             maxindy=min(max(spine(1,2)-8+maxindy,1),size(obj.data.filteredArray,2));
            %             maxindz=min(max(spine(3,2)-4+maxindz,1),size(obj.data.filteredArray,3));
            %             matrix=obj.data.gaussianArray(up:down,left:right,top:bottom);
            %             obj.data.spines(i).intensitygaussian=mean(matrix(:));    
        end
        if isempty(obj.data.spines)
            obj.data.spines=[];
        end
    else
        obj.data.spines=[];
    end
    
    try
        
        for axonnumber=1:size(obj.data.dendrites,2)
            
            
            % find maxint based on the cube around it on raw images
            
            %                 plane=[];
            %                 neighborhood=obj.parameters.threedma.axoncaliberxy;
            % %                neighborhood=3;
            %                 height=0;%obj.parameters.threedma.axoncaliberz-1;
            %                 for j=-height:height  %height focal section up and one plane down
            %                     for k=1:neighborhood*2-1  %crosssection; neighborhood pixels away on each side
            %                         crossdx=-(k-neighborhood).*dy;
            %                         crossdy=(k-neighborhood).*dx;
            %                         % use reflective boundary
            %                         x=abs(round(voxel(2,:)+crossdx-1));
            %                         x=maxx-abs(maxx-x-1)-1;
            %                         y=abs(round(voxel(1,:)-1+crossdy));
            %                         y=maxy-abs(maxy-y-1)-1;
            %                         z=abs(round(voxel(3,:)-1+j));
            %                         z=maxz-abs(maxz-z-1)-1;
            %                         %  plane=[plane; obj.data.filteredArray(y+x*maxy+z*maxy*maxx+1)];
            %                         plane=[plane; obj.data.filteredArray(y+x*maxy+z*maxy*maxx+1)];
            %                     end
            %                 end
            %                 plane=double(plane)-obj.data.imagemedian;  % subtract the image background    
            %                 if (size(plane,1)>1)
            %                     maxintcube=mean(plane);
            %                 else
            %                     maxintcube=plane;
            %                 end
            %                 
            %                 maxintcube=smooth([fliplr(maxintcube(1:14)) maxintcube fliplr(maxintcube(end-13:end))],7)';       % smooth the skeleton with reflecting boundary condition
            %                 maxintcube=double(maxintcube(15:end-14));
            %                 
            %                 maxint=maxintcube;
            %                 
            %                % scale to same level as rawtrace
            %                 stretchraw=std(maxintraw)/std(maxint);
            %                 shiftraw=mean(maxintraw)-mean(maxint)*stretchraw;
            %              %   obj.data.dendrites(axonnumber).maxintraw=(maxintraw-shiftraw)/stretchraw;
            %                 obj.data.dendrites(axonnumber).shiftraw=shiftraw;
            %                 obj.data.dendrites(axonnumber).stretchraw=stretchraw;
            
            
            
            
            %                 % recalculate meanback
            %                 %filter histogram
            %                   % A smooth histogram
            %                   in=maxint;                  
            %                   min_val = min(in);
            %                   max_val = max(in);
            %                   interval = (max_val-min_val)/199;
            %                   border = 16;
            %                   max_val = max_val+border*interval;
            %                   min_val = min_val-border*interval;
            %                   [histogram,bins] = hist(in,200);
            %                   hh=[0.0014795   0.0038042   0.0087535    0.018023    0.033208     0.05475    0.080775     0.10664     0.12598     0.13318     0.12598     0.10664    0.080775     0.05475    0.033208    0.018023   0.0087535   0.0038042   0.0014795];
            %                   histogram=conv(histogram,hh);
            %                   
            %                   % Find peak
            %                   [max_value,max_element] = max(histogram);
            % 
            %                 
            %                 
            %                 
            %                %  threshold_new=(max_value-min(in))*parameter+min(in);
            %                 
            % 
            %                 meanback=bins(max_element);
            %                 %meanback=median(maxint);
            %                 meanback=mean(medianfiltered);
            %                 
            %                 
            %                 figure;
            %                 hold on;
            %                 plot((maxintraw-shiftraw)/stretchraw,'b');
            %                 plot(maxintcube,'r');
            %                 
            %                 stretchraw=std(maxintcubemax)/std(maxint);
            %                 shiftraw=mean(maxintcubemax)-mean(maxint)*stretchraw;
            %                 
            %                 plot((maxintcubemax-shiftraw)/stretchraw,'g');
            
            %                 obj.data.dendrites(axonnumber).maxintcorrected=maxint;
            % %                 obj.data.dendrites(axonnumber).maxintcube=maxint;
            % %                 obj.data.dendrites(axonnumber).maxintcubemax=maxintcubemax;
            % %                 obj.data.dendrites(axonnumber).maxintcubegaussian=maxintcubegaussian;
            % % %                obj.data.dendrites(axonnumber).maxintcuberaw=maxintcuberaw;
            
            
            if 0        %don't use karel's function
                maxint=maxint-medianfiltered;
                maxint=maxint/meanback;
                left=maxint(maxint<1);
                thres=1+std([left-1 1-left])*obj.parameters.threedma.varicositythreshold;    % choose the threshold based on gaussian distribution of background
                obj.data.dendrites(axonnumber).thres=thres;               
                
                
                % find peaks
                lastint=0;
                mode=0;
                peaknum=0;
                peaks=[];
                localmin=[];
                localmax=[];
                currentmax=0;
                currentmin=0;
                currentmaxpos=0;
                currentminpos=0;
                noiselevel=mean(abs(diff(maxint)));
                maxint=[0 maxint 0]; % make first and last point subthreshold
                if thres<max(maxint)
                    try
                        for i=1:size(maxint,2)
                            if (mode==0)    % under threshold
                                if ((lastint<thres*0.8)&(maxint(i)>thres*0.8))
                                    mode=1;
                                    currentmax=thres*0.8;
                                end
                            end
                            if (mode==1)    % find local maxima
                                if (maxint(i)>currentmax)
                                    currentmax=maxint(i);
                                    currentmaxpos=i;
                                end
                                if ((maxint(i)<maxint(i-1)-noiselevel)&(maxint(i)<meanback+(currentmax-meanback)*0.8))   %start of a local dip
                                    mode=2;
                                    localmax=[localmax currentmaxpos];
                                    currentmin=maxint(i);
                                    currentminpos=i;
                                end
                            end    
                            if (mode==2)    % find local minima
                                if (maxint(i)<currentmin)
                                    currentmin=maxint(i);
                                    currentminpos=i;
                                end
                                if (maxint(i)>currentmin+meanback*0.5)    % start of a local peak
                                    localmin=[localmin currentminpos];
                                    mode=1;
                                    currentmax=maxint(i);
                                    currentmaxpos=i;
                                end
                            end
                            if (mode~=0) & (maxint(i)<thres*0.8) % end of peak
                                if (size(localmin,2)==size(localmax,2))
                                    localmax=[localmax currentmaxpos];
                                end
                                %determine which max/min is real
                                for j=1:size(localmin,2)
                                    if ((localmin(j)-localmax(j)<3) | (localmax(j+1)-localmin(j)<3) | (maxint(localmin(j))>maxint(localmax(j+1))-0.7*meanback) ...
                                            |(maxint(localmin(j))>maxint(localmax(j))-0.7*meanback)) 
                                        % if the peaks are too close or the peak not high enough, the valley is not a true valley
                                        localmin(j)=0;
                                        if (maxint(localmax(j))>maxint(localmax(j+1)))
                                            localmax(j+1)=localmax(j);
                                            localmax(j)=0;
                                        else
                                            localmax(j)=0;
                                        end
                                    end
                                end
                                localmax=localmax(find(localmax));
                                localmax=localmax(find(maxint(localmax)>thres));                                                        %max has to be larger than threshold
                                mode=0;
                                peaks=[peaks localmax];
                                localmax=[];
                                localmin=[];
                                currentmax=0;
                                currentmin=0;
                                currentmaxpos=0;
                                currentminpos=0;
                            end
                        end
                    catch
                        i    
                    end
                    peaks=peaks-1;
                end
            else
                maxint=double(obj.data.dendrites(axonnumber).maxint)/obj.data.dendrites(axonnumber).meanback;
                if ~isfield(obj.parameters.threedma,'absvaricositythreshold')
                    obj.parameters.threedma.absvaricositythreshold=1.3;  
                end
                if ~isfield(obj.parameters.threedma,'relvaricositythreshold')
                    obj.parameters.threedma.relvaricositythreshold=0.5;
                end
                if ~isfield(obj.parameters.threedma,'maxboutonwidth')
                    obj.parameters.threedma.maxboutonwidth=25;
                end
                if ~isfield(obj.parameters.threedma,'minboutonwidth')
                    obj.parameters.threedma.minboutonwidth=10;
                end
                absthres=obj.parameters.threedma.absvaricositythreshold;  
                relthres=obj.parameters.threedma.relvaricositythreshold;   
                obj.data.dendrites(axonnumber).absthres=absthres;
                obj.data.dendrites(axonnumber).relthres=relthres;
               
                peaks=findTerminals(maxint./median(maxint), relthres, absthres,obj.parameters.threedma.maxboutonwidth, obj.parameters.threedma.minboutonwidth);
                
                goodpeaks=findTerminals(maxint./median(maxint), relthres, absthres,obj.parameters.threedma.maxboutonwidth, obj.parameters.threedma.minboutonwidth);
                
%                goodpeaks=findTerminals(maxint./median(maxint), 0.5, 1.3, 35,10);
            end
            voxel=round(double(obj.data.dendrites(axonnumber).voxel));
            
            spines=[];
            numberofspines=0;
            if (size(peaks,2)>0)
                spines=[struct('voxel',[voxel(:,peaks(1)) voxel(:,peaks(1))],'color',[0.7 0.7 0.7],'edittype',0,'den_ind',axonnumber,'dendis',peaks(1),'type',2,'len',0,'intensity',maxint(peaks(1)),'label',0,'voxels',1)];    
                for j=1:size(peaks,2)
                    spines(j+numberofspines)=struct('voxel',[voxel(:,peaks(j)) voxel(:,peaks(j))],'color',[0.7 0.7 0.7],'edittype',2,'den_ind',axonnumber,'dendis',peaks(j),'type',2,'len',0,'intensity',maxint(peaks(j)),'label',0,'voxels',1);
                    
                    maxx=size(obj.data.ch(ch).imageArray,2);
                    maxy=size(obj.data.ch(ch).imageArray,1);
                    maxz=size(obj.data.ch(ch).imageArray,3);
                    left=min(max(spines(j+numberofspines).voxel(2,1)-5,1),maxx);
                    right=min(max(spines(j+numberofspines).voxel(2,1)+5,1),maxx);
                    up=min(max(spines(j+numberofspines).voxel(1,1)-5,1),maxy);
                    down=min(max(spines(j+numberofspines).voxel(1,1)+5,1),maxy);
                    top=min(max(spines(j+numberofspines).voxel(3,1)-1,1),maxz);
                    bottom=min(max(spines(j+numberofspines).voxel(3,1)+1,1),maxz);
                    spines(j+numberofspines).intensity=double(max(max(max(obj.data.ch(ch).imageArray(up:down,left:right,top:bottom)))))-obj.data.ch(ch).imagemedian;
                  %  spines(j+numberofspines).intensitygaussian=double(max(max(max(obj.data.gaussianArray(up:down,left:right,top:bottom)))))-obj.data.imagemedian;
                    
                    if (ismember(peaks(j),goodpeaks))
                        spines(j+numberofspines).edittype=0;
                    else
                        spines(j+numberofspines).edittype=2;
                    end
                end
                numberofspines=size(spines,2);
                
                for j=1:numberofspines
                    spines(j).color=hsv2rgb([(j-5*floor(j/5))*0.2+0.2*j/numberofspines,1,1]);
                    spines(j).label=j;
                end
            end
            numofelements=size(obj.data.spines,2);
            for j=1:size(spines,2)
                names=fieldnames(spines);
                for k=1:size(names,1)
                    obj.data.spines(numofelements+1).(char(names(k)))=spines(j).(char(names(k)));                    
                end
                numofelements=numofelements+1;
            end
        end
        
    catch
        disp('Error when performing axonprofile.');
        disp(lasterr);
    end
    for i=1:size(obj.data.spines,2)
        obj.data.spines(i).label=i;
    end
    obj.data.slicespines={};
    for i=1:size(obj.data.ch(ch).imageArray,3)
        ind=1;
        %   obj.data.slicespines(i)=struct([]);
        for j=1:size(obj.data.spines,2)
            pos=find(obj.data.spines(j).voxel(3,:)==i);
            if (pos)
                obj.data.slicespines(i).spines(ind)=struct('voxel',[obj.data.spines(j).voxel(1,pos);obj.data.spines(j).voxel(2,pos)],'index',j);
                ind=ind+1;    
            end
        end
    end;
    obj.state.display.displayrejectedspine=1;
end