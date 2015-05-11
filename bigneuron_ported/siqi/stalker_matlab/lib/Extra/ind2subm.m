function I = ind2subm(SIZ,IND)
% IND2SUBM   Multiple subscripts from linear index.
%
%   IND2SUBM is used to determine the equivalent subscript values
%   corresponding to a given single index into an array.
%
%   I = IND2SUBM(SIZ,IND) returns the matrix I containing in each row
%   all the subscripts of the corresponding index IND.
%
%   The only difference between IND2SUBM and IND2SUB is that, in IND2SUBM,
%   only one output is produced, independently of the number of dimensions
%   involved. This is useful when writting functions that must support 
%   multidimensional matrix.
%

if size(IND,1)==1 & size(IND,2)>1
   IND = IND';
end
   
I = zeros( length(IND),length(SIZ) );
IND = IND - 1;

for dim = length(SIZ) : -1 : 2
   p = prod(SIZ(1:dim-1));
   I(:,dim) = floor(IND/p);
   IND = IND - I(:,dim)*p;
end
I(:,1) = IND;

I = I+1;
   

