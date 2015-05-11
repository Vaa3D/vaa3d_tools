function y = plotcolor(x)
% PLOTCOLOR   Returns plot color strings
%
%    y = PLOTCOLOR(x) returns sequentially the color strings 
%    'b','r','g','c','m','k','y' for integer values of x.
%
%    This function is intended to be used within for loops where 
%    there is the need of plotting with different colors sequentially.
%    
%    If x is a vector, y will be a length(x) x 3 matrix where each row
%    corresponds to one color string (the string is length 3 because it
%    includes two quotes and the letter).
%    

x = mod(x-1,7)+1;
c = ['b','r','g','c','m','k','y',];

for i = 1 : length(x)
   y(i,:) = [char(39), c(x(i)), char(39)];
end
