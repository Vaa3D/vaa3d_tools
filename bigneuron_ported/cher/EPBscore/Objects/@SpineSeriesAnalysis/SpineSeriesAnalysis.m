function s = SpineSeriesAnalysis(a)
% SpineAnalysisImage SpineAnalysisImage class constructor.
%   s = SpineAnalysisImage(a) creates a spineanalysisimage class from the
%   specified ini file

if nargin == 0
   s.ID = -1;
   s = class(s,'SpineSeriesAnalysis');
elseif isa(a,'SpineSeriesAnalysis')
   s = a;
else
   global self;
   global ogh;
   self.ID=-1;
   initGUIs(a);
   % initialize the graphical handles correctly
   for field=fieldnames(ogh)'
       self.gh.(field{1})=ogh.(field{1});
   end
   self = class(self,'SpineSeriesAnalysis');
   s = self;
end