function maximize(h)

% MAXIMIZE the size of a window to fill the entire screen.
%
% maximize(h)
%	resize figure h to fill the screen.
%
%	The function was tested under the following conditions:
%	screen resolutions:
%	[800x600]
%	[1024x768]
%	[1152x864]
%	[1280x960]
%	Matlab version: R12, R13
%	OS:	WinXP
%
% maximize('calibrate')
%	The function was calibrated to handle WinXP standard toolbar (at the
%	bottom of the screen, one icon row) and standard screen (4x3 ratio - not
%	wide). If you are using different settings, the maximized figure size
%	could be wrong.
%	If you find yourself in such a position, fear not. Run the function in
%	'calibrate' mode to calibrate the settings.
%	The application will ask you to maximize a figure window manually and
%	use these settings as a reference from now on.
%
% maximize('forget')
%	forget the calibration settings (delete the file) and use default
%	settings from now on.

% Author:	Yuval Cohen, Be4 Ltd.
%			yuval@gobe4.com

% You are free to use and distribute this code, as long as credit comments
% are preserved.

% Revision History
% 5-Aug-2003	V 1.0
% 16-Aug-2003   V 1.1   Calibration modified

% check input argument
error(nargchk(1,1,nargin));

% Calibration file
[p,f,e]=fileparts(mfilename);
fn=fullfile(p,'Maximize screen calibration.mat');

method=0;
if isstr(h)
    method=h;
    h=0;
elseif ishandle(h(1))
    if ~strcmp(get(h,'Type'),'figure')
        error('Invalid figure handle')
    end
elseif h(1)==-1;
    % function called by the resize function
    set(h(2),'Units','normalized');
    maxPos=get(h(2),'position');
    
    % check if the window is indeed maximized
    if (maxPos(1)==0 & maxPos(3)==1) | (maxPos(2)==0 & maxPos(4)==1)
        set(h(2),'Units','pixels','ActivePositionProperty','outerposition');
        maxPos=get(h(2),'OuterPosition');
        set(h(3),'String',['Calibration complete.' char(10) 'Please close this window.'],'Color','b');
        set(h(2),'ResizeFcn','');
        % save calibration file
        try
            save (fn,'maxPos');
        catch
            error('Failed to save calibration file.');
        end
    end
    return
else
	error('Invalid function usage')
end

if method
	switch lower(method)
		case {'calibrate','calib'}
			% calibration
            h=figure;
            hT=text(0,0,['Calibrating window size.' char(10) 'Please maximize this window.'], ...
                'HorizontalAlignment','Center','FontSize',20,'Color','r');
            callBackStr=sprintf('%s %0.15f %0.15f%s','maximize([-1',h,hT,'])');
            set(h,'ResizeFcn',callBackStr);
            axis([-1 1 -1 1]); axis off
			return
		case 'forget'
			% Use default settings and delete the calibration file
            if exist(fn,'file')
                try
                    delete(fn);
                end
                disp('Calibration file deleted. Using default settings.');
            end
		otherwise
			% unknown method
			set(h,'Units',oldUnits,'ActivePositionProperty',oldActivePositionProperty);
			error('Invalid method used')
	end
else
	% try loading the calibration file, if exists
	try
		load(fn,'maxPos')
    catch
        % determine defaults figure size
        oldRootUnits=get(0,'Units');
        set(0,'Units','Pixels');
        pos=get(0,'ScreenSize');
        set(0,'Units',oldRootUnits);
        maxPos=[-3 27 pos(3)+8 pos(4)-22];
    end
    % store old window properties
    oldUnits=get(h,'Units');
    oldActivePositionProperty=get(h,'ActivePositionProperty'); % an undocumented figure property
    set(h,'Units','Pixels','ActivePositionProperty','outerposition');
    
    % resize the window
    set(h,'Position',maxPos,'OuterPosition',maxPos);
    
    % restore old figure properties
    set(h,'Units',oldUnits,'ActivePositionProperty',oldActivePositionProperty);
end
