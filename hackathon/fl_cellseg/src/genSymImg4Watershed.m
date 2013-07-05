a = zeros(100,100,100);

c1 = [40,40,40];
c2 = [55,55,40];
rad = 20;

for i=1:100
    for j=1:100
        for k=1:100
            if sum(([i,j,k]-c1).^2)<rad*rad
                a(i,j,k) =1;
            end;
            if sum(([i,j,k]-c2).^2)<rad*rad
                a(i,j,k) =1;
            end;
        end;
    end;
end;

b = bwdist(max(a(:))-a);

c = b;
c(b==0)=0;
c(b>0) = max(b(b>0))-b(b>0);

% d = watershed_old(c);

d(:,:,:,1) = c;
d(:,:,:,2) = c;
d(:,:,:,3) = c;

saveStack2File_c(uint8(d),'testWatershedDist.raw');

