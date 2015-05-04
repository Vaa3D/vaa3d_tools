function Aout = opentif(filename)
global state

% Open a tif file and store its contents as array Aout.
% filename is the file name with extension.  
% 
% Written By: Thomas Pologruto
% Cold Spring Harbor Labs
% February 1, 2001

frames = length(imfinfo(filename));
h = waitbar(0,'Opening Tif image...', 'Name', 'Open TIF Image', 'Pointer', 'watch');
state.imageProc.colorMap = 0;

	for i = 1:frames
	waitbar(i/frames,h, ['Loading Frame Number ' num2str(i)]);
		try
			Aout(:,:,i) = imread(filename, i);
		catch
			if i == 1  %If it is mismatch error on first frame, it is a rgb tif file.
				try
					Aout = imread(filename);
					state.imageProc.colorMap = 1;
					waitbar(1,h, 'Done');
					close(h);
					return
				catch
					beep;
					waitbar(1,h, 'Done');
					close(h);
					Aout = 'File Not Found';
					disp(['File ' filename ' Not Found; Check Name or Path.']);
				end
			end
		end	
	end

	waitbar(1,h, 'Done');
	close(h);



