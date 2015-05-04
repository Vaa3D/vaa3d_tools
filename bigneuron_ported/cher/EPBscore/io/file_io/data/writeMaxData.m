function writeMaxData
% saves the max projection data into a 16 bit tiff files.  Each channel is saved sequentially in
% the same file
global state

if ~state.acq.averaging & state.acq.numberOfFrames > 1		% if it is not possible to do a max
	return													% then return
end
	
first = 1;
for channelCounter = 1:state.init.maximumNumberOfInputChannels
	if getfield(state.acq, ['maxImage' num2str(channelCounter)]) ...	% if max is on
			& getfield(state.acq, ['savingChannel' num2str(channelCounter)])	% and channel is saved
		if state.acq.maxMode==1
			state.acq.maxData{channelCounter}=uint16(state.acq.maxData{channelCounter});
		end
		if first
			fileName = [state.files.fullFileName 'max.tif'];
			imwrite(state.acq.maxData{channelCounter}, fileName,  'WriteMode', 'overwrite', ...
				'Compression', 'none', 'Description', state.headerString);	
			first = 0;
		else
			imwrite(state.acq.maxData{channelCounter}, fileName,  'WriteMode', 'append', ...
				'Compression', 'none', 'Description', state.headerString);	
		end
	end	
end

