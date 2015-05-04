function obj=load(obj)
[fname, pname] =  uigetfile({'*.spi'}, 'Choose analysis file to load');
obj=loadImageByNameGUI(obj,[pname,fname]);