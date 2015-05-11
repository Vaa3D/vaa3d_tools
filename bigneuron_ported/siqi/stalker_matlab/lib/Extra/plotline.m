function y = plotline(x,d,varargin)
% PLOTLINE   Plots a vertical or horizontal line
%
%    PLOTLINE(x,dir) plots a horizontal (dir='h') or vertical (dir='v')
%    dotted line at position x on the current figure.
%
%    PLOTLINE(x,dir,S) uses plots the line using the specified color and 
%    linestyle. S is a string containing one element from any or all the
%    columns bellow (just like in the plot command).
%
%     Colors:              Styles:
%    y     yellow        -     solid
%    m     magenta       :     dotted
%    c     cyan          -.    dashdot 
%    r     red           --    dashed   
%    g     green         
%    b     blue          
%    w     white         
%    k     black         
%                        
%    PLOTLINE(...,'label') plots the value label x next to the line.
%

[S,lab] = parse_inputs(varargin{:});

h = ishold;
ax = axis;

hold on;

if strcmp(d,'h')
   for i = 1 : length(x)
      plot([ax(1) ax(2)], [x(i) x(i)], S)
      if lab
         text( (ax(1)+ax(2))/2, x(i), num2str(x(i)))
      end
   end
elseif strcmp(d,'v')
   for i = 1 : length(x)
      plot([x(i) x(i)], [ax(3) ax(4)], S)
      if lab
         text( x(i), (ax(3)+ax(4))/2, num2str(x(i)))
      end
   end
else
   error('Unknow direction.')
end

if h
   hold on;
else
   hold off
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [S,lab] = parse_inputs(varargin)
S = [];
lab = -1;

for i = 1 : nargin
   if strcmp('label',varargin{i}) & lab==-1
      lab = 1;
   elseif isempty(S)
      S = varargin{i};
   else
      error('Too many parameters.')
   end
end

if lab==-1
   lab=0;
end
if isempty(S)
   S='k:';
end