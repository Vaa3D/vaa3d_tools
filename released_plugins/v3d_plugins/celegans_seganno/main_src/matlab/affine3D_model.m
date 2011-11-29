%compute 3D affine transform matrix
%X2=T*X1
%X1,X2=[3,N]
%by Lei Qu
function T=affine3D_model(X1,X2)

% normalize points
[X1,T1]=normalize_points(X1);
[X2,T2]=normalize_points(X2);

% fill matrix A
%
%  | h1, h2, h3, h4 |    |x1| |x2|
%  | h5, h6, h7, h8 | *  |y1|=|y2| <=>
%  | h9, h10,h11,h12|    |z1| |z2|
%  | 0 ,  0,  0,  1 |    |1 | |1 |
%
%  |x1, y1, z1, 1,  0,  0,  0,  0,  0,  0,  0,  0, -x2 |   
%  | 0,  0,  0, 0, x1, y1, z1,  1,  0,  0,  0,  0, -y2 | * |h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11,h12,h13|=0
%  | 0,  0,  0, 0, 0, 0, 0, 0,  0, x1, y1, z1,  1, -z2 |
%
N=size(X1,2);
A=zeros(3*N,13);
zero=[0; 0; 0; 0;];

row=1;
for i=1:N        
    a=[X1(:,i)',1];
    X2_x=X2(1,i);
    X2_y=X2(2,i);
    X2_z=X2(3,i);

    A(row,:)   = [a     zero' zero' -X2_x];
    A(row+1,:) = [zero'   a   zero' -X2_y];
    A(row+2,:) = [zero' zero'  a    -X2_z];

    row=row+3;
end


% compute T
[U S V]=svd(A);
h=V(:, 13);

if S(12,12)==0 % too singular
    T=[];
    return;
end

% de-homo
h=h/h(13); 

% reshape the output
T=[h(1:4)'; h(5:8)'; h(9:12)'; 0 0 0 1];

% and denormalize
% X2=T2*A2 |
% X1=T1*A1  > T2*A2=T*T1*A1 ==> A2=(inv(T2)*T*T1)*A1 ==> T=inv(T2)*T*T1
% X2=T *T1 |
T=T2\T*T1;

return

