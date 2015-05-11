function Y = limindex(X, I, F)
%
% LIMINDEX   Extracts matrix elements by limiting indexes.
%
%     Y = LIMINDEX(X,[I1 I2 ... In], [F1 F2 ... Fn]) is the same as 
%     Y = X(I1:F1, I2:F2, ..., In:Fn)
%

ind = cell(1,length(I));

for dim = 1 : length(I)
   ind{dim} = I(dim):F(dim);
end

Y = X(ind{:});
