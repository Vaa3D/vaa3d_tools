function arrayToTiff(arrayName, str, str2)
global state gh

% Function that takes a 3D or 2D array and converts it into a tiff file with the 
% name from the name string above.
% You must input the extension as the filename input str.
%
% The second input string is the header for that image.
% It only gets written to the first frame of a multi-frame image.
%
% The first frame is overwritten, to avoid cocatenation of arrays.
%
% Written By: Thomas Pologruto
% Cold Spring Harbor Labs
% January 30, 2001

fileName = str;
header = str2;
fullFileName = [fileName '.tif'];

if ndims(arrayName) == 2
	imwrite(arrayName, fullFileName , 'writeMode', 'overwrite', 'description', header);
else
	numberOfFrames = size(arrayName, 3);
	for i = 1 
		imwrite(arrayName(:,:,i), fullFileName,  'writeMode', 'overwrite', 'description', header);
	end

	for i = 2:numberOfFrames;
		imwrite(arrayName(:,:,i), fullFileName, 'writeMode', 'append');
	end
end
