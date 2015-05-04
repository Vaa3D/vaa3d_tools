function varargout = SeriesAnalysisAxon(varargin)
% SPINESERIESANALYSISAXON Application M-file for SpineSeriesAnalysis.fig
%    FIG = SPINESERIESANALYSISAXON launch SpineSeriesAnalysis GUI.
%    SPINESERIESANALYSISAXON('callback_name', ...) invoke the named callback.

% Last Modified by GUIDE v2.5 12-Jan-2004 11:48:38

if nargin == 0  % LAUNCH GUI

	fig = openfig(mfilename);

	% Use system color scheme for figure:
	set(fig,'Color',get(0,'defaultUicontrolBackgroundColor'));

    % Generate a structure of handles to pass to callbacks, and store it. 
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
    SpineSeriesAnalysis('standardspineseriesanalysis.ini');
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
    % handle window closing gracefully
    set(fig,'closeRequestFcn','global objects;obj=eval(getappdata(gcf,''object''));destroy(obj);');

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


% --------------------------------------------------------------------
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
function varargout = alignbutton_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=fiducialalign(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function varargout = tracespines_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=tracespines(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% 
% 
% % --------------------------------------------------------------------
% function varargout = NewImage_Callback(h, eventdata, handles, varargin)
% data= getappdata(gcbf,'data');
% data.images=[data.images spineanalysisimage];
% data.totalimages=data.totalimages+1;
% set(data.images(data.totalimages),'name',[int2str(data.totalimages) '-']);
% setappdata(gcbf,'data',data);


% --------------------------------------------------------------------
function varargout = OpenImages_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=getimages(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function varargout = LoadSelections_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=loadselections(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function varargout = SaveSelections_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=saveselections(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% % --------------------------------------------------------------------
% function varargout = Deblur_Callback(h, eventdata, handles,  varargin)
% data= getappdata(gcbf,'seriesdata');
% filenames=get(handles.listbox,'String');
% pathname=data.pathname;
% ker=opentif('d:\research\newkernel1.tif');
% for i=1:size(filenames,1)
%     [path,name,ext] = fileparts([pathname char(filenames(i))]);
%     if (strcmp(ext,'.tif'))
%         data=initializeImageData;
%         try
%             data.imageArray=opentif([path '\' name ext]);
%         catch
%             print('Could not open image');
%             continue;
%         end
%         data.path=path;
%         data.name=name;
%         tmpArray=cat(3,zeros(size(data.imageArray,1),size(data.imageArray,2)),data.imageArray,zeros(size(data.imageArray,1),size(data.imageArray,2)));
%         data.medianArray=median3d(tmpArray);
%         data.medianArray=data.medianArray(:,:,2:size(data.medianArray,3)-1);
%         data.filteredArray=deconv3d(data.medianArray,uint16(ker),struct('maxITER',data.parameters.deconvolutioniteration,'iresx',0.088,'iresy',0.088,'kresx',0.0117,'kresy',0.0117));  
%         save([path '\' name '.spi'],'data');
%     end
% end


% --------------------------------------------------------------------
function varargout = Analyze_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=analyzeall(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end
% --------------------------------------------------------------------
function varargout = ReAnalyze_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=reanalyzeall(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function varargout = ReTrace_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=retraceall(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function varargout = Montage_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=montage(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end



% --------------------------------------------------------------------
function varargout = parameters_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    obj=showparameters(obj);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function generic_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function listbox_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function varargout = inputtext_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=genericCallbackObj(obj,h);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function varargout = NextPair_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=nextpair(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function varargout = SpineTrace_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=tracespines(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function varargout = manualalign_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=manualalign(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function varargout = autotracespines_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=autotracespines(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function varargout = manualtracespines_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=manualtracespines(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function varargout = seriestrace_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=seriestrace(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function varargout = saveseries_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=saveseries(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function varargout = SelectDatabase_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    [dbname, dbpath] =  uigetfile({'*.db'}, 'Choose database file to load');
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=set(obj,'state.dbpath',[dbpath dbname]);
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end

% --------------------------------------------------------------------
function varargout = SaveDatabase_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    global db;
    [dbname, dbpath] =  uigetfile({'*.db'}, 'Choose database file to save to');
    save('-mat',[dbpath dbname],'db');    
end


% --------------------------------------------------------------------
function varargout = summaryfigure_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=saveseries(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end
% --------------------------------------------------------------------
function varargout = imagestrace_Callback(h, eventdata, handles, varargin)
global objects;
str=getappdata(parentFigure(h),'object');
if (str)
    obj=eval(getappdata(parentFigure(h),'object'));
    obj=imagestrace(obj);  
    eval([getappdata(parentFigure(h),'object') '=obj;']);
end


% 
% % --------------------------------------------------------------------
% function varargout = removespines1_Callback(h, eventdata, handles, varargin)
% seriesdata= getappdata(gcbf,'seriesdata');
% data=getappdata(seriesdata.currentimage,'data');
% numstr=char(get(handles.inputtext,'String'));
% rem=numstr;
% while(rem)
% [token,rem]=strtok(rem);
% [first,second]=strtok(token,'-');
% if (second)
%     first=str2double(first);
%     second=abs(str2double(second));
%     for i=first:second
%         data.spines(find([data.spines.label]==i)).edittype=1;
%     end    
% else
%     num=str2double(token);
%     if (num>0)
%         data.spines(find([data.spines.label]==num)).edittype=1;
%     else
%         data.spines(find([data.spines.label]==abs(num))).edittype=0; 
%     end
% end
% end
% %displayImage(guidata(seriesdata.currentimage),data);
% displayProjectionImage(guidata(seriesdata.currentimage),data);
% setappdata(seriesdata.currentimage,'data',data);
% 
% % --------------------------------------------------------------------
% function varargout = removespines2_Callback(h, eventdata, handles, varargin)
% seriesdata= getappdata(gcbf,'seriesdata');
% data=getappdata(seriesdata.previmage,'data');
% numstr=char(get(handles.inputtext,'String'));
% rem=numstr;
% while(rem)
% [token,rem]=strtok(rem);
% [first,second]=strtok(token,'-');
% if (second)
%     first=str2double(first);
%     second=str2double(second);
%     for i=first:second
%         data.spines(find([data.spines.label]==num)).edittype=1;
%     end    
% else
%     num=str2double(token);
%     if (num>0)
%         data.spines(find([data.spines.label]==num)).edittype=1;
%     else
%         data.spines(find([data.spines.label]==abs(num))).edittype=0; 
%     end
% end
% end
% %displayImage(guidata(seriesdata.previmage),data);
% displayProjectionImage(guidata(seriesdata.previmage),data);
% setappdata(seriesdata.previmage,'data',data);
% 
% % --------------------------------------------------------------------
% function varargout = reorderspines1_Callback(h, eventdata, handles, varargin)
% seriesdata= getappdata(gcbf,'seriesdata');
% data=getappdata(seriesdata.currentimage,'data');
% currentlabel=data.newspinenum;
% spineind=[];
% for i=1:size(data.dendrites,2)
%     loc=find([data.spines.den_ind]==i);
%     [A,B]=sort([data.spines(loc).dendis]);
%     spineind=[spineind loc(B)];
% end
% data.spines=data.spines(spineind);
% for i=1:size(data.spines,2)
%     if (data.spines(i).edittype>0)
%         data.spines(i).label=-1;
%     elseif ((data.spines(i).label>=data.newspinenum) | (data.spines(i).label<=-10) | (data.spines(i).label==0))
%         data.spines(i).label=currentlabel;
%         currentlabel=currentlabel+1;
%     end
% end
% data.GUI.displayrejectedspine=0;
% displayImage(guidata(seriesdata.currentimage),data);
% displayProjectionImage(guidata(seriesdata.currentimage),data);
% setappdata(seriesdata.currentimage,'data',data);
% 
% % --------------------------------------------------------------------
% function varargout = reorderspines2_Callback(h, eventdata, handles, varargin)
% seriesdata= getappdata(gcbf,'seriesdata');
% data=getappdata(seriesdata.previmage,'data');
% spineind=[];
% for i=1:size(data.dendrites,2)
%     loc=find([data.spines.den_ind]==i);
%     [A,B]=sort([data.spines(loc).dendis]);
%     spineind=[spineind loc(B)];
% end
% data.spines=data.spines(spineind);
% currentlabel=data.newspinenum;
% for i=1:size(data.spines,2)
%     if (data.spines(i).edittype>0)
%         data.spines(i).label=-1;
%     elseif ((data.spines(i).label>=data.newspinenum) | (data.spines(i).label<=-10))
%         data.spines(i).label=currentlabel;
%         currentlabel=currentlabel+1;
%     end
% end
% data.GUI.displayrejectedspine=0;
% displayImage(guidata(seriesdata.previmage),data);
% displayProjectionImage(guidata(seriesdata.previmage),data);
% setappdata(seriesdata.previmage,'data',data);
% 
% % --------------------------------------------------------------------
% function varargout = flipdendrite_Callback(h, eventdata, handles, varargin)
% seriesdata= getappdata(gcbf,'seriesdata');
% data=getappdata(seriesdata.currentimage,'data');
% numstr=char(get(handles.inputtext,'String'));
% rem=numstr;
% labels=[data.spines.label];
% while(rem)
% [token,rem]=strtok(rem);
% num=str2double(token);
% if (data.spines(find(labels==num)).den_ind)
%     dendrite=data.dendrites(data.spines(find(labels==num)).den_ind);
%     start=dendrite.start;
%     dendrite.start=dendrite.end;
%     dendrite.end=start;
%     dendrite.voxel=fliplr(dendrite.voxel);
%     denlen=size(dendrite.voxel,2);
%     spinelocs=find([data.spines.den_ind]==dendrite.index);
%     for j=1:size(spinelocs,2)
%         data.spines(spinelocs(j)).dendis=denlen+1-data.spines(spinelocs(j)).dendis;
%     end
%     data.dendrites(data.spines(find(labels==num)).den_ind)=dendrite;
% end
% end
% spineind=[];
% for i=1:size(data.dendrites,2)
%     loc=find([data.spines.den_ind]==i);
%     [A,B]=sort([data.spines(loc).dendis]);
%     spineind=[spineind loc(B)];
% end
% data.spines=data.spines(spineind);
% 
% data.GUI.displayrejectedspine=0;
% displayImage(guidata(seriesdata.currentimage),data);
% displayProjectionImage(guidata(seriesdata.currentimage),data);
% setappdata(seriesdata.currentimage,'data',data);
% 
% 
% 
% % --------------------------------------------------------------------
% function varargout = flipdendrite2_Callback(h, eventdata, handles, varargin)
% seriesdata= getappdata(gcbf,'seriesdata');
% data=getappdata(seriesdata.previmage,'data');
% numstr=char(get(handles.inputtext,'String'));
% rem=numstr;
% labels=[data.spines.label];
% while(rem)
% [token,rem]=strtok(rem);
% num=str2double(token);
% if (data.spines(find(labels==num)).den_ind)
%     dendrite=data.dendrites(data.spines(find(labels==num)).den_ind);
%     start=dendrite.start;
%     dendrite.start=dendrite.end;
%     dendrite.end=start;
%     dendrite.voxel=fliplr(dendrite.voxel);
%     denlen=size(dendrite.voxel,2);
%     spinelocs=find([data.spines.den_ind]==dendrite.index);
%     for j=1:size(spinelocs,2)
%         data.spines(spinelocs(j)).dendis=denlen+1-data.spines(spinelocs(j)).dendis;
%     end
% end
% end
% spineind=[];
% for i=1:size(data.dendrites,2)
%     loc=find([data.spines.den_ind]==i);
%     [A,B]=sort([data.spines(loc).dendis]);
%     spineind=[spineind loc(B)];
% end
% data.spines=data.spines(spineind);
% 
% data.GUI.displayrejectedspine=0;
% displayImage(guidata(seriesdata.previmage),data);
% displayProjectionImage(guidata(seriesdata.previmage),data);
% setappdata(seriesdata.previmage,'data',data);
% 
% 
% % --------------------------------------------------------------------
% function varargout = breakdendrite_Callback(h, eventdata, handles, varargin)
% seriesdata= getappdata(gcbf,'seriesdata');
% data=getappdata(seriesdata.currentimage,'data');
% numstr=char(get(handles.inputtext,'String'));
% rem=numstr;
% labels=[data.spines.label];
% while(rem)
% [token,rem]=strtok(rem);
% num=str2double(token);
% [first,second]=strtok(token,'-');
% numfirst=str2double(first);
% numsecond=-str2double(second);
% labels=[data.spines.label];
% den=data.spines(find(labels==numfirst)).den_ind;
% if (den)
%     breakind=(data.spines(find(labels==numsecond)).dendis+data.spines(find(labels==numfirst)).dendis)/2;
%     newden=size(data.dendrites,2)+1;
%     data.dendrites(newden).voxel=data.dendrites(den).voxel(:,floor(breakind)+1:end);
%     data.dendrites(newden).index=newden;
%     data.dendrites(newden).start=data.dendrites(den).voxel(:,floor(breakind)+1);
%     data.dendrites(newden).end=data.dendrites(den).end;
%     data.dendrites(den).voxel=data.dendrites(den).voxel(:,1:floor(breakind));
%     data.dendrites(den).end=data.dendrites(den).voxel(:,floor(breakind));
%     for i=1:size(data.spines,2)
%         if ((data.spines(i).den_ind==den) & (data.spines(i).dendis>breakind))
%             data.spines(i).den_ind=newden;
%             data.spines(i).dendis=data.spines(i).dendis-breakind;
%         end
%     end
%     for i=1:size(data.dendrites,2)
%         data.dendrites(i).length=size(data.dendrites(i).voxel,2);
%     end
%     [len,order]=sort([data.dendrites.length]);
%     order=flipdim(order,2);
%     data.dendrites=data.dendrites(order);
%     
%     
%     for i=1:size(data.spines,2)
%         data.spines(i).den_ind=find(order==data.spines(i).den_ind);   
%     end
% end
% end
% data.GUI.displayrejectedspine=0;
% displayImage(guidata(seriesdata.currentimage),data);
% displayProjectionImage(guidata(seriesdata.currentimage),data);
% setappdata(seriesdata.currentimage,'data',data);
% 
% 
% % --------------------------------------------------------------------
% function varargout = generateprojection_Callback(h, eventdata, handles, varargin)
% seriesdata= getappdata(gcbf,'seriesdata');
% filenames=get(handles.listbox,'String');
% pathname=seriesdata.pathname;
% for day=1:size(filenames,1)
%     [path,name,ext] = fileparts([pathname char(filenames(day))]);
%     if (strcmp(ext,'.spi'))
%         try
%             load('-mat',[path '\' name ext]);
%         catch
%             print('Could not open data');
%             continue;
%         end
% 
%         %extract intensity information
%         
%         axonnumber=1
% voxel=double(data.dendrites(axonnumber).voxel);
% maxx=size(data.filteredArray,2);
% maxy=size(data.filteredArray,1);
% maxz=size(data.filteredArray,3);
% %index=voxel(1,:)+(voxel(2,:)-1)*maxy+(voxel(3,:)-1)*maxy*maxx;
% %calculate the instantaneous slope in X-Y plane use neighborhood of 3 pixels to each side
% sizeofaxon=size(data.dendrites(axonnumber).voxel,2);
% 
% %calculate axon length
% locy=double(data.dendrites(axonnumber).voxel(1,1:10:size(data.dendrites(axonnumber).voxel(1,:),2)))*0.083;
% locx=double(data.dendrites(axonnumber).voxel(2,1:10:size(data.dendrites(axonnumber).voxel(2,:),2)))*0.083;
% length=sum(sqrt(diff(locx).*diff(locx)+diff(locy).*diff(locy)));
% 
% dy=([voxel(1,1) voxel(1,1) voxel(1,1) voxel(1,1:sizeofaxon-3)]-[voxel(1,4:sizeofaxon) voxel(1,sizeofaxon) voxel(1,sizeofaxon) voxel(1,sizeofaxon)]);
% dx=([voxel(2,1) voxel(2,1) voxel(2,1) voxel(2,1:sizeofaxon-3)]-[voxel(2,4:sizeofaxon) voxel(2,sizeofaxon) voxel(2,sizeofaxon) voxel(2,sizeofaxon)]);
% maxint=zeros(1,sizeofaxon);
% length=sqrt(dx.*dx+dy.*dy);
% %change length of 0 to 1 to avoid division by zero
% length(find(length==0))=1;
% dx=dx./length;
% dy=dy./length;
% for i=1:5 %crosssection of 3 on each side
%     crossdx{i}=round((i-3).*dx);
%     crossdy{i}=round((i-3).*dy);
%     for j=-1:1
%         x=min(max(voxel(2,:)+crossdy{i},0),maxx-1);
%         y=min(max(voxel(1,:)+crossdx{i},0),maxy-1);
%         z=min(max(voxel(3,:)+j,0),maxz-1);
%         maxint=max(maxint,data.filteredArray(y+x*maxy+z*maxy*maxx));    
%     end
% end
% thres=min(maxint(threshold(maxint,'background')));
% meanback=mean(maxint(threshold(maxint,'background')==0));
%     
%     %generate projection
%         proj=collapse(data.filteredArray(:,:,1:size(data.imageArray,3)),'XY');
%     %normalize the intensity
%         proj=uint16(double(proj)*100/meanback);
%         arrayToTiff(proj, [path '\' name 'proj.tif'], '');
%     end
% end
% setappdata(gcbf,'seriesdata',seriesdata);
% 
% % --------------------------------------------------------------------
% 
% 
% % --- Executes on button press in twosides.
% function twosides_Callback(hObject, eventdata, handles)
% % hObject    handle to twosides (see GCBO)
% % eventdata  reserved - to be defined in a future version of MATLAB
% % handles    structure with handles and user data (see GUIDATA)
% 
% % Hint: get(hObject,'Value') returns toggle state of twosides
% 
% 
