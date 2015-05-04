% --------------------------------------------------------------------
function obj = FindSpines_Callback(obj)
if isfield(obj.data,'burnArray') & isfield(obj.data,'dendrites')

for i=1:size(obj.data.dendrites,2)
       % the C program uses indices starting from 0. This needs to be converted
        % to MATLAB convention which starts from 1
        dendrites(i).voxel=uint16(double(obj.data.dendrites(i).voxel)-[ones(1,size(obj.data.dendrites(i).voxel,2));ones(1,size(obj.data.dendrites(i).voxel,2));ones(1,size(obj.data.dendrites(i).voxel,2));zeros(1,size(obj.data.dendrites(i).voxel,2))]);   
   end
obj.data.spines=spinedetect(obj.data.burnArray,dendrites,struct('um_x',1.0,'um_y',1.0,'um_z', 1.0, 'domerge', 1, 'disc_tol', obj.parameters.threedma.maxdisconnspinedis, 'minlen_tol',1.0,'brk_den',0));
numspines=size(obj.data.spines,2);
%Calculate distances
for j=1:numspines
      obj.data.spines(j).voxels=size(obj.data.spines(j).voxel,2);
      %Calculate distances
      dendrite=double(obj.data.dendrites(obj.data.spines(j).den_ind).voxel);
      spine=double(obj.data.spines(j).voxel);
      dis=(dendrite(1,:)-spine(1,2)).*(dendrite(1,:)-spine(1,2))+(dendrite(2,:)-spine(2,2)).*(dendrite(2,:)-spine(2,2))+(dendrite(3,:)-spine(3,2)).*(dendrite(3,:)-spine(3,2));
      [a,ind]=min(dis);
      obj.data.spines(j).voxel(:,1)=uint16(dendrite(1:3,ind));
      obj.data.spines(j).dendis=ind;
      obj.data.spines(j).edittype=0;    % initially no editing
      if (size(obj.data.spines(j).voxel,2)<obj.data.parameters.minspinevoxel) %suppress disconnected spines that are too small
          obj.data.spines(j).edittype=1;
      end;
      if (obj.data.spines(j).len<obj.data.parameters.minspinelength)   %supress spines that are too close or too small
          obj.data.spines(j).edittype=1;
      end;
end
spineind=[];
for i=1:size(obj.data.dendrites,2)
    loc=find([obj.data.spines.den_ind]==i);
    [A,B]=sort([obj.data.spines(loc).dendis]);
    spineind=[spineind loc(B)];
end
obj.data.spines=obj.data.spines(spineind);
obj.data.newspinenum=1;
for j=1:numspines
    obj.data.spines(j).color=hsv2rgb([(j-5*floor(j/5))*0.2+0.2*j/numspines,1,1]);
    obj.data.spines(j).label=j;
end
obj.data.slicespines={};
for i=1:size(obj.data.burnArray,3)
    ind=1;
    for j=1:size(obj.data.spines,2)
        pos=find(obj.data.spines(j).voxel(3,:)==i-1);
        if (pos)
            obj.data.slicespines(i).spines(ind)=struct('voxel',[obj.data.spines(j).voxel(1,pos);obj.data.spines(j).voxel(2,pos)],'index',j);
            ind=ind+1;    
        end
    end
end;
for j=1:size(obj.data.spines,2)
            den=obj.data.dendrites(obj.data.spines(j).den_ind).voxel;
            dend=den(:,max(1,obj.data.spines(j).dendis-5):min(size(den,2),obj.data.spines(j).dendis+5));
            coeff=(double([ones(1,size(dend,2)); dend(1,:)]))'\(double(dend(2,:)))';
            obj.data.spines(j).side=sign(double(obj.data.spines(j).voxel(2,2))-coeff(2)*double(obj.data.spines(j).voxel(1,2))-coeff(1));    
end
square=ones(1,16);
obj.data.bigspinesr=find(([obj.data.spines.voxels]>100)&([obj.data.spines.side]>0));
obj.data.bigspinesl=find(([obj.data.spines.voxels]>100)&([obj.data.spines.side]<0));
obj.data.spinogramr=zeros(1,size(obj.data.dendrites(1).voxel,2));
obj.data.spinograml=obj.data.spinogramr;
obj.data.spinograml([obj.data.spines(obj.data.bigspinesl).dendis])=1;%[obj.data.spines(obj.data.bigspines).voxels]./100;
obj.data.spinogramr([obj.data.spines(obj.data.bigspinesr).dendis])=1;
%  obj.data.gaussianspinogramr=conv(obj.data.spinogramr,gaussian);
%  obj.data.gaussianspinograml=conv(obj.data.spinograml,gaussian);
obj.data.squarespinogramr=conv(obj.data.spinogramr,square);
obj.data.squarespinograml=conv(obj.data.spinograml,square);
end;

% 
% 
% % --------------------------------------------------------------------
% function varargout = FindAxonStubs_Callback(h, eventdata, handles, varargin)
% data=getappdata(gcbf,'data');
% if isfield(data,'burnArray') & isfield(data,'dendrites')
%     for i=1:size(obj.data.dendrites,2)
%         obj.data.dendrites(i).voxel=obj.data.dendrites(i).voxel(1:4,:);
%     end;
% %    obj.data.dendrites=obj.data.dendrites(1);
% obj.data.spines=spinedetect(obj.data.burnArray,obj.data.dendrites,struct('um_x',1.0,'um_y',1.0,'um_z', 1.0, 'domerge', 1, 'disc_tol', obj.data.parameters.maxdisconnspinedis, 'minlen_tol',1.0,'brk_den',0));
% numspines=size(obj.data.spines,2);
% %Calculate distances
% for j=1:numspines
%       obj.data.spines(j).voxels=size(obj.data.spines(j).voxel,2);
%       %Calculate distances
%       dendrite=double(obj.data.dendrites(obj.data.spines(j).den_ind).voxel);
%       spine=double(obj.data.spines(j).voxel);
%       dis=(dendrite(1,:)-spine(1,2)).*(dendrite(1,:)-spine(1,2))+(dendrite(2,:)-spine(2,2)).*(dendrite(2,:)-spine(2,2))+(dendrite(3,:)-spine(3,2)).*(dendrite(3,:)-spine(3,2));
%       [a,ind]=min(dis);
%       obj.data.spines(j).voxel(:,1)=uint16(dendrite(1:3,ind));
%       obj.data.spines(j).dendis=ind;
%       obj.data.spines(j).edittype=0;    % initially no editing
%       if (size(obj.data.spines(j).voxel,2)<obj.data.parameters.minspinevoxel) %suppress disconnected spines that are too small
%           obj.data.spines(j).edittype=1;
%       end;
%       if (obj.data.spines(j).len<obj.data.parameters.minspinelength)   %supress spines that are too close or too small
%           obj.data.spines(j).edittype=1;
%       end;
% end
% spineind=[];
% for i=1:size(obj.data.dendrites,2)
%     loc=find([obj.data.spines.den_ind]==i);
%     [A,B]=sort([obj.data.spines(loc).dendis]);
%     spineind=[spineind loc(B)];
% end
% obj.data.spines=obj.data.spines(spineind);
% obj.data.newspinenum=1;
% for j=1:numspines
%     obj.data.spines(j).color=hsv2rgb([(j-5*floor(j/5))*0.2+0.2*j/numspines,1,1]);
%     obj.data.spines(j).label=j;
% end
% obj.data.slicespines={};
% for i=1:size(obj.data.burnArray,3)
%     ind=1;
%     for j=1:size(obj.data.spines,2)
%         pos=find(obj.data.spines(j).voxel(3,:)==i-1);
%         if (pos)
%             obj.data.slicespines(i).spines(ind)=struct('voxel',[obj.data.spines(j).voxel(1,pos);obj.data.spines(j).voxel(2,pos)],'index',j);
%             ind=ind+1;    
%         end
%     end
% end;
% for j=1:size(obj.data.spines,2)
%             den=obj.data.dendrites(obj.data.spines(j).den_ind).voxel;
%             dend=den(:,max(1,obj.data.spines(j).dendis-5):min(size(den,2),obj.data.spines(j).dendis+5));
%             coeff=(double([ones(1,size(dend,2)); dend(1,:)]))'\(double(dend(2,:)))';
%             obj.data.spines(j).side=sign(double(obj.data.spines(j).voxel(2,2))-coeff(2)*double(obj.data.spines(j).voxel(1,2))-coeff(1));    
% end
% square=ones(1,16);
% obj.data.bigspinesr=find(([obj.data.spines.voxels]>100)&([obj.data.spines.side]>0));
% obj.data.bigspinesl=find(([obj.data.spines.voxels]>100)&([obj.data.spines.side]<0));
% obj.data.spinogramr=zeros(1,size(obj.data.dendrites(1).voxel,2));
% obj.data.spinograml=obj.data.spinogramr;
% obj.data.spinograml([obj.data.spines(obj.data.bigspinesl).dendis])=1;%[obj.data.spines(obj.data.bigspines).voxels]./100;
% obj.data.spinogramr([obj.data.spines(obj.data.bigspinesr).dendis])=1;
% %  obj.data.gaussianspinogramr=conv(obj.data.spinogramr,gaussian);
% %  obj.data.gaussianspinograml=conv(obj.data.spinograml,gaussian);
% obj.data.squarespinogramr=conv(obj.data.spinogramr,square);
% obj.data.squarespinograml=conv(obj.data.spinograml,square);
% end;
% setappdata(gcbf,'data',data);
