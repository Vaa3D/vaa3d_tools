%% IMSEG plots an image with its superpixel segmentation 
%
%   I = imseg(I, L)
%   See also NLFILTER

%   Copyright © 2009 Computer Vision Lab, 
%   École Polytechnique Fédérale de Lausanne (EPFL), Switzerland.
%   All rights reserved.
%
%   Authors:    Kevin Smith         http://cvlab.epfl.ch/~ksmith/
%               Aurelien Lucchi     http://cvlab.epfl.ch/~lucchi/
%
%   This program is free software; you can redistribute it and/or modify it 
%   under the terms of the GNU General Public License version 2 (or higher) 
%   as published by the Free Software Foundation.
%                                                                     
% 	This program is distributed WITHOUT ANY WARRANTY; without even the 
%   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
%   PURPOSE.  See the GNU General Public License for more details.

function I = imseg(I, L)

fun = @(x) ~sameval(x(:));
E = nlfilter(L, [3 3], fun);
%E = bwmorph(E, 'shrink');
E = bwmorph(~E, 'thicken', Inf);
E = bwmorph(~E, 'shrink');

I(E == 1) = 50;

