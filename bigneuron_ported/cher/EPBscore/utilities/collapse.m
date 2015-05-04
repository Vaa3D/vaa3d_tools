function imageOut = collapse(Image, directionOfCollapse)
global state gh

% This function will tahe an image as array (3d stack of intensity images) and 
% will take a maximum projection along the directionOfCollapse input (either XY, XZ, or YZ);
%

% sizeimage = size(Image);

Rows = size(1,1);
Columns = size(1,2);

if ndims(Image) == 3
	NumberOfFrames = size(1,3);
else
	NumberOfFrames=1;
	imageOut = Image;
	return
end

switch directionOfCollapse
case 'XY' % Collapse Along Z
	imageOut = max(Image,[],3);

% 	Image = reshape(Image, (Rows*Columns), NumberOfFrames);
% 	Image = Image';
% 	Image = max(Image);
% 	imageOut = reshape(Image, Rows, Columns);

case 'XZ' % Collapse along Y
	Image = max(Image,[],1);

% 	Image = permute(Image, [2 3 1]);
% 	Image = reshape(Image, (NumberOfFrames*Columns), Rows);
% 	Image = Image';
% 	Image = max(Image);
	imageOut = reshape(Image, Columns, NumberOfFrames);
 	imageOut = imageOut';
% 	
case 'YZ' % Collapse along X
	Image = max(Image,[],2);
	%imageOut = reshape(imageOut, Rows, NumberOfFrames, Columns);
	
% 	Image = permute(Image, [1 3 2]);
% 	Image = reshape(Image, (Rows*NumberOfFrames), Columns);
% 	Image = Image';
% 	Image = max(Image);
	imageOut = reshape(Image, Rows, NumberOfFrames);
	
	
otherwise
	disp('The direction of projection must be ''XY'' , ''XZ'', or ''YZ''.');
end




