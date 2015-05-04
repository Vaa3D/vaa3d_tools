function out=savingInfoIsOK
%updated by tp 10/25/01

global state
st=[];
% TPMOD 2/6/02
if state.files.autoSave	
    if isempty(state.files.baseName) %if no base name is chosen
        disp('*** ERROR: Please enter a basename ***')
        answer  = inputdlg('Select base name','Choose Base Name for Acquisition',1,{state.files.baseName});
        if ~isempty(answer)
            state.files.baseName = answer{1};
            updateGUIbyglobal('state.files.baseName');
        else
            st='? Basename ';
        end
    end
    if isempty(state.files.savePath)
        disp('*** ERROR: Please set a save path using save ''File\Header Structure As...'' ***');
        button = questdlg('A Save path has not been selected.','Do you wish to:','Select New Path','Use Current','Select New Path');
        if strcmp(button,'Select New Path')
            setSavePath;
        else strcmp(button,'Use Current')  
            state.files.savePath = 'C:/';
            updateGUIbyglobal('state.files.savePath');
        end
    end
    if isempty(state.files.savePath)
        if isempty(st)
            st='? Save Path ';
        else
            st=[st ', Path'];
        end
    end
    
    if ~isempty(st)
        st=[st '?'];
        setStatusString(st);
        beep; %TPMOD 2/6/02
        out=0;
    else % everything is ok...
        disp([clockToString(clock) ' *** '''  state.files.fullFileName ''' ***']);
        out=1;
    end
else
    out=1;
end

