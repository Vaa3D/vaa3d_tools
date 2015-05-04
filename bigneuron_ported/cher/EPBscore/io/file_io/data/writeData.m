function writeData
	global state

% writeData.m******
% Function that searches through all the channels and sees which to save.
% It then will save the acquired Data from the global state.acq.acquiredData{i} (i = 1:numberOFChannels)
% as tif files.  
% The images are interleaved....They are stored as frame 1, Channel 1, frame 1, Channel 2, ....
% frame 2, channel 1, frame 2, channel 2, ....and Z-slice stacks are stored one after the other.
%
% Each new Z-slice has the string header state.headerString stored in the tif header under the 
% field ImageDescription.
%
% Written By: Bernardo Sabatini and Thomas Pologruto
% Cold Spring Harbor Labs
% Februarry 1, 2001
		
	% Make the file name witht the tif extension
	fileName = [state.files.fullFileName '.tif'];
	
	if state.internal.zSliceCounter == 0
		first = 1;
	else 
		first = 0;
	end
	
	% If we are averaging, then there is only 1 frame per Z-Slice
	if state.acq.averaging 
		numberOfFrames = 1;
	else  % If we are not averaging,, there are state.acq.numberOfFrames per slice
		numberOfFrames = state.acq.numberOfFrames;
	end
	
	if state.internal.keepAllSlicesInMemory		% BSMOD 1/18/2
		startingFrame=state.internal.zSliceCounter*numberOfFrames;
	else
		startingFrame=0;
	end
	
	for frameCounter=1:numberOfFrames % Loop through all the frames
		for channelCounter = 1:state.init.maximumNumberOfInputChannels % Loop through all the channels
			if getfield(state.acq, ['savingChannel' num2str(channelCounter)]) % If saving..
				if first % if its the first frame of first channel, then overwrite...
					imwrite(state.acq.acquiredData{channelCounter}(:,:,frameCounter + startingFrame) ... % BSMOD 1/18/2
						, fileName,  'WriteMode', 'overwrite', 'Compression', 'none', 'Description', state.headerString);
					first = 0;
				else
					imwrite(state.acq.acquiredData{channelCounter}(:,:,frameCounter + startingFrame) ... % BSMOD 1/18/2
						, fileName,  'WriteMode', 'append', 'Compression', 'none');
				end	
			end
		end
	end
