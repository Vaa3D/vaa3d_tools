function [Gmag,Fphi,Ftheta] = gradient_3D(gt)

% calculate the gradient from the 3D objects 

% first calculate the direction on 3D 

[Fx,Fy,Fz] = gradient(gt);

Gmag = sqrt(Fx .^ 2 + Fy .^ 2 + Fz .^ 2);

Gind = find(Gmag > 0);

Gmag1 = Gmag(Gind);

Fx1 = Fx(Gind) ./ Gmag1;

Fy1 = Fy(Gind) ./ Gmag1;

Fz1 = Fz(Gind) ./ Gmag1;

% calculate the unit vectors for the actual gradient vectors 

Fphi1 = atan2(Fx1,Fy1) / pi * 180;

Ftheta1 = acos(Fz1) / pi * 180;

clear Fx Fy Fz;

Fphi = zeros(size(gt));

Ftheta = zeros(size(gt));

Fphi(Gind) = Fphi1;

Ftheta(Gind) = Ftheta1;