function [voxel, voxelmax, maxint, dx, dy] = shiftBranchMax(voxel,imageArray,restricted)

%fix the neurite so it coincides with the maximal intensity in the perpendicular plane

maxx=size(imageArray,2);
maxy=size(imageArray,1);
maxz=size(imageArray,3);

arraysize=size(voxel,2);

%padding the array

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


for i=1:1
    
    %fix the neurite so it coincides with the maximal intensity in the perpendicular plane
    
    %calculate the instantaneous slope in X-Y plane use neighborhood of 5 pixels to each side
    sizeofaxon=size(voxel,2);
    
    %calculate the normal direction of the axon based on 3 pixels on
    %each side
    oldvoxel=voxel;
    if (sizeofaxon>7)
        %        smoothing again to futher suppress jumps
        %padding
        
        voxel(1,:)=medfilt1pad(voxel(1,:),14);
        voxel(2,:)=medfilt1pad(voxel(2,:),14);
        voxel(3,:)=medfilt1pad(voxel(3,:),14);
        voxel(1,:)=smooth(voxel(1,:),7)';
        voxel(2,:)=smooth(voxel(2,:),7)';
        voxel(3,:)=smooth(voxel(3,:),7)';
    end
    neighborhood=14;
    if (sizeofaxon>neighborhood)
        dy=([voxel(1,1)*ones(1,neighborhood) voxel(1,1:sizeofaxon-neighborhood)]-[voxel(1,neighborhood+1:sizeofaxon) voxel(1,sizeofaxon)*ones(1,neighborhood)]);
        dx=([voxel(2,1)*ones(1,neighborhood) voxel(2,1:sizeofaxon-neighborhood)]-[voxel(2,neighborhood+1:sizeofaxon) voxel(2,sizeofaxon)*ones(1,neighborhood)]);
    else
        dx=ones(1,sizeofaxon)*(voxel(1,1:sizeofaxon)-voxel(1,1));
        dy=ones(1,sizeofaxon)*(voxel(2,1:sizeofaxon)-voxel(2,1));
    end
    dr=sqrt(dx.*dx+dy.*dy);
    %    change length of 0 to 1 to avoid division by zero
    dr(find(dr==0))=1;
    %    normalize to unit vector length
    dx=dx./dr;
    dy=dy./dr;
    voxel=oldvoxel;
    
    
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
        indx=-(ind-neighborhood).*dy;
        indy=(ind-neighborhood).*dx;
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
        neighborhood=7;
        height=5;
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
        indx=-(ind-neighborhood).*dy;
        indy=(ind-neighborhood).*dx;
        indz=indz-height-1;
        voxel(2,:)=round(min(max(voxel(2,:)+indx,1),maxx));
        voxel(1,:)=round(min(max(voxel(1,:)+indy,1),maxy));
        voxel(3,:)=round(min(max(voxel(3,:)+indz,1),maxz));
        voxelmax=voxel;
        
        if (sizeofaxon>7)
            % smoothing again to futher suppress jumps
            voxel(1,:)=medfilt1pad(voxel(1,:),14);
            voxel(2,:)=medfilt1pad(voxel(2,:),14);
            voxel(3,:)=medfilt1pad(voxel(3,:),14);
            voxel(1,:)=smooth(voxel(1,:),7)';
            voxel(2,:)=smooth(voxel(2,:),7)';
            voxel(3,:)=smooth(voxel(3,:),7)';
        end
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
        
        
        if (sizeofaxon>7)
            voxel=[voxel(:,1)*ones(1,30) voxel voxel(:,end)*ones(1,30)];
            % smoothing again to futher suppress jumps
            voxel(1,:)=medfilt1pad(voxel(1,:),20);
            voxel(2,:)=medfilt1pad(voxel(2,:),20);
            voxel(3,:)=medfilt1pad(voxel(3,:),20);
            voxel=voxel(:,31:end-30);
        end
        
        neighborhood=14;
        if (sizeofaxon>neighborhood)
            dy=([voxel(1,1)*ones(1,neighborhood) voxel(1,1:sizeofaxon-neighborhood)]-[voxel(1,neighborhood+1:sizeofaxon) voxel(1,sizeofaxon)*ones(1,neighborhood)]);
            dx=([voxel(2,1)*ones(1,neighborhood) voxel(2,1:sizeofaxon-neighborhood)]-[voxel(2,neighborhood+1:sizeofaxon) voxel(2,sizeofaxon)*ones(1,neighborhood)]);
        else
            dx=ones(1,sizeofaxon)*(voxel(1,1:sizeofaxon)-voxel(1,1));
            dy=ones(1,sizeofaxon)*(voxel(2,1:sizeofaxon)-voxel(2,1));
        end
        dr=sqrt(dx.*dx+dy.*dy);
        %change length of 0 to 1 to avoid division by zero
        dr(find(dr==0))=1;
        %normalize to unit vector length
        dx=dx./dr;
        dy=dy./dr;
        
        plane=[];
        line=[];
        neighborhood=5;
        height=5;
        inds=[];
        zs=[];
        for j=-height:height   %five focal section up and five plane down
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
            % find local minima
            
            line=double(line);
            
            minima=imregionalmax(imhmin(line,median(double(maxint))/3,[0 1 0; 0 1 0; 0 1 0]),[0 1 0; 0 1 0; 0 1 0]);
            
            % find local the closest local minima
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
        indx=-(ind-neighborhood).*dy;
        indy=(ind-neighborhood).*dx;
        indz=indz-height-1;
        voxel(2,:)=round(min(max(voxel(2,:)+indx,1),maxx));
        voxel(1,:)=round(min(max(voxel(1,:)+indy,1),maxy));
        voxel(3,:)=round(min(max(voxel(3,:)+indz,1),maxz));
        voxelmax=voxel;
        
        %     if (sizeofaxon>7)
        %         % smoothing again to futher suppress jumps
        %         voxel(1,:)=medfilt1(voxel(1,:),14);
        %         voxel(2,:)=medfilt1(voxel(2,:),14);
        %         voxel(3,:)=medfilt1(voxel(3,:),14);
        %         voxel(1,:)=smooth(voxel(1,:),7)';
        %         voxel(2,:)=smooth(voxel(2,:),7)';
        %         voxel(3,:)=smooth(voxel(3,:),7)';
        %     end
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
    end    
    
    %calculate the normal direction of the axon based on 7 pixels
    neighborhood=14;
    if (sizeofaxon>neighborhood)
        dy=([voxel(1,1)*ones(1,neighborhood) voxel(1,1:sizeofaxon-neighborhood)]-[voxel(1,neighborhood+1:sizeofaxon) voxel(1,sizeofaxon)*ones(1,neighborhood)]);
        dx=([voxel(2,1)*ones(1,neighborhood) voxel(2,1:sizeofaxon-neighborhood)]-[voxel(2,neighborhood+1:sizeofaxon) voxel(2,sizeofaxon)*ones(1,neighborhood)]);
    else
        dx=ones(1,sizeofaxon)*(voxel(1,1:sizeofaxon)-voxel(1,1));
        dy=ones(1,sizeofaxon)*(voxel(2,1:sizeofaxon)-voxel(2,1));
    end
    dr=sqrt(dx.*dx+dy.*dy);
    %change length of 0 to 1 to avoid division by zero
    dr(find(dr==0))=1;
    %normalize to unit vector length
    dx=dx./dr;
    dy=dy./dr;
end