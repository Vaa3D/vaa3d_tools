function varargout = spineanalysisimageGUI(varargin)
% spineanalysisimageGUI Application M-file for spineanalysisimageGUI.fig
%    FIG = spineanalysisimageGUI launch spineanalysisimageGUI GUI.
%    spineanalysisimageGUI('callback_name', ...) invoke the named callback.

% Last Modified by GUIDE v2.5 08-Feb-2011 14:27:39

if nargin == 0  % LAUNCH GUI
    fig = openfig(mfilename,'new');
    % Use system color scheme for figure:
    set(fig,'Color',get(0,'defaultUicontrolBackgroundColor'));
    
    handles = guihandles(fig);
    global ogh;
    ogh=[];
    ogh.mainGUI=handles;
    ogh.mainGUI.Figure=fig;
    parametersfig=openfig('spineanalysisparameters.fig','new');
    ogh.parametersGUI=guihandles(parametersfig);
    ogh.parametersGUI.Figure=parametersfig;
    global self;
    self=[];
    SpineAnalysisImage('standardspineanalysisimage.ini');
    backself=self;
    self=struct(self);
    initGUIs('standardspineanalysisparameters.ini');
    close(parametersfig);
    self.gh.parametersGUI=[];
    ogh.parametersGUI=[];
    backself=set(backself,'struct',self);
    self=backself;
    object=registerObject(self);
    
    % Generate a structure of handles to pass to callbacks, and store it. 
    handles = guihandles(fig);
    guidata(fig, handles);
    setappdata(fig,'object',object);
    set(fig,'DoubleBuffer', 'on');
    % handle window closing gracefully
    set(fig,'closeRequestFcn','global objects;obj=eval(getappdata(gcf,''object''));destroy(obj);');
    recordWindowPos(self);
    
    if nargout > 0
        varargout{1} = fig;
    end
    
elseif ischar(varargin{1}) % INVOKE NAMED SUBFUNCTION OR CALLBACK
    
    try
        if (nargout)
            [varargout{1:nargout}] = feval(varargin{:}); % FEVAL switchyard
        else
            feval(varargin{:}); % FEVAL switchyard
        end
    catch
        disp(lasterr);
    end
    
end

%| ABOUT CALLBACKS:
%| GUIDE automatically appends subfunction prototypes to this file, and 
%| sets objects' callback properties to call them through the FEVAL 
%| switchyard above. This comment describes that mechanism.
%|
%| Each callback subfunction declaration has the following form:
%| <SUBFUNCTION_NAME>(H, EVENTDATA, HANDLES, VARARGIN)
%|
%| The subfunction name is composed using the object's Tag and the 
%| callback type separated by '_', e.g. 'slider2_Callback',
%| 'figure1_CloseRequestFcn', 'axis1_ButtondownFcn'.
%|
%| H is the callback object's handle (obtained using GCBO).
%|
%| EVENTDATA is empty, but reserved for future use.
%|
%| HANDLES is a structure containing handles of components in GUI using
%| tags as fieldnames, e.g. handles.figure1, handles.slider2. This
%| structure is created at GUI startup using GUIHANDLES and stored in
%| the figure's application data using GUIDATA. A copy of the structure
%| is passed to each callback.  You can store additional information in
%| this structure at GUI startup, and you can change the structure
%| during callbacks.  Call guidata(h, handles) after changing your
%| copy to replace the stored original so that subsequent callbacks see
%| the updates. Type "help guihandles" and "help guidata" for more
%| information.
%|
%| VARARGIN contains any extra arguments you have passed to the
%| callback. Specify the extra arguments by editing the callback
%| property in the inspector. By default, GUIDE sets the property to:
%| <MFILENAME>('<SUBFUNCTION_NAME>', gcbo, [], guidata(gcbo))
%| Add any extra arguments after the last argument, before the final
%| closing parenthesis.

% --------------------------------------------------------------------
function varargout = generic_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    % kludge set the slider correctly
%    set();
if ~isempty(handles)
    set(handles.zslider,'Min',get(obj,'state.ROI.minz'));
    set(handles.zslider,'Max',get(obj,'state.ROI.maxz'));
    set(handles.zslider,'SliderStep',[1/(get(obj,'state.ROI.maxz')-get(obj,'state.ROI.minz')) 1/(get(obj,'state.ROI.maxz')-get(obj,'state.ROI.minz'))]);
end
    displayImage(obj);
    displayProjectionImage(obj);
    displayProfile(obj);
end

% --------------------------------------------------------------------
function varargout = zslider_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
end
% --------------------------------------------------------------------
function editbox_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end
% --------------------------------------------------------------------
function varargout = projectionbutton_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    obj=showprojection(obj);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayProjectionImage(obj);
end
% --------------------------------------------------------------------
function stackbutton_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    obj=showstack(obj);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
end

% --------------------------------------------------------------------
function varargout = profilebutton_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    obj=showprofile(obj);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayProfile(obj);
end

% --------------------------------------------------------------------
function varargout = parametersbutton_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    obj=showparameters(obj);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function varargout = rawbutton_Callback(h, eventdata, handles, varargin)
if (get(handles.rawbutton,'Value')==0)
    set(handles.rawbutton,'Value',1);
    return;
end;
global self;
global objects;
global ogh;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    self=struct(obj);
    self.state.display.displaybinary=0;
    ogh=self.gh;
    updateGUIbyglobal('self.state.display.displaybinary');
    self.state.display.displayfiltered=0;
    updateGUIbyglobal('self.state.display.displayfiltered');
    obj=set(obj,'struct',self);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
end
% --------------------------------------------------------------------
function varargout = filteredbutton_Callback(h, eventdata, handles, varargin)
if (get(handles.filteredbutton,'Value')==0)
    set(handles.filteredbutton,'Value',1);
    return;
end;
global self;
global ogh;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    self=struct(obj);
    self.state.display.displayraw=0;
    ogh=self.gh;
    updateGUIbyglobal('self.state.display.displayraw');
    self.state.display.displaybinary=0;
    updateGUIbyglobal('self.state.display.displaybinary');
    obj=set(obj,'struct',self);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
end
% --------------------------------------------------------------------
function varargout = binarybutton_Callback(h, eventdata, handles, varargin)
if (get(handles.binarybutton,'Value')==0)
    set(handles.binarybutton,'Value',1);
    return;
end;
global self;
global ogh;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    self=struct(obj);
    self.state.display.displayraw=0;
    ogh=self.gh;
    updateGUIbyglobal('self.state.display.displayraw');
    self.state.display.displayfiltered=0;
    updateGUIbyglobal('self.state.display.displayfiltered');
    obj=set(obj,'struct',self);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
end

% --------------------------------------------------------------------
function minz_Callback(h, eventdata, handles)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayProjectionImage(obj);
    displayImage(obj);
    displayProfile(obj);
end
% --------------------------------------------------------------------
function maxz_Callback(h, eventdata, handles)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayProjectionImage(obj);
    displayImage(obj);
    displayProfile(obj);
end
% ------------------------------------------------------------------------
function ch1_Callback(h, eventdata, handles)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayProjectionImage(obj);
    displayImage(obj);
    displayProfile(obj);
end
% ------------------------------------------------------------------------
function ch2_Callback(h, eventdata, handles)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayProjectionImage(obj);
    displayImage(obj);
    displayProfile(obj);
end



% ------------------------------------------------------------------------
% Mouse Events handling
% --------------------------------------------------------------------
function varargout = projectionimageaxes_ButtonDownFcn(h, eventdata, handles, varargin)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=ProjectionImageAxes_ButtonDown(obj);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
     displayImage(obj);
     displayProjectionImage(obj);
     displayProfile(obj);
end

% --------------------------------------------------------------------
function varargout = imageaxes_ButtonDownFcn(h, eventdata, handles, varargin)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=ImageAxes_ButtonDown(obj);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
     displayImage(obj);
     displayProjectionImage(obj);
     displayProfile(obj);
end

% ------------------------------------------------------------------------
% Menu Items
% ------------------------------------------------------------------------
function varargout = openimage_Callback(h, eventdata, handles, varargin)
% get the object associated with the control window
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=openimage(obj);  
    obj=showparameters(obj);
    obj=showstack(obj);
    obj=showprojection(obj);
    obj=showprofile(obj);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
    displayProfile(obj);
end

% --------------------------------------------------------------------
function opendualchannelimage_Callback(h, eventdata, handles)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=opendualchannelimage(obj);  
    obj=showparameters(obj);
    obj=showstack(obj);
    obj=showprojection(obj);
    obj=showprofile(obj);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
    displayProfile(obj);
end

% --------------------------------------------------------------------
function opentwolaser_Callback(h, eventdata, handles)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=opentwolaserimage(obj);  
    obj=showparameters(obj);
    obj=showstack(obj);
    obj=showprojection(obj);
    obj=showprofile(obj);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
    displayProfile(obj);
end
% --------------------------------------------------------------------
function varargout = Load_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=load(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    obj=showparameters(obj);
    obj=showstack(obj);
    obj=showprojection(obj);
    obj=showprofile(obj);
    displayImage(obj);
    displayProjectionImage(obj);
    displayProfile(obj);
end

% --------------------------------------------------------------------
function varargout = Save_Callback(h, eventdata, handles, varargin)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=save(obj);
end

% --------------------------------------------------------------------
function exportmaxprojection_Callback(h, eventdata, handles)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=exportmaxprojection(obj);
end

% --------------------------------------------------------------------
function exportstack_Callback(h, eventdata, handles)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=exportstack(obj);
end


% --------------------------------------------------------------------
function varargout = savesection_Callback(h, eventdata, handles, varargin)
data= getappdata(gcbf,'data');
[filename, pathname] = uiputfile('*.tif','Save As');
section=uint16(data.imageArray(:,:,data.parameters.ROI.minz:data.parameters.ROI.maxz));
%section=uint16(data.filteredArray(:,:,data.parameters.ROI.minz:data.parameters.ROI.maxz));
%proj=collapse(data.filteredArray(:,:,max(data.parameters.ROI.minz,1):min(data.parameters.ROI.maxz,size(data.imageArray,3))),'XY');
%arrayToTiff(proj, [pathname filename], '');
arrayToTiff(section, [pathname filename], '');

% --------------------------------------------------------------------
function varargout = Close_Callback(h, eventdata, handles, varargin)


% --------------------------------------------------------------------
function Keepintermediates_Callback(h, eventdata, handles)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    self=struct(obj);
    if strcmp(get(h,'Checked'),'on')
        set(h,'Checked','off');
        self.state.display.keepintermediates=0;
    else
        set(h,'Checked','on');
        self.state.display.keepintermediates=1;
    end
    obj=set(obj,'struct',self);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function varargout = Deconvolve_Callback(h, eventdata, handles, varargin)
data= getappdata(gcbf,'data');
if isfield(data,'medianArray')
    w = waitbar(0.1,'Deconvolving of Tif', 'Name', 'Deconvolving');
    counter=0;
    ker=opentif('D:\research\newkernel.tif'); % !!! this needs to be changed to reflect where your kernel image file is stored
    if (size(data.medianArray,3)==1)
        maxker=collapse(ker,'XY');
        data.filteredArray=deconv3d(data.medianArray,uint16(maxker),struct('maxITER',data.parameters.deconvolutioniteration,'iresx',0.088,'iresy',0.088,'kresx',0.0365,'kresy',0.0365));
    else
        data.filteredArray=deconv3d(data.medianArray,uint16(ker),struct('maxITER',data.parameters.deconvolutioniteration,'iresx',0.128,'iresy',0.128,'kresx',0.0365,'kresy',0.0365));
    end
    %suppress the edges    
    data.filteredArray(1:3,:,:)=deal(0);
    data.filteredArray(1:3,1,:)=deal(0);
    data.filteredArray(size(data.filteredArray,1)-3:size(data.filteredArray,1),:,:)=deal(0);
    data.filteredArray(:,size(data.filteredArray,2)-3:size(data.filteredArray,2),:)=deal(0);
    close(w);
    setappdata(gcbf,'data',data);
    set(handles.filteredbutton,'Value',1);
    filteredbutton_Callback(h, eventdata, handles, varargin);
end;
% --------------------------------------------------------------------
function varargout = MedianFilter_Callback(h, eventdata, handles, varargin)
global self;
global objects;
global ogh;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=medianFilter(obj);
    self=struct(obj);
    ogh=self.gh;
    updateGUIbyglobal('self.state.display.lowpixelch1');
    if (self.state.display.ch2==1)
     updateGUIbyglobal('self.state.display.lowpixelch2');
    end
    self.state.display.displayfiltered=1;
    updateGUIbyglobal('self.state.display.displayfiltered');
    self.state.display.displayraw=0;
    updateGUIbyglobal('self.state.display.displayraw');
    self.state.display.displaybinary=0;
    updateGUIbyglobal('self.state.display.displaybinary');
    obj=set(obj,'struct',self);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
end
% --------------------------------------------------------------------
function varargout = Binarize_Callback(h, eventdata, handles, varargin)
global self;
global objects;
global ogh;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=Binarize(obj);
    self=struct(obj);
    ogh=self.gh;
    self.state.display.displayfiltered=0;
    updateGUIbyglobal('self.state.display.displayfiltered');
    self.state.display.displayraw=0;
    updateGUIbyglobal('self.state.display.displayraw');
    self.state.display.displaybinary=1;
    updateGUIbyglobal('self.state.display.displaybinary');
    obj=set(obj,'struct',self);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
end
% --------------------------------------------------------------------
function varargout = FindProcesses_Callback(h, eventdata, handles, varargin)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=FindProcesses(obj);
    obj=set(obj,'state.display.displaydendrites',1);
    self=struct(obj);
    ogh=self.gh;
    updateGUIbyglobal('self.state.display.displaydendrites');
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
end

% ------------------------ --------------------------------------------
function varargout = AxonProfile_Callback(h, eventdata, handles, varargin)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=AxonProfile(obj);
    obj=set(obj,'state.display.displayspines',1);
    self=struct(obj);
    ogh=self.gh;
    updateGUIbyglobal('self.state.display.displayspines');
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
    displayProfile(obj);
end


% ------------------------ --------------------------------------------
function varargout = FindSpines_Callback(h, eventdata, handles, varargin)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=FindSpines(obj);
    obj=set(obj,'state.display.displayspines',1);
    self=struct(obj);
    ogh=self.gh;
    updateGUIbyglobal('self.state.display.displayspines');
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
end

% --- Executes on button press in changelabel.
function changelabel_Callback(h, eventdata, handles)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=changelabel(obj);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
    displayProfile(obj);
end

% ------------------------ --------------------------------------------
function varargout = cutstack_Callback(h, eventdata, handles, varargin)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=cutStack(obj);
    obj=set(obj,'state.display.displayspines',1);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
end

% --------------------------------------------------------------------
function varargout = Cut_Callback(h, eventdata, handles, varargin)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=cutSection(obj);
    obj=set(obj,'state.display.displayspines',1);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
end

% --------------------------------------------------------------------
% function selectdendrites_Callback(h, eventdata, handles, varargin)
% global self;
% global objects;
% str=getappdata(parentFigure(h),'object');
% if (str)
%     obj=eval(getappdata(parentFigure(h),'object'));
%     obj=selectDendrites(obj);
%     eval([getappdata(parentFigure(h),'object') '=obj;']);
%     displayImage(obj);
%     displayProjectionImage(obj);
%     displayProfile(obj);
% end

% --------------------------------------------------------------------
function varargout = ExtendAxon_Callback(h, eventdata, handles, varargin)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=extendAxon(obj);
    obj=set(obj,'state.display.displayspines',1);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
end



% --------------------------------------------------------------------
function varargout = ShowRejected_Callback(h, eventdata, handles, varargin)
data= getappdata(gcbf,'data');
if (data.GUI.displayrejectedspine==0)
    data.GUI.displayrejectedspine=1;
else
    data.GUI.displayrejectedspine=0;
end
setappdata(gcbf,'data',data);



% --------------------------------------------------------------------
function varargout = Brighten_Callback(h, eventdata, handles, varargin)
data= getappdata(gcbf,'data');
%[xmin, ymin, width, height]=getrect(data.fig);
rect=getrect(handles.imageaxes);
if (data.GUI.displayfiltered)
    data.filteredArray(rect(2):rect(2)+rect(4),rect(1):rect(1)+rect(3),data.GUI.currentz)=uint16(double(data.filteredArray(rect(2):rect(2)+rect(4),rect(1):rect(1)+rect(3),data.GUI.currentz))*1.2);     
    displayImage(handles,data);
end
setappdata(gcbf,'data',data);



% --------------------------------------------------------------------
function varargout = Darken_Callback(h, eventdata, handles, varargin)
data= getappdata(gcbf,'data');
%[xmin, ymin, width, height]=getrect(data.fig);
rect=getrect(handles.imageaxes);
if (data.GUI.displayfiltered)
    data.filteredArray(rect(2):rect(2)+rect(4),rect(1):rect(1)+rect(3),data.GUI.currentz)=uint16(double(data.filteredArray(rect(2):rect(2)+rect(4),rect(1):rect(1)+rect(3),data.GUI.currentz))*0.5);     
    displayImage(handles,data);
end
setappdata(gcbf,'data',data);

% --------------------------------------------------------------------
function varargout = ShowIntensityHistogram_Callback(h, eventdata, handles, varargin)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    self=struct(obj);
    if (isfield(self.data,'filteredArray'))
        [n,x]=hist(double(self.data.imageArray(:)),200);
      
        figure;
        bar(x,n);
        title([num2str(median(double(self.data.imageArray(:)))) ' ' num2str(mean(double(self.data.imageArray(:))))]);    
    end
end;


% --------------------------------------------------------------------
function EditSpines_Callback(h, eventdata, handles)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=set(obj,'state.display.mousemode','editspines');
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    
    %set(obj.gh.mainGUI.EditMarks,'Checked','off');
    %set(obj.gh.mainGUI.EditSpines,'Checked','on');
end


% --------------------------------------------------------------------
function EditMarks_Callback(h, eventdata, handles)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=set(obj,'state.display.mousemode','editmarks');
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    
    %set(obj.gh.mainGUI.EditMarks,'Checked','on');
    %set(obj.gh.mainGUI.EditSpines,'Checked','off');
end


% --------------------------------------------------------------------
function summaryfigure_Callback(h, eventdata, handles)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    summaryfigure(obj);
end


% --------------------------------------------------------------------
function CutAxons_Callback(h, eventdata, handles)
% hObject    handle to CutAxons (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=cutAxons(obj);
    obj=set(obj,'state.display.displayspines',1);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
    displayProfile(obj);
end

% --------------------------------------------------------------------
function JoinAxons_Callback(h, eventdata, handles)
% hObject    handle to JoinAxons (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=joinAxons(obj);
    obj=set(obj,'state.display.displayspines',1);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
    displayProfile(obj);
end


% --------------------------------------------------------------------
function StraightenAxon_Callback(h, eventdata, handles)
% hObject    handle to StraightenAxon (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=StraightenAxon(obj);
    obj=set(obj,'state.display.displayspines',1);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
    displayProfile(obj);
end

% --------------------------------------------------------------------
function CommitChanges_Callback(h, eventdata, handles)
% hObject    handle to CommitChanges (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=commitChanges(obj);
    obj=set(obj,'state.display.displayspines',1);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
    displayProfile(obj);
end


% --------------------------------------------------------------------
function ROI_Callback(h, eventdata, handles)
% hObject    handle to CommitChanges (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=ROI(obj);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
    displayProfile(obj);
end


% --------------------------------------------------------------------
function ComputeDistance_Callback(h, eventdata, handles)
% hObject    handle to ComputeDistance (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=CorrectDendrites(obj);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
    displayProfile(obj);
end


% --------------------------------------------------------------------
function ShiftAxonMax_Callback(h, eventdata, handles)
% hObject    handle to ShiftAxonMax (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=ShiftDendriteMaxObj(obj,0);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
    displayProfile(obj);
end

% --------------------------------------------------------------------
function GaussianFilter_Callback(h, eventdata, handles)
% hObject    handle to GaussianFilter (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global self;
global objects;
global ogh;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=gaussianFilter(obj);
    self=struct(obj);
    ogh=self.gh;
%    self.data.filteredArray=self.data.gaussianArray;
    self.state.display.displaygaussian=1;
    updateGUIbyglobal('self.state.display.displaygaussian');
    self.state.display.displayraw=0;
    updateGUIbyglobal('self.state.display.displayraw');
    self.state.display.displaybinary=0;
    updateGUIbyglobal('self.state.display.displaybinary');
    obj=set(obj,'struct',self);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
end


% --------------------------------------------------------------------
function ShiftBranchMax_Callback(h, eventdata, handles)
% hObject    handle to ShiftAxonMax (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=ShiftBranchMaxObj(obj,0);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
    displayProfile(obj);
end

% --------------------------------------------------------------------
function varargout = NewAxon_Callback(h, eventdata, handles, varargin)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=newAxon(obj);
    obj=set(obj,'state.display.displayspines',1);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
end


% --------------------------------------------------------------------
function BranchStats_Callback(h, eventdata, handles)
% hObject    handle to Keepintermediates (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=BranchStats(obj,0);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
    displayProfile(obj);
end


% --------------------------------------------------------------------
function ClearProcesses_Callback(h, eventdata, handles)
% hObject    handle to ClearProcesses (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    self=struct(obj);
    ogh=self.gh;
    self.data.dendrites=[];
    self.data.spines=[];
    self.data.marks=[];
    obj=set(obj,'struct',self);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
    displayProfile(obj);
end


% --------------------------------------------------------------------
function SegmentAxon_Callback(h, eventdata, handles)
% hObject    handle to Keepintermediates (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global self;
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=segmentAxonObj(obj);
    obj=set(obj,'state.display.displayspines',1);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
end


% --------------------------------------------------------------------
function Exportbouton_Callback(hObject, eventdata, handles)
% hObject    handle to Exportbouton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global self;
global objects;
str=getappdata(parentFigure(hObject),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=ExportboutonObj(obj);
end


% --------------------------------------------------------------------
function SelectAxons_Callback(hObject, eventdata, handles)
% hObject    handle to SelectAxons (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global self;
global objects;
str=getappdata(parentFigure(hObject),'object');
if (str)
    obj=eval(getappdata(parentFigure(hObject),'object'));
    obj=selectDendrites(obj);
    eval([getappdata(parentFigure(hObject),'object') '=obj;']);
    displayImage(obj);
    displayProjectionImage(obj);
    displayProfile(obj);
end



function binarythreshold_Callback(h, eventdata, handles)
% hObject    handle to binarythreshold (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of binarythreshold as text
%        str2double(get(hObject,'String')) returns contents of binarythreshold as a double
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
    displayProjectionImage(obj);
    displayImage(obj);
    displayProfile(obj);
end




% --- Executes during object creation, after setting all properties.
function binarythreshold_CreateFcn(hObject, eventdata, handles)
% hObject    handle to binarythreshold (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in Binarizebutton.
function Binarizebutton_Callback(hObject, eventdata, handles)
% hObject    handle to Binarizebutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global self;
global objects;
global ogh;
str=getappdata(parentFigure(hObject),'object');
if (str)
    obj=eval(getappdata(parentFigure(hObject),'object'));
  
    obj=Binarize(obj);
    self=struct(obj);
    ogh=self.gh;
    self.state.display.displayfiltered=0;
    updateGUIbyglobal('self.state.display.displayfiltered');
    self.state.display.displayraw=0;
    updateGUIbyglobal('self.state.display.displayraw');
    self.state.display.displaybinary=1;
    updateGUIbyglobal('self.state.display.displaybinary');
    obj=set(obj,'struct',self);
    eval([getappdata(parentFigure(hObject),'object') '=obj;']);
    
    displayImage(obj);
    displayProjectionImage(obj);
end


% --------------------------------------------------------------------
function SelectGaussian_Callback(hObject, eventdata, handles)
% hObject    handle to SelectGaussian (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global self;
global objects;
global ogh;
str=getappdata(parentFigure(hObject),'object');
if (str)
    obj=eval(getappdata(parentFigure(hObject),'object'));
    self=struct(obj);
    numstr=char(self.state.display.editbox);
    num=str2num(numstr);
    self.state.display.gaussian=num;
    obj=set(obj,'struct',self);
    eval([getappdata(parentFigure(hObject),'object') '=obj;']);
    
    displayImage(obj);
    displayProjectionImage(obj);
end
