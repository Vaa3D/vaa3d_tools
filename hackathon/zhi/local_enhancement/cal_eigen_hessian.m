function cal_eigen_hessian(infile)
%function cal_eigen_hessian(infile)
% by Hanchuan Peng
% 2013-08-27

a=loadRaw2Stack(infile);
a=double(a);

%%

bxx=mydiff3d(mydiff3d(a,1), 1);
bxy=mydiff3d(mydiff3d(a,1), 2);
bxz=mydiff3d(mydiff3d(a,1), 3);

byx=mydiff3d(mydiff3d(a,2), 1);
byy=mydiff3d(mydiff3d(a,2), 2);
byz=mydiff3d(mydiff3d(a,2), 3);

bzx=mydiff3d(mydiff3d(a,3), 1);
bzy=mydiff3d(mydiff3d(a,3), 2);
bzz=mydiff3d(mydiff3d(a,3), 3);


%%

outimg = a;
outimg1 = a;

% outimg2 = a;

fprintf('\n');
n=0;
for k=1:size(a,3),
    fprintf('[%d] ', k);
    if mod(k,10)==9, fprintf('\n'); end;
    for j=1:size(a,2), 
        for i=1:size(a,1), 
            n=n+1; 
            
            t = [bxx(n) bxy(n) bxz(n); ...
                 byx(n) byy(n) byz(n); ...
                 bzx(n) bzy(n) bzz(n)];
             
            [vv, dd] = eig(t); 
            
            D = [dd(1,1) dd(2,2) dd(3,3)];
            DA = abs(D);
            [Y, I] = sort(DA, 'descend');
            D = D(I);
            
            if (D(1)<0 & D(2)<0),
                outimg(n) = Y(2).*(Y(2)-Y(3))./Y(1);
                outimg1(n) = 255;
            else,
                outimg(n) = 0;
                outimg1(n) = 0;
            end;
            
%             if (Y(1)~=0),
%                 outimg2(n) = Y(2).*(Y(2)-Y(3))./Y(1);
%             else,
%                 outimg2(n) = 0;
%             end;
        end; 
    end; 
end;
fprintf('\n');

%%

saveStack2File_c(single(outimg), [infile '_enhanced_indicator.v3draw']);
saveStack2File_c(single(outimg1), [infile '_enhanced_valuereserved.v3draw']);

% saveStack2File_c(single(outimg2), [infile '_enhanced_2.v3draw']);


%% 
function d = mydiff3d(a, dim)
d = diff(a, 1, dim);
if (dim==1),
    z = repmat(0, [1, size(a,2), size(a,3)]);
    d = cat(1, z, d);
elseif (dim==2),
    z = repmat(0, [size(a,1), 1, size(a,3)]);
    d = cat(2, z, d);
elseif (dim==3),
    z = repmat(0, [size(a,1), size(a,2), 1]);
    d = cat(3, z, d);
end;
