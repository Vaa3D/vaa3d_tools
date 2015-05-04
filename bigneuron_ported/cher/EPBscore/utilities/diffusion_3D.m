function Jd=diffusion_3D(J,N,K,u,v,w,dt)

[Nx,Ny,Nz]=size(J);

for i=1:N;

       In=circshift(J,[ 1   0   0])-J;
       Is=circshift(J,[ -1   0   0])-J;
       Ie=circshift(J,[ 0   1   0])-J;
   Iw=circshift(J,[ 0   -1   0])-J;
       Iu=circshift(J,[ 0   0   1])-J;
   Id=circshift(J,[ 0   0   -1])-J;

   In(1,:,:)=0; Is(Nx,:,:)=0;
   Ie(:,1,:)=0; Iw(:,Ny,:)=0;
   Iu(:,:,1)=0; Id(:,:,Nz)=0;
   u=u+0.01; v=v+0.01; w=w+0.01;

   Cn=1./(1+(K./u).^2);
       Cs=1./(1+(K./u).^2);
       Ce=1./(1+(K./v).^2);
   Cw=1./(1+(K./v).^2);
       Cu=1./(1+(K./w).^2);
   Cd=1./(1+(K./w).^2);

  J=J+dt*(Cn.*In + Cs.*Is + Ce.*Ie + Cw.*Iw + Cu.*Iu + Cd.*Id);
end;

Jd = J;
