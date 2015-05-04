function listdbcontent(imagedir,db)
for i=1:size(db.series,2)
            % gather relevant image files
            %concat the names
            names=[];
            for j=1:size(db.series(i).names,2)
                names=[names ' ' db.series(i).names{j}];
            end
            display(['processing    ' num2str(i) '  ' names]);

            for j=1:size(db.series(i).names,2)
                [path,bytes,name]=dirr(imagedir, [db.series(i).names{j} '\.tif\>'],'name');
                if length(name)<1
                    display(['image not found :' db.series(i).names{j}]);
                    if j==1
                        break;
                    end
                end                
            end
end

% verfiy the db has the right uid correspondence
for i=1:size(db.dendrites,2)
    if db.dendrites(i).uid~=i
        display(i);
    end
end

for i=1:size(db.series,2)
    if db.series(i).uid~=i
        display(i);
    end
end

for i=1:size(db.spines,2)
    if db.spines(i).uid~=i
        display(i);
    end
end