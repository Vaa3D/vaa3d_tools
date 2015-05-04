function generatefiles(db)
root=pwd;
for i=1:length(db.experiments)
    name=db.experiments(i).rawImageName;
    [start_idx, end_idx, extents, matches, tokens, names, splits]=regexp(name,'(\w+)([a-z])([0-9]*)');
    t=tokens{1};
    cd(root);
    try
        mkdir(t{1});
    catch
    end
    cd(t{1});
    try 
        mkdir([t{1} '-' t{3}]);
    catch
    end
    cd([t{1} '-' t{3}]);
    try
        mkdir([t{1} t{2} t{3} 'ziptemp']);
    catch
    end
    cd([t{1} t{2} t{3} 'ziptemp']);
    self=generatedat(db,i);
    save([t{1} t{2} t{3} '.dat'],'self');
end
for i=1:length(db.experiments)
    name=db.experiments(i).rawImageName;
    [start_idx, end_idx, extents, matches, tokens, names, splits]=regexp(name,'(\w+)([a-z])([0-9]*)');
    t=tokens{1};
    cd(root);
    try
        mkdir(t{1});
    catch
    end
    cd(t{1});
    try 
        mkdir([t{1} '-' t{3}]);
    catch
    end
    cd([t{1} '-' t{3}]);
    try
        mkdir([t{1} t{2} t{3} 'ziptemp']);
    catch
    end
    cd([t{1} t{2} t{3} 'ziptemp']);
    % try to find the tif file
    r=rdir(['f:\axons\data\**\' t{1} t{2} t{3} '.tif']);
    if length(r)>0
        disp(r(1).name)
        copyfile(r(1).name,'.');
        cd ..;
        zip([t{1} t{2} t{3}],[t{1} t{2} t{3} 'ziptemp']);
        movefile([t{1} t{2} t{3} '.zip'],[t{1} t{2} t{3} '.spi']);
    else
        r=rdir(['f:\axons\data\**\' t{1} t{2} t{3} '.spi']);
        if length(r)>0
            disp(r(1).name)
            unzip(r(1).name,[r(1).name(1:end-4) 'ziptemp']);
            copyfile([r(1).name(1:end-4) 'ziptemp/' t{1} t{2} t{3} '.tif'],'.');
            cd ..;
            zip([t{1} t{2} t{3}],[t{1} t{2} t{3} 'ziptemp']);
            movefile([t{1} t{2} t{3} '.zip'],[t{1} t{2} t{3} '.spi']);
        else
            disp (['not found'  t{1} t{2} t{3} '.tif'])
        end
    end
end
cd(root)