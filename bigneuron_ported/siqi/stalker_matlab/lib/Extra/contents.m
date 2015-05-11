% General Extra Toolbox
% Version 0.2    Mar-2002
% Frederico D'Almeida - DEE/Federal University of Bahia - Brazil
% Comments, improvements, bugs, etc. -> f_dalmeida@hotmail.com
%
% Generic functions.
%   iscol       - True for column vectors.
%   iseven      - True for even numbers.
%   isint       - True for integer numbers.
%   isodd       - True for odd numbers.
%   isrow       - True for row vectors.
%   quote       - Returns a quoted string.
%
% Matrix Functions.
%   count       - Counts the number of different elements.
%   gmax        - Global maximum element.
%   gmin        - Global minimum element.
%   normn       - Matrix norm.
%   mmax        - Multi matrix maximum.
%   mmin        - Multi matrix minimum.
%   extreme     - Sets\gets matrix extremum values.
%   thomas      - Fast tridiagonal linear system solver algorithm.
%   toggle      - Matrix element classification.
%   vanherk     - Fast 1D max/min filter algorithm.
%
% Matrix Manipulation
%   grow        - Expands a matrix by border repetition.
%   pad         - Pads matrix with specified element.
%   resize      - Resizes an matrix.
%   roll        - Rolls or rotates matrix elements.
%   scale       - Scales matrix elements to a new range.
%   shift       - Shifts matrix elements.
%   subdim      - Extracts matrix elements by dimension.
%
% Matrix index function
%   ind2subm    - Multiple subscripts from linear index.
%   lind2cell   - Creates matrix indexes from indexes limits (cell format).
%   lind2str    - Creates matrix indexes from indexes limits (string format).
%   limindex    - Extracts matrix elements by its indexes.
%
% Image/Graphic functions
%   imagem     - Displays image and markers.
%   invert     - Inverts an image.
%   maxfilt2   - Two-dimensional maximum filter.
%   minfilt2   - Two-dimensional minimum filter.
%   noise      - Adds noise to an image/matrix.
%   plotcolor  - Returns plot color strings.
%   plotline   - Plots vertical/horizontal lines.
%   posterize  - Reduces number of colors in a image.
%   toggle     - Matrix elements classification.
%
% Color Maps.
%   cmap        - Creates color maps.
%   colormapc   - Creates circular color maps.
%   colormaps   - Creates simmetrical color maps.
%
% Search Path utils.
%   addpaths    - Adds multiple directories to search path.
%   rmpathds    - Remove multiple directories from search path.
%   subdirs     - Returns the subdirs of a directory.
%
% Fourier Transform.
%   fft2fix     - Fixes FFT2 graphic.
%   nfft        - Discrete Fourier transform.
%   nfft2       - 2D discrete Fourier transform.
%   

