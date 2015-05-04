function v=moments_3D(img);
% v - direction of an axis having largest 2nd moment of the image img
%VZ   05/2006


[bx,by,bz] = size(img);
r=double((bx-1)/2);

[x,y,z]=meshgrid(1:bx,1:by,1:bz);

sph=(x-r-1).^2+(y-r-1).^2+(z-r-1).^2<=r^2;

img=double(img.*sph);

area = sum(sum(sum(img)));
mx = sum(sum(sum(img.*x)));
my = sum(sum(sum(img.*y)));
mz = sum(sum(sum(img.*z)));

mxx = sum(sum(sum(img.*x.*x)));
myy = sum(sum(sum(img.*y.*y)));
mzz = sum(sum(sum(img.*z.*z)));
mxy = sum(sum(sum(img.*x.*y)));
mxz = sum(sum(sum(img.*x.*z)));
myz = sum(sum(sum(img.*y.*z)));

if area>0
   cxx = mxx - mx^2/area;
   cyy = myy - my^2/area;
   czz = mzz - mz^2/area;
   cxy = mxy - mx*my/area;
   cxz = mxz - mx*mz/area;
   cyz = myz - my*mz/area;
   M = [ cxx  cxy cxz; cxy  cyy cyz; cxz  cyz czz];

   [eig_vec, eig_val ] = eig(M);
   e =  eig(M);
   a = max(e);
   b = min(e);
   v=eig_vec(:,3);%*area;
   if a>0
       v=double(v);%*(a-b)/(a+b);
   else
       v=double([0;0;0]);
   end
else
   v=double([0;0;0]);
end