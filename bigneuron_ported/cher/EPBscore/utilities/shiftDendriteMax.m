function [voxel, voxelmax, maxint, dx, dy] = shiftDendriteMax(voxel,imageArray,restricted)

%fix the neurite so it coincides with the maximal intensity in the perpendicular plane

maxx=size(imageArray,2);
maxy=size(imageArray,1);
maxz=size(imageArray,3);

arraysize=size(voxel,2);

   voxel=single(voxel);

for i=1:1
    
    %fix the neurite so it coincides with the maximal intensity in the perpendicular plane
    
    %calculate the instantaneous slope in X-Y plane use neighborhood of 5 pixels to each side
    sizeofaxon=size(voxel,2);
    
    %calculate the normal direction of the axon based on 3 pixels on
    %each side
    oldvoxel=voxel;
%         if (sizeofaxon>7)
%             %        smoothing again to futher suppress jumps
%             voxel(1,:)=medfilt1(voxel(1,:),14);
%             voxel(2,:)=medfilt1(voxel(2,:),14);
%             voxel(3,:)=medfilt1(voxel(3,:),14);
%             voxel(1,:)=smooth(voxel(1,:),7)';
%             voxel(2,:)=smooth(voxel(2,:),7)';
%             voxel(3,:)=smooth(voxel(3,:),7)';
%         end
 
    neighborhood=22;
    if (sizeofaxon>neighborhood)
        dy=([voxel(1,1)*ones(1,neighborhood/2) voxel(1,1:sizeofaxon-neighborhood/2)]-[voxel(1,neighborhood/2+1:sizeofaxon) voxel(1,sizeofaxon)*ones(1,neighborhood/2)]);
        dx=([voxel(2,1)*ones(1,neighborhood/2) voxel(2,1:sizeofaxon-neighborhood/2)]-[voxel(2,neighborhood/2+1:sizeofaxon) voxel(2,sizeofaxon)*ones(1,neighborhood/2)]);
    else
        dx=ones(1,sizeofaxon).*(voxel(1,1:sizeofaxon)-voxel(1,1));
        dy=ones(1,sizeofaxon).*(voxel(2,1:sizeofaxon)-voxel(2,1));
    end
    dr=sqrt(dx.*dx+dy.*dy);
    %    change length of 0 to 1 to avoid division by zero
    dr(find(dr==0))=1;
    %    normalize to unit vector length
    dx=dx./dr;
    dy=dy./dr;
    dx=smooth(dx,21)';
    dy=smooth(dy,21)';
    newvoxel=round(voxel);
    voxel=oldvoxel;
    
    maxpath=0;
    if (maxpath)
        %define the search region
        mask=zeros(size(imageArray),'uint8');
        mask(newvoxel(1,:)+(newvoxel(2,:)-1)*maxy+(newvoxel(3,:)-1)*maxy*maxx)=1;
        neighborhood=35;
        height=4;
        se=strel(ones(neighborhood*2+1,neighborhood*2+1,height*2+1,'uint8'));
        mask=imdilate(mask,se);
        o=find(mask);
        mlength=length(o(:));
        tlength=length(imageArray(:));
        o=[];
        graph_crop=sparse([],[],[],tlength,tlength,mlength);
        
        %zeros(length(o(:))*26,3,'double');        
        % assume 26 connectivity
        count=1;
        for di=[-1 0 1]
            for dj=[-1 0 1]
                for dk=[-1 0 1]
                    % if ~(di==0 && dj==0 && dk==0)
                    if (dk>0) || (dk==0 && dj>0) || (dk==0 && dj==0 && di>0)
                        
                        indblock=zeros(size(mask),'uint8');
                        
                        indblock(max(1,1-di):(end+min(-di,0)),max(1,1-dj):(end+min(-dj,0)),max(1,1-dk):(end+min(-dk,0)))=1;
                        indblock=indblock&mask;
                        o=find(indblock);
                        
                  
                        
                        
                        offset=di+dj*size(mask,1)+dk*size(mask,1)*size(mask,2);
                        
                       
                        % add distance for existing nodes need to figure
                        % out right metric here
                        %d=sqrt(di.*di+dj.*dj+dk.*dk);
                        graph_crop=graph_crop+sparse(o,o+offset,10000-double(min([imageArray(o) imageArray(o+offset)],[],2)),tlength,tlength);
                    end
                end
            end
        end
        o=[];
        indblock=[];
        mask=[];

        graph_crop=graph_crop+graph_crop';
                    
        ind=newvoxel(:,end);
        ending=sub2ind(size(imageArray),ind(1),ind(2),ind(3));
        
        ind=newvoxel(:,1);
        starting=sub2ind(size(imageArray),ind(1),ind(2),ind(3));
        
        
        % first do mst based path, this uses only diagonal
        % connections
        [d pred] = shortest_paths(graph_crop,starting);
        
        
        % trace from end to start
        
        p=ending;
        d=[];
        
        %midline=zeros(size(mask));
        x=[];
        y=[];
        z=[];
        while p~=starting
            [yi,xi,zi]=ind2sub(size(imageArray),p);
            x=[x xi];
            y=[y yi];
            z=[z zi];
            %midline(yi,xi,zi)=1;
            p=pred(p);
        end
        %figure
        %imagesc(max(midline,[],3))
        maxint=imageArray(y+(x-1)*maxy+(z-1)*maxy*maxx);
       
         voxel=[y; x; z];
         
         voxel=fliplr(voxel);
         maxint=fliplr(maxint);
       
         sizeofaxon=size(voxel,2);
           voxelmax=voxel;
    else
        voxel(1,:)=smooth(voxel(1,:),7)';
        voxel(2,:)=smooth(voxel(2,:),7)';
        voxel(3,:)=smooth(voxel(3,:),7)';
        
        
        %iterate this process
        
        maxx=size(imageArray,2);
        maxy=size(imageArray,1);
        maxz=size(imageArray,3);
        
        %find maxint as center of line
        x=round(min(max(voxel(2,:)-1,0),maxx-1));
        y=round(min(max(voxel(1,:)-1,0),maxy-1));
        z=round(min(max(voxel(3,:)-1,0),maxz-1));
        
        maxint=imageArray(y+x*maxy+z*maxy*maxx+1);
        smoothmaxint=smooth(double(maxint),7);
        
        
        if (restricted)
            plane=[];
            line=[];
            neighborhood=3;
            height=1;
            inds=[];
            zs=[];
            for j=-height:height   %five focal section up and three plane down
                plane=[];
                for k=1:neighborhood*2+1  %crosssection; neighborhood pixels away on each side
                    crossdx=-(k-neighborhood-1).*dy;
                    crossdy=(k-neighborhood-1).*dx;
                    x=round(min(max(voxel(2,:)-1+crossdx,0),maxx-1));
                    y=round(min(max(voxel(1,:)-1+crossdy,0),maxy-1));
                    z=round(min(max(voxel(3,:)-1+j,0),maxz-1));
                    plane=[plane; imageArray(y+x*maxy+z*maxy*maxx+1)];
                end
                [maxinplane,ind]=max(plane);
                line=[line; maxinplane]; %obj.data.filteredArray(y+x*maxy+z*maxy*maxx+1)];
                inds=[inds; ind];
                zs=[zs; z];
            end
            
            if (height>2)
                %find local minima
                
                line=double(line);
                
                minima=imregionalmax(imhmin(line,median(double(maxint))/3,[0 1 0; 0 1 0; 0 1 0]),[0 1 0; 0 1 0; 0 1 0]);
                
                %find local the closest local minima
                indz=zeros(1,size(voxel,2));
                for j=0:height
                    [maxv,ind]=max([minima(height-j+1,:).*line(height-j+1,:);minima(height+j+1,:).*line(height+j+1,:)]);
                    ind=(maxv>0).*floor(height+j*2*(ind-1.5)+1);
                    indz=indz+(indz==0).*ind;
                end
            else
                %global minima
                if height==0
                    indz=ones(size(line,2),1);
                else
                    [val,indz]=max(line,[],1);
                end
            end
            inds=inds';
            ind=inds((indz-1)*size(line,2)+(1:size(line,2)));
            indx=-(ind-neighborhood-1).*dy;
            indy=(ind-neighborhood-1).*dx;
            indz=indz-height-1;
            voxel(2,:)=round(min(max(voxel(2,:)+indx,1),maxx));
            voxel(1,:)=round(min(max(voxel(1,:)+indy,1),maxy));
            voxel(3,:)=round(min(max(voxel(3,:)+indz,1),maxz));
            voxelmax=voxel;
            voxel(2,:)=min(max(voxel(2,:),1),maxx);
            voxel(1,:)=min(max(voxel(1,:),1),maxy);
            voxel(3,:)=min(max(voxel(3,:),1),maxz);
            % find maxint as center of line
            x=round(min(max(voxel(2,:)-1,0),maxx-1));
            y=round(min(max(voxel(1,:)-1,0),maxy-1));
            z=round(min(max(voxel(3,:)-1,0),maxz-1));
            
            maxint=imageArray(y+x*maxy+z*maxy*maxx+1);
            
            RMS=sqrt(mean((smoothmaxint-smooth(double(maxint),7)).^2));
            disp(['RMS error for step ' num2str(i) ': ' num2str(RMS)]);
            smoothmaxint=smooth(double(maxint),7);
           
            
        else
            plane=[];
            line=[];
            neighborhood=15;
            height=5;
            inds=[];
            zs=[];
            for j=-height:height   %three focal section up and three plane down
                plane=[];
                for k=1:neighborhood*2+1  %crosssection; neighborhood pixels away on each side
                    crossdx=-(k-neighborhood-1).*dy;
                    crossdy=(k-neighborhood-1).*dx;
                    x=round(min(max(voxel(2,:)-1+crossdx,0),maxx-1));
                    y=round(min(max(voxel(1,:)-1+crossdy,0),maxy-1));
                    z=round(min(max(voxel(3,:)-1+j,0),maxz-1));
                    plane=[plane; imageArray(y+x*maxy+z*maxy*maxx+1)];
                end
                [maxinplane,ind]=max(plane);
                line=[line; maxinplane]; %obj.data.filteredArray(y+x*maxy+z*maxy*maxx+1)];
                inds=[inds; ind];
                zs=[zs; z];
            end
            
            if (height>5)
                %find local minima
                
                line=double(line);
                
                minima=imregionalmax(imhmin(line,median(double(maxint))/3,[0 1 0; 0 1 0; 0 1 0]),[0 1 0; 0 1 0; 0 1 0]);
                
                %find local the closest local minima
                indz=zeros(1,size(voxel,2));
                for j=0:height
                    [maxv,ind]=max([minima(height-j+1,:).*line(height-j+1,:);minima(height+j+1,:).*line(height+j+1,:)]);
                    ind=(maxv>0).*floor(height+j*2*(ind-1.5)+1);
                    indz=indz+(indz==0).*ind;
                end
            else
                %global minima
                if height==0
                    indz=ones(size(line,2),1)';
                else
                    [val,indz]=max(line,[],1);
                end
            end
            inds=inds';
            ind=inds((indz-1)*size(line,2)+(1:size(line,2)));
            if height==0
                ind=ind';
            end
            indx=-(ind-neighborhood-1).*dy;
            indy=(ind-neighborhood-1).*dx;
            indz=indz-height-1;
            voxel(2,:)=(min(max(voxel(2,:)+indx,1),maxx));
            voxel(1,:)=(min(max(voxel(1,:)+indy,1),maxy));
            voxel(3,:)=(min(max(voxel(3,:)+indz,1),maxz));
            voxelmax=voxel;
            
            %         if (sizeofaxon>7)
            %             % smoothing again to futher suppress jumps
            %             voxel(1,:)=medfilt1(voxel(1,:),14);
            %             voxel(2,:)=medfilt1(voxel(2,:),14);
            %             voxel(3,:)=medfilt1(voxel(3,:),14);
            %             voxel(1,:)=smooth(voxel(1,:),7)';
            %             voxel(2,:)=smooth(voxel(2,:),7)';
            %             voxel(3,:)=smooth(voxel(3,:),7)';
            %         end
            voxel(2,:)=min(max(voxel(2,:),1),maxx);
            voxel(1,:)=min(max(voxel(1,:),1),maxy);
            voxel(3,:)=min(max(voxel(3,:),1),maxz);
            %find maxint as center of line
            x=round(min(max(voxel(2,:)-1,0),maxx-1));
            y=round(min(max(voxel(1,:)-1,0),maxy-1));
            z=round(min(max(voxel(3,:)-1,0),maxz-1));
            
            maxint=imageArray(y+x*maxy+z*maxy*maxx+1);
            
            RMS=sqrt(mean((smoothmaxint-smooth(double(maxint),7)).^2));
            disp(['RMS error for step ' num2str(i) ': ' num2str(RMS)]);
            smoothmaxint=smooth(double(maxint),7);
            
            
            %         if (sizeofaxon>7)
            %             voxel=[voxel(:,1)*ones(1,30) voxel voxel(:,end)*ones(1,30)];
            %             % smoothing again to futher suppress jumps
            %             voxel(1,:)=medfilt1(voxel(1,:),20);
            %             voxel(2,:)=medfilt1(voxel(2,:),20);
            %             voxel(3,:)=medfilt1(voxel(3,:),20);
            %             voxel=voxel(:,31:end-30);
            %         end
            %
            %         neighborhood=14;
            %         if (sizeofaxon>neighborhood)
            %             dy=([voxel(1,1)*ones(1,neighborhood) voxel(1,1:sizeofaxon-neighborhood)]-[voxel(1,neighborhood+1:sizeofaxon) voxel(1,sizeofaxon)*ones(1,neighborhood)]);
            %             dx=([voxel(2,1)*ones(1,neighborhood) voxel(2,1:sizeofaxon-neighborhood)]-[voxel(2,neighborhood+1:sizeofaxon) voxel(2,sizeofaxon)*ones(1,neighborhood)]);
            %         else
            %             dx=ones(1,sizeofaxon)*(voxel(1,1:sizeofaxon)-voxel(1,1));
            %             dy=ones(1,sizeofaxon)*(voxel(2,1:sizeofaxon)-voxel(2,1));
            %         end
            %         dr=sqrt(dx.*dx+dy.*dy);
            %         %change length of 0 to 1 to avoid division by zero
            %         dr(find(dr==0))=1;
            %         %normalize to unit vector length
            %         dx=dx./dr;
            %         dy=dy./dr;
            
            %         plane=[];
            %         line=[];
            %         neighborhood=5;
            %         height=5;
            %         inds=[];
            %         zs=[];
            %         for j=-height:height   %five focal section up and five plane down
            %             plane=[];
            %             for k=1:neighborhood*2+1  %crosssection; neighborhood pixels away on each side
            %                 crossdx=-(k-neighborhood-1).*dy;
            %                 crossdy=(k-neighborhood-1).*dx;
            %                 x=round(min(max(voxel(2,:)-1+crossdx,0),maxx-1));
            %                 y=round(min(max(voxel(1,:)-1+crossdy,0),maxy-1));
            %                 z=round(min(max(voxel(3,:)-1+j,0),maxz-1));
            %                 plane=[plane; imageArray(y+x*maxy+z*maxy*maxx+1)];
            %             end
            %             [maxinplane,ind]=max(plane);
            %             line=[line; maxinplane]; %obj.data.filteredArray(y+x*maxy+z*maxy*maxx+1)];
            %             inds=[inds; ind];
            %             zs=[zs; z];
            %         end
            %
            %         if (height>2)
            %             % find local minima
            %
            %             line=double(line);
            %
            %             minima=imregionalmax(imhmin(line,median(double(maxint))/3,[0 1 0; 0 1 0; 0 1 0]),[0 1 0; 0 1 0; 0 1 0]);
            %
            %             % find local the closest local minima
            %             indz=zeros(1,size(voxel,2));
            %             for j=0:height
            %                 [maxv,ind]=max([minima(height-j+1,:).*line(height-j+1,:);minima(height+j+1,:).*line(height+j+1,:)]);
            %                 ind=(maxv>0).*floor(height+j*2*(ind-1.5)+1);
            %                 indz=indz+(indz==0).*ind;
            %             end
            %         else
            %             %global minima
            %             if height==0
            %                 indz=ones(size(line,2),1);
            %             else
            %                 [val,indz]=max(line,[],1);
            %             end
            % %         end
            %         inds=inds';
            %         ind=inds((indz-1)*size(line,2)+(1:size(line,2)));
            %         indx=-(ind-neighborhood-1).*dy;
            %         indy=(ind-neighborhood-1).*dx;
            %         indz=indz-height-1;
            %         voxel(2,:)=round(min(max(voxel(2,:)+indx,1),maxx));
            %         voxel(1,:)=round(min(max(voxel(1,:)+indy,1),maxy));
            %         voxel(3,:)=round(min(max(voxel(3,:)+indz,1),maxz));
            %         voxelmax=voxel;
            %
            %     if (sizeofaxon>7)
            %         % smoothing again to futher suppress jumps
            %         voxel(1,:)=medfilt1(voxel(1,:),14);
            %         voxel(2,:)=medfilt1(voxel(2,:),14);
            %         voxel(3,:)=medfilt1(voxel(3,:),14);
            %         voxel(1,:)=smooth(voxel(1,:),7)';
            %         voxel(2,:)=smooth(voxel(2,:),7)';
            %         voxel(3,:)=smooth(voxel(3,:),7)';
            %     end
            %         voxel(2,:)=min(max(voxel(2,:),1),maxx);
            %         voxel(1,:)=min(max(voxel(1,:),1),maxy);
            %         voxel(3,:)=min(max(voxel(3,:),1),maxz);
            %         % find maxint as center of line
            %         x=round(min(max(voxel(2,:)-1,0),maxx-1));
            %         y=round(min(max(voxel(1,:)-1,0),maxy-1));
            %         z=round(min(max(voxel(3,:)-1,0),maxz-1));
            %
            %         maxint=imageArray(y+x*maxy+z*maxy*maxx+1);
            %
            %         RMS=sqrt(mean((smoothmaxint-smooth(double(maxint),7)).^2));
            %         disp(['RMS error for step ' num2str(i) ': ' num2str(RMS)]);
            %         smoothmaxint=smooth(double(maxint),7);
        end
    end
    
    %calculate the normal direction of the axon based on 7 pixels
    neighborhood=14;
    if (sizeofaxon>neighborhood)
        dy=([voxel(1,1)*ones(1,neighborhood) voxel(1,1:sizeofaxon-neighborhood)]-[voxel(1,neighborhood+1:sizeofaxon) voxel(1,sizeofaxon)*ones(1,neighborhood)]);
        dx=([voxel(2,1)*ones(1,neighborhood) voxel(2,1:sizeofaxon-neighborhood)]-[voxel(2,neighborhood+1:sizeofaxon) voxel(2,sizeofaxon)*ones(1,neighborhood)]);
    else
        dx=ones(1,sizeofaxon).*(voxel(1,1:sizeofaxon)-voxel(1,1));
        dy=ones(1,sizeofaxon).*(voxel(2,1:sizeofaxon)-voxel(2,1));
    end
    dr=sqrt(dx.*dx+dy.*dy);
    %change length of 0 to 1 to avoid division by zero
    dr(find(dr==0))=1;
    %normalize to unit vector length
    dx=dx./dr;
    dy=dy./dr;
end