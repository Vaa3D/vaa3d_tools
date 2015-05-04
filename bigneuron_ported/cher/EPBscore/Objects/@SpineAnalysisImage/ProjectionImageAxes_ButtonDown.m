function obj = ProjectionImageAxes_ButtonDown(obj)
global self;
global ogh;
ogh=obj.gh;
fig=obj.gh.projectionGUI.Figure;
currentpoint=ceil(get(gca,'CurrentPoint'));
type = get(gcbf,'SelectionType');
handles=guidata(fig);

if (obj.state.display.ch1==1) && isfield(obj.data.ch(1),'filteredArray')
     ch=1;
end
if (obj.state.display.ch2==1) && isfield(obj.data.ch(2),'filteredArray')
     ch=2;
end
           
key = get(fig,'CurrentKey');
switch key
    case 'a'
        maxspine = 0;i1=0;j1=0;z1=0;
        for i=((currentpoint(1,1)-1)-5):((currentpoint(1,1)-1)+5)
               for j=((currentpoint(1,2)-1)-5):((currentpoint(1,2)-1)+5)
                    [f,z]=max(obj.data.ch(ch).filteredArray(j,i,obj.state.ROI.minz:obj.state.ROI.maxz));
                    if maxspine<f
                          maxspine=f;
                          i1=i;j1=j;z1=z;
                    end
               end
        end
        fprintf(strcat('******:i=',num2str(i1),';j=',num2str(j1),';z=',num2str(z1),';light=',num2str(maxspine),'\n'));
        %eval([
        self=struct(obj);
        %eval([('obj.state.display =setfield( self.state.display,''positionx'',i1);')]);
        self.state.display =setfield( self.state.display,'positionx',i1);
        self.state.display =setfield( self.state.display,'positiony',j1);
        self.state.display =setfield( self.state.display,'currentz',z1);
        self.state.display =setfield( self.state.display,'editbox',maxspine);
        %set(obj.state.display,'positiony',num2str(j1));
        %set(obj.state.display,'positionz',num2str(z1));
        %set(obj.state.display,'maxz',num2str(maxspine));
        updateGUIVars(ogh.mainGUI.positionx);
        updateGUIVars(ogh.mainGUI.positiony);
        updateGUIVars(ogh.mainGUI.positionz);
        updateGUIVars(ogh.mainGUI.editbox);
        return;
end

switch obj.state.display.mousemode
    case 'editspines'
         set(ogh.mainGUI.EditMarks,'Checked','off');
         set(ogh.mainGUI.EditSpines,'Checked','on');
         %if (obj.state.display.ch1==1) && isfield(obj.data.ch(1),'filteredArray')
         %       ch=1;
         %   end
         %   if (obj.state.display.ch2==1) && isfield(obj.data.ch(2),'filteredArray')
         %       ch=2;
         %   end
        switch type
            case 'normal'   % left button Click
                if (obj.state.display.displayfiltered) & isfield(obj.data.ch(ch),'filteredArray')
                    [maxv,obj.state.display.currentz]=max(obj.data.ch(ch).filteredArray(currentpoint(1,2)-1,currentpoint(1,1)-1,obj.state.ROI.minz:obj.state.ROI.maxz));
                    global ROI;
%                     ROI=obj.data.filteredArray((currentpoint(1,2)-10:currentpoint(1,2)+10),(currentpoint(1,1)-10:currentpoint(1,1)+10),(obj.state.display.currentz-1:obj.state.display.currentz+1))
%                     max(max(max(obj.data.filteredArray((currentpoint(1,2)-20:currentpoint(1,2)+20),(currentpoint(1,1)-20:currentpoint(1,1)+20),(obj.state.display.currentz-3:obj.state.display.currentz+3)))))
                    obj.state.display.positionintensity=obj.data.ch(ch).filteredArray(currentpoint(1,2),currentpoint(1,1),obj.state.display.currentz);
                    obj.state.display.positionx=currentpoint(1,1);
                    obj.state.display.positiony=currentpoint(1,2);
                    obj.state.display.positionz=obj.state.display.currentz;
                    
                    self=struct(obj);
                     updateGUIbyGlobal('self.state.display.positionx');
                    updateGUIbyGlobal('self.state.display.positiony');
                    updateGUIbyGlobal('self.state.display.positionintensity');
                    updateGUIbyGlobal('self.state.display.currentz');
                end
                if (obj.state.display.displayraw) & isfield(obj.data.ch(ch),'imageArray')
                    [maxv,obj.state.display.currentz]=max(obj.data.ch(ch).imageArray(currentpoint(1,2)-1,currentpoint(1,1)-1,obj.state.ROI.minz:obj.state.ROI.maxz));
                   global ROI;
%                     ROI=obj.data.filteredArray((currentpoint(1,2)-10:currentpoint(1,2)+10),(currentpoint(1,1)-10:currentpoint(1,1)+10),(obj.state.display.currentz-1:obj.state.display.currentz+1))
%                     max(max(max(obj.data.filteredArray((currentpoint(1,2)-20:currentpoint(1,2)+20),(currentpoint(1,1)-20:currentpoint(1,1)+20),(obj.state.display.currentz-3:obj.state.display.currentz+3)))))
                    obj.state.display.positionintensity=obj.data.ch(ch).imageArray(currentpoint(1,2),currentpoint(1,1),obj.state.display.currentz);
                    obj.state.display.positionx=currentpoint(1,1);
                    obj.state.display.positiony=currentpoint(1,2);
                    obj.state.display.positionz=obj.state.display.currentz;
                    
                    self=struct(obj);
                     updateGUIbyGlobal('self.state.display.positionx');
                    updateGUIbyGlobal('self.state.display.positiony');
                    updateGUIbyGlobal('self.state.display.positionintensity');
                    updateGUIbyGlobal('self.state.display.currentz');
                end
%                 if (obj.state.display.displaygaussian) & isfield(obj.data,'gaussianArray')
%                     [maxv,obj.state.display.currentz]=max(obj.data.gaussianArray(currentpoint(1,2)-1,currentpoint(1,1)-1,obj.state.ROI.minz:obj.state.ROI.maxz));
%                     global ROI;
%                     ROI=obj.data.imageArray(currentpoint(1,2)-10:currentpoint(1,2)+10,currentpoint(1,1)-10:currentpoint(1,1)+10,(obj.state.display.currentz-1:obj.state.display.currentz+1))                   
%                     max(max(max(obj.data.imageArray((currentpoint(1,2)-20:currentpoint(1,2)+20),(currentpoint(1,1)-20:currentpoint(1,1)+20),(obj.state.display.currentz-3:obj.state.display.currentz+3)))))
%                    
%                     self=struct(obj);
%                     updateGUIbyGlobal('self.state.display.currentz');
%                 end
%                 if (obj.state.display.displaybinary) & isfield(obj.data,'filteredArray')
%                     [maxv,obj.state.display.currentz]=max(obj.data.filteredArray(currentpoint(1,2)-1,currentpoint(1,1)-1,obj.state.ROI.minz:obj.state.ROI.maxz));
%                     self=struct(obj);
%                     updateGUIbyGlobal('self.state.display.currentz');
%                 end
%                 
            case 'extend'   % click both button to add or shift click 
                if (obj.state.display.displayspines)
                    [maxv,maxz]=max(obj.data.ch(ch).filteredArray(currentpoint(1,2)-1,currentpoint(1,1)-1,obj.state.ROI.minz:obj.state.ROI.maxz));
                    maxz=maxz+obj.state.ROI.minz-1;
                    j=size(obj.data.spines,2)+1;
                    obj.data.spines(j).type=0;
                    obj.data.spines(j).color=[0.7,0.7,0.7];
                    obj.data.spines(j).voxels=3;
                    obj.data.spines(j).voxel=[0 0 0;currentpoint(1,2)-1 currentpoint(1,1)-1 maxz;currentpoint(1,2)-1 currentpoint(1,1)-1 maxz]';
                    
                    distances=[];
                    den_ind=obj.state.display.axonnumber;
                    if (den_ind==0)   % if not specified, find the closest neurite             
                        for i=1:size(obj.data.dendrites,2)
                            dendrite=double(obj.data.dendrites(i).voxel);
                            spine=double(obj.data.spines(j).voxel);
                            dis=(dendrite(1,:)-spine(1,2)).*(dendrite(1,:)-spine(1,2))+(dendrite(2,:)-spine(2,2)).*(dendrite(2,:)-spine(2,2))+(dendrite(3,:)-spine(3,2)).*(dendrite(3,:)-spine(3,2));
                            [a,ind]=min(dis);
                            distances=[distances a];
                        end
                        [a,den_ind]=min(distances);
                    end
                    
                    %xuesong added
                    %maxspine = 0;
                    %for i=((currentpoint(1,1)-1)-5):((currentpoint(1,1)-1)+5)
                    %    for j=((currentpoint(1,2)-1)-5):((currentpoint(1,2)-1)+5)
                    %        [f,z]=max(obj.data.ch(ch).filteredArray(j,i,obj.state.ROI.minz:obj.state.ROI.maxz));
                    %        if maxspine<f
                    %            maxspine=f;
                    %            i1=i;j1=j;z1=z;
                    %        end
                    %    end
                    %end
                    %fprintf('i='+num2str(i1)+';j='+num2str(j1)+';z='+num2str(z1)+'/n');
                    %xuesong added
                    
                    %Calculate distances
                    dendrite=double(obj.data.dendrites(den_ind).voxel);
                    spine=double(obj.data.spines(j).voxel);
                    dis=sqrt((dendrite(1,:)-spine(1,2)).*(dendrite(1,:)-spine(1,2))+(dendrite(2,:)-spine(2,2)).*(dendrite(2,:)-spine(2,2))+(dendrite(3,:)-spine(3,2)).*(dendrite(3,:)-spine(3,2)));
                    [a,ind]=min(dis);
                    dendrite=double(obj.data.dendrites(den_ind).voxel);
                    obj.data.spines(j).den_ind=den_ind;
                    if (a<5) % 5 pixels
                        [maxval,maxind]=max(double(obj.data.dendrites(den_ind).maxint(ind-5:ind+5)));
                        ind=ind+maxind-6;
                        obj.data.spines(j).dendis=ind;
                        
                        obj.data.spines(j).voxel(:,1)=uint16(dendrite(1:3,ind));
                        obj.data.spines(j).voxel(:,2)=uint16(dendrite(1:3,ind));
                        obj.data.spines(j).voxel(:,3)=uint16(dendrite(1:3,ind));
                        
                        obj.data.spines(j).dendis=ind;
                        obj.data.spines(j).len=0;
                        obj.data.spines(j).edittype=0;
                        obj.data.spines(j).label=max([obj.data.spines.label])+1;
                        spine=double(obj.data.spines(j).voxel);
                        obj.data.spines(j).intensity=obj.data.dendrites(den_ind).maxint(ind);
                     else
                         % get the dendritic point
                         [X,Y] = getpts;
                         [maxz,z]=max(obj.data.ch(ch).imageArray(floor(Y(1)),floor(X(1)),obj.state.ROI.minz:obj.state.ROI.maxz));
                         z=z+obj.state.ROI.minz-1; 
                         
                         left=max(spine(2,2)-7,1);
                         right=min(spine(2,2)+7,size(obj.data.ch(ch).imageArray,1));
                         up=max(spine(1,2)-7,1);
                         down=min(spine(1,2)+7,size(obj.data.ch(ch).imageArray,2));
                         top=max(spine(3,2)-3,1);
                         bottom=min(spine(3,2)+3,size(obj.data.ch(ch).imageArray,3)); 
                         [maxval,maxindz]=max(obj.data.ch(ch).imageArray(up:down,left:right,top:bottom),[],3);
                         [maxval,maxindy]=max(maxval,[],1);
                         [maxval,maxindx]=max(maxval,[],2);
                         maxindy=maxindy(maxindx);
                         maxindz=maxindz(maxindy,maxindx);
                         maxindx=min(max(left+maxindx-1,1),size(obj.data.ch(ch).imageArray,1));
                         maxindy=min(max(up+maxindy-1,1),size(obj.data.ch(ch).imageArray,2));
                         maxindz=min(max(top+maxindz-1,1),size(obj.data.ch(ch).imageArray,3));
                        
                         
                         
                         % draw an imaginary line from the clicked points
                         % and shift to maximal point.
                         
                         
                         % fill in the intermediate points on the line 
                         % extend out 10 points both directions
                         % define synapse strength as the maximal point on
                         % the smoothed intensity profile on the line in a
                         % position close to the bouton (left and right 10
                         % points)
                         startpoint=[maxindy maxindx maxindz]';
                         
                         left=max(X-7,1);
                         right=min(X+7,size(obj.data.ch(ch).imageArray,1));
                         up=max(Y-7,1);
                         down=min(Y+7,size(obj.data.ch(ch).imageArray,2));
                         top=max(z-3,1);
                         bottom=min(z+3,size(obj.data.ch(ch).imageArray,3)); 
                         [maxval,maxindz]=max(obj.data.ch(ch).imageArray(up:down,left:right,top:bottom),[],3);
                         [maxval,maxindy]=max(maxval,[],1);
                         [maxval,maxindx]=max(maxval,[],2);
                         maxindy=maxindy(maxindx);
                         maxindz=maxindz(maxindy,maxindx);
                         maxindx=min(max(left+maxindx-1,1),size(obj.data.ch(ch).imageArray,1));
                         maxindy=min(max(up+maxindy-1,1),size(obj.data.ch(ch).imageArray,2));
                         maxindz=min(max(top+maxindz-1,1),size(obj.data.ch(ch).imageArray,3));
                         
                         
                         
                         
                         endpoint=[maxindy maxindx maxindz]';
                       %endpoint=[Y X z]';
                         newstartpoint=round(startpoint-(endpoint-startpoint)*10/sqrt(sum((endpoint-startpoint).*(endpoint-startpoint))));
                         newendpoint=round(endpoint+(endpoint-startpoint)*10/sqrt(sum((endpoint-startpoint).*(endpoint-startpoint))));
                         newstartpoint=max([min([newstartpoint';size(obj.data.ch(ch).imageArray,1) size(obj.data.ch(ch).imageArray,2) size(obj.data.ch(ch).imageArray,3)]);1 1 1])';
                         newendpoint=max([min([newendpoint';size(obj.data.ch(ch).imageArray,1) size(obj.data.ch(ch).imageArray,2) size(obj.data.ch(ch).imageArray,3)]);1 1 1])';
                         
                         newvoxels=[]; 
                         for l=0:floor(sqrt(sum((newendpoint-newstartpoint).*(newendpoint-newstartpoint))))
                             newvoxels=[newvoxels round(newstartpoint+(newendpoint-newstartpoint)*l/sqrt(sum((newendpoint-newstartpoint).*(newendpoint-newstartpoint))))];
                         end 
                         %oldvoxels=newvoxels;
                         [newvoxel, newvoxelmax, maxint, dx, dy] = shiftDendriteMax(newvoxels,obj.data.ch(ch).imageArray,1);
                         %[maxint, dx, dy] = shiftDendriteMax(newvoxels,obj.data.ch(ch).imageArray,1);
                         [val,indmax]=max(maxint(1:20));
                         smoothmaxint=reflectivesmooth(double(maxint),7);
                         figure;
                         plot(maxint);
                         hold on;
                         plot(smoothmaxint,'r');
                         
                         
                         
                        dis=sqrt((dendrite(1,:)-Y(1)).*(dendrite(1,:)-Y(1))+(dendrite(2,:)-X(1)).*(dendrite(2,:)-X(1))+(dendrite(3,:)-z).*(dendrite(3,:)-z));
                        [a,ind]=min(dis);
                        spine=[dendrite(1:3,ind)';newvoxelmax(:,indmax)';newvoxelmax(:,indmax)']';
                        
                        obj.parameters.xspacing=0.083;
                        obj.parameters.yspacing=0.083;
                        obj.parameters.zspacing=1;
                        xs=obj.parameters.xspacing;
                        ys=obj.parameters.yspacing;
                        zs=obj.parameters.zspacing;
                        obj.data.spines(j).voxel=spine;
                        a=sqrt((dendrite(1,ind)-spine(1,2)).*(dendrite(1,ind)-spine(1,2))*xs*xs+(dendrite(2,ind)-spine(2,2)).*(dendrite(2,ind)-spine(2,2))*ys*ys+(dendrite(3,ind)-spine(3,2)).*(dendrite(3,ind)-spine(3,2))*zs*zs);
                        
                        obj.data.spines(j).dendis=ind;
                        obj.data.spines(j).len=a;
                        obj.data.spines(j).edittype=0;
                        obj.data.spines(j).label=max([obj.data.spines.label])+1;
                        obj.data.spines(j).intensity=(smoothmaxint(indmax)-obj.data.ch(ch).imagemedian);
                        %obj.data.spines(j).intensity
                    end
                    %         den=data.dendrites(data.spines(j).den_ind).voxel;
                    %         dend=den(:,max(1,data.spines(j).dendis-5):min(size(den,2),data.spines(j).dendis+5));
                    %         coeff=(double([ones(1,size(dend,2)); dend(1,:)]))'\(double(dend(2,:)))';
                    %         obj.data.spines(j).side=sign(double(data.spines(j).voxel(2,2))-coeff(2)*double(data.spines(j).voxel(1,2))-coeff(1));
                    
                    try
                        obj.data.slicespines={};            % Take care of slice spines for display 
                        for i=1:size(obj.data.ch(ch).imageArray,3)
                            ind=1;
                            %   obj.data.slicespines(i)=struct([]);
                            for j=1:size(obj.data.spines,2)
                                try
                                    pos=find(obj.data.spines(j).voxel(3,:)==i-1);
                                catch
                                    i
                                    j
                                end
                                if (pos)
                                    try
                                        obj.data.slicespines(i).spines(ind)=struct('voxel',[obj.data.spines(j).voxel(1,pos);obj.data.spines(j).voxel(2,pos)],'index',j);
                                        ind=ind+1;    
                                    catch
                                        i
                                        j
                                    end
                                end
                            end
                        end;
                        
                    catch
                        i 
                        j
                    end
                end
            case 'alt'    % click right button to delete
                if (obj.state.display.displayspines)
                    [maxv,maxz]=max(obj.data.ch(ch).filteredArray(currentpoint(1,2)-1,currentpoint(1,1)-1,obj.state.ROI.minz:obj.state.ROI.maxz));
                    distance=[];
                    for j=1:size(obj.data.spines,2)
                        distance=[distance sqrt((obj.data.spines(j).voxel(1,2)-currentpoint(1,2)+1).*(obj.data.spines(j).voxel(1,2)-currentpoint(1,2)+1)+(obj.data.spines(j).voxel(2,2)-currentpoint(1,1)+1).*(obj.data.spines(j).voxel(2,2)-currentpoint(1,1)+1))];
                    end
                    [a,ind]=min(distance);
                    if (obj.data.spines(ind).edittype==1)
                        obj.data.spines(ind).edittype=0;
                    else
                        obj.data.spines(ind).edittype=1; %edittype of 1 is deleted
                    end
                    %                     displayImage(obj);
                    %                     displayProjectionImage(obj);
                end
        end
    case 'editmarks'
         set(ogh.mainGUI.EditMarks,'Checked','on');
         set(ogh.mainGUI.EditSpines,'Checked','off');
         if (obj.state.display.ch1==1) && isfield(obj.data.ch(1),'filteredArray')
                ch=1;
            end
            if (obj.state.display.ch2==1) && isfield(obj.data.ch(2),'filteredArray')
                ch=2;
            end
        switch type
            case 'extend'   % shift button Click
                if (obj.state.display.displaymarks)
                    if ~isfield(obj.data,'marks')
                        obj.data.marks=[];
                    end
                    
                    [maxv,maxz]=max(obj.data.ch(ch).filteredArray(currentpoint(1,2)-1,currentpoint(1,1)-1,obj.state.ROI.minz:obj.state.ROI.maxz));
                    j=size(obj.data.marks,2)+1;
                    obj.data.marks(j).type=0;
                    obj.data.marks(j).color=[0.7,0.7,0.7];
                    obj.data.marks(j).voxels=3;
                    obj.data.marks(j).voxel=[0 0 0;currentpoint(1,2)-1 currentpoint(1,1)-1 maxz-1;currentpoint(1,2)-1 currentpoint(1,1)-1 maxz-1]';
                    
                    distances=[];
                    den_ind=obj.state.display.axonnumber;
                    if (den_ind==0)   % if not specified, find the closest neurite             
                        for i=1:size(obj.data.dendrites,2)
                            dendrite=double(obj.data.dendrites(i).voxel);
                            spine=double(obj.data.marks(j).voxel);
                            dis=(dendrite(1,:)-spine(1,2)).*(dendrite(1,:)-spine(1,2))+(dendrite(2,:)-spine(2,2)).*(dendrite(2,:)-spine(2,2))+(dendrite(3,:)-spine(3,2)).*(dendrite(3,:)-spine(3,2));
                            [a,ind]=min(dis);
                            distances=[distances a];
                        end
                        [a,den_ind]=min(distances);
                    end
                    %Calculate distances
                    dendrite=double(obj.data.dendrites(den_ind).voxel);
                    spine=double(obj.data.marks(j).voxel);
                    dis=sqrt((dendrite(1,:)-spine(1,2)).*(dendrite(1,:)-spine(1,2))+(dendrite(2,:)-spine(2,2)).*(dendrite(2,:)-spine(2,2))+(dendrite(3,:)-spine(3,2)).*(dendrite(3,:)-spine(3,2)));
                    [a,ind]=min(dis);
                    
                    dendrite=double(obj.data.dendrites(den_ind).voxel);
                    obj.data.marks(j).den_ind=den_ind;
                    obj.data.marks(j).voxel(:,1)=uint16(dendrite(1:3,ind));
                    obj.data.marks(j).dendis=ind;
                    obj.data.marks(j).edittype=0;
                    obj.data.marks(j).label=0;
                    obj.data.marks(j).label=max([obj.data.marks([obj.data.marks.den_ind]==den_ind).label])+1;
                end
            case 'alt'    % click right button to delete
                if (obj.state.display.displaymarks)
                    [maxv,maxz]=max(obj.data.ch(ch).filteredArray(currentpoint(1,2)-1,currentpoint(1,1)-1,obj.state.ROI.minz:obj.state.ROI.maxz));
                    
                    % find out which dendrite to work on:
                    den_ind=obj.state.display.axonnumber;
                    samedendritemarks=[obj.data.marks.den_ind]==den_ind;
                    
                    distance=[];
                    % currentpoint (x,y)
                    % voxel (y,x)
                    for j=1:size(obj.data.marks,2)
                        dx=double(obj.data.marks(j).voxel(1,1))-currentpoint(1,2)+1;
                        dy=double(obj.data.marks(j).voxel(2,1))-currentpoint(1,1)+1;
                        distance=[distance sqrt(dx*dx+dy*dy)];
                    end
                    distance(~samedendritemarks)=10000;
                    [a,ind]=min(distance);
                    
                    den_ind=obj.data.marks(ind).den_ind;
                    obj.data.marks(ind)=[];
                    samedendritemarks=find([obj.data.marks.den_ind]==den_ind);
                    inds=1:size(samedendritemarks,2);
                    for i=1:size(samedendritemarks,2)
                        obj.data.marks(samedendritemarks(i)).label=inds(i);
                    end
                end        
        end
end