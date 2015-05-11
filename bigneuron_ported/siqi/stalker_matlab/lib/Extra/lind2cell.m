function Y = lind2cell(I, F)
%
% LIND2CELL   Converts limiting indexes to cell.
%
%     Y = LIND2CELL([I1 I2 ... In], [F1 F2 ... Fn]) creates the cell array
%     Y where each cell contains the sequence of integers betweeen the limits
%     'Ii' and 'Fi'.
%
%     Exemple: Y = LIN2CELL([2 4 0],[5 9 1]) creates the 3 cells array Y
%     Y = [1x4 double] [1x6 double] [1x2 double]
%     Y{1} = [2 3 4 5]; Y{2} = [4 5 6 7 8 9]; Y{3} = [0 1]
%
%     This function is extremally usefull to operate only in part of a 
%     matrix (M) if the initial and final limits of the affected area
%     are contained in two vectors [Ii] and [Fi].
%
%     Exemple:      X = [ 1 2 3 4 5 6 7 8
%                         9 0 1 2 3 4 5 6
%                         7 8 9 0 1 2 3 4
%                         5 6 7 8 9 0 1 2 ]
%
%     Y = lind2cell([2 3],[4 6]) = [2 3 4]; [3 4 5 6]
%     X(Y{:}) = X(2:4,3:6) = [1 2 3 4; 9 0 1 2; 7 8 9 0]
%

Y = cell(1,length(I));

for dim = 1 : length(I)
   Y{dim} = I(dim):F(dim);
end
