function doGUICallBack(handle)
% calls the Callback function of a GUI if one has been set by the user
	if hasUserDataField(handle, 'Callback')
		funcName=getUserDataField(handle, 'Callback');
        % Sen 11/27/03 if function has a form of a-b then it's a subfunction within a function
        if sum(funcName=='-')>0
            [funcN,subfuncN]=strtok(funcName,'-');
            if exist(funcN)==2
			    try
				    eval([funcN '(''' subfuncN(2:end) ''',handle,[],guidata(handle));']);
			    catch
				    disp(['doGUICallBack: ' lasterr ' in ' funcName ]);
			    end
		    else
			    if length(funcName)>0
				    disp(['doGUICallBack: Callback function (' funcName ') does not exist. No action.']);
			    end
		    end
        elseif exist(funcName)==2
			try
				eval([funcName '(handle);']);
			catch
				disp(['doGUICallBack: ' lasterr ' in ' funcName ]);
			end
		else
			if length(funcName)>0
				disp(['doGUICallBack: Callback function (' funcName ') does not exist. No action.']);
			end
		end
	end
		