function obj= seriestrace(obj,pathnames)
global self;
global objects;
global db;  
filenames=obj.state.display.filenames;
if (nargin==1)
    pathnames=obj.state.display.pathnames;
end
pathname=obj.state.pathname;
marks=[];
db=[];

try
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    %       This section get all the data and store them in the
    %       database
    %
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
        seriesuid=1;
    
    % go through each file and get all the data
    for q=1:size(pathnames,2)
        [path name ext]=fileparts(char(pathnames(q)));
        [name,status]=strtok(name,'_');
  
        if (strcmp(ext,'.spi'))
            warning off MATLAB:MKDIR:DirectoryExists;
            mkdir(path, [name 'ziptemp']);
            h = waitbar(0,'Unzipping Spi File...', 'Name', 'Unzipping Spi File', 'Pointer', 'watch');
            waitbar(0,h, ['Unzipping Spi File...']);
            if exist([path '\' name 'ziptemp'],'dir')
                tiffile=dir([path '\' name 'ziptemp\*.tif']);
                filtif=dir([path '\' name 'ziptemp\*.tif']);
                datfile=dir([path '\' name 'ziptemp\*.dat']);
                if ~((size(tiffile,1)>=2) & (size(filtif,1)>=1) & (size(datfile,1)>=1))
                    % infounzip([path '\' name ext],[path '\' name 'ziptemp']);
                    [s,w]=system(['unzip -u ' path '\' name status ext ' -d ' path '\' name 'ziptemp']);
                    if (s~=0)
                        disp('Could not unzip data');
                        close(h);
                        return;
                    end
                end
            else
                %  infounzip([path '\' name ext],[path '\' name 'ziptemp']);
                [s,w]=system(['unzip -u ' path '\' name status ext ' -d ' path '\' name 'ziptemp']);
                if (s~=0)
                    disp('Could not unzip data');
                    close(h);
                    return;
                end
            end
            try
                cd ([path '\' name 'ziptemp\ziptemp']);
                copyfile('*.*','..');
            catch
                
            end
            waitbar(1,h, 'Done');
            close(h);
            
            cd (path);               
            
            datfile=dir([path '\' name 'ziptemp\*.dat']);
            load('-mat',[path '\' name 'ziptemp\' datfile(1).name]);
            
    
            % Add dendrites
          %  for i=1:size(self.data.dendrites,2)
           i=1;
                if isfield(self.data,'marks')
                    % recalculate the dendis of all the marks to be used in
                    % the future for alignment
                    if (size(self.data.marks([self.data.marks.den_ind]==i),2)<2)
                        disp(['ERROR! missing marks from ' name]);
                        return;
                    end
                    marks=[marks; sort(self.data.dendrites(i).length([self.data.marks([self.data.marks.den_ind]==i).dendis]))];
                    
                else
                    disp(['error in ' name ':no marks detected']);
                end
                %     end
            
        end
    end
catch
    disp(['problem with getting data from sereis:' pathnames(q)]);
    disp(lasterr);
end
s=printmatrix(diff(marks'));
clipboard('copy',s)