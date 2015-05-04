function obj=openImageByNameGUI(obj,filename,redfilename)
% get the name of the image
[path,name,ext] = fileparts(filename);
imageArray=[];
try
   if exist('redfilename','var')
        obj=openImageByName(obj,filename,redfilename);
   else
        obj=openImageByName(obj,filename);
   end   
catch
    display('Could not open image');
    return;
end

% close all windows
if (isfield(obj.gh.projectionGUI,'Figure'))
    close(obj.gh.projectionGUI.Figure)
end
obj.gh.projectionGUI=[];    
if (isfield(obj.gh.imageGUI,'Figure'))
    close(obj.gh.imageGUI.Figure)
end
obj.gh.imageGUI=[];    
if (isfield(obj.gh.parametersGUI,'Figure'))
    close(obj.gh.parametersGUI.Figure)
end
obj.gh.parametersGUI=[];   

% reset the values to initial state
global self;
global ogh;
self=[];
self.ID=obj.ID;
ogh=obj.gh;
initGUIs('standardspineanalysisimage.ini');

% initialize the graphical handles correctly
for field=fieldnames(ogh)'
    self.gh.(field{1})=ogh.(field{1});
end
self.parameters=obj.parameters;
self.state.rawImagePath=obj.state.rawImagePath;
self.state.rawImageName=obj.state.rawImageName;
self.data=obj.data;
% Set Image related properties
self.state.ROI.maxz=obj.state.ROI.maxz;
self.state.ROI.minz=obj.state.ROI.minz;
self.state.display.currentz=obj.state.display.currentz;
self.state.display.lowpixelch1=0;
self.state.display.highpixelch1=300;
self.state.display.ch1=1;

updateGUIbyglobal('self.state.ROI.minz');
updateGUIbyglobal('self.state.ROI.maxz');
updateGUIbyglobal('self.state.display.currentz');
zslider=getGuiOfGlobal('self.state.display.currentz');
zslider=eval(zslider{1});
set(zslider,'Min',self.state.ROI.minz,'Max',self.state.ROI.maxz,'SliderStep',[1/(self.state.ROI.maxz-self.state.ROI.minz) 1/(self.state.ROI.maxz-self.state.ROI.minz)]);
updateGUIbyglobal('self.state.display.lowpixelch1');
updateGUIbyglobal('self.state.display.highpixelch1');
updateGUIbyglobal('self.state.display.ch1');

if exist('redfilename','var')
    self.state.display.lowpixelch2=0;
    self.state.display.highpixelch2=300;
    self.state.display.ch2=1;
    updateGUIbyglobal('self.state.display.lowpixelch2');
    updateGUIbyglobal('self.state.display.highpixelch2');
    updateGUIbyglobal('self.state.display.ch2');
end

set(self.gh.mainGUI.Figure,'name',[name '-' path '\' name]);

self.state.display.showprojection=1;
updateGUIbyglobal('self.state.display.showprojection');
obj=set(obj,'struct',self);             % generic object assigning part
ogh=[];
self=[];
