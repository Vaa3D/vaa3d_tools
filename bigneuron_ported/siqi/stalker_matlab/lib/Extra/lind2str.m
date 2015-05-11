function Y = lind2str(I, F)
%
% LIND2STR   Converts limiting indexes to string.
%
%     S = LIND2STR([I1 I2 ... In], [F1 F2 ... Fn]) creates the string 'S'
%     so that S = 'I1:F1, I2:F2, ..., In:Fn'.
%
%     This function is very similar to LIND2CELL. The main diference is
%     the fact that LIND2STR does not evaluates the resulting string and
%     therefore, in case there are many elements between Ii and Fi, LIND2CELL
%     will generate a very big output matrix. By the other side, LIND2STR will
%     allways require the using of the eval command to operate.
%
%     Exemple:      X = [ 1 2 3 4 5 6 7 8
%                         9 0 1 2 3 4 5 6
%                         7 8 9 0 1 2 3 4
%                         5 6 7 8 9 0 1 2 ]
%
%     Y = lind2str([2 3],[4 6]) = '2:4,3:6'
%     eval(['X(',Y,')']) = X(2:4,3:6) = [1 2 3 4; 9 0 1 2; 7 8 9 0]
%

if size(I,2)==1
   I=I';
end
if size(F,2)==1
   F=F';
end

Y = strcat(int2str(I'),':',int2str(F'),',')';
Y = Y(:)';
Y(end)=[];

