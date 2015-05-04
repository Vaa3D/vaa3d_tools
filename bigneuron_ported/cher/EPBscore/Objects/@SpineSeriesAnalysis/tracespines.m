function obj=tracespines(obj)
global self;
global objects;
if (obj.state.firstimage) & (obj.state.secondimage)
    firststr=getappdata(obj.state.firstimage,'object');
    firststruct=struct(eval(firststr));
    secondstr=getappdata(obj.state.secondimage,'object');
    secondstruct=struct(eval(secondstr));
    if ~isfield(firststruct.data,'maxserieslabel')
        firstobj=eval(firststr);
        firstobj=set(firstobj,'data.maxserieslabel',max([firststruct.data.spines.label]));
        eval([firststr '=firstobj;']);
        firststruct=struct(firstobj);
    end
    maxserieslabel=firststruct.data.maxserieslabel;
    % check to make sure the maxserieslabel is at least bigger than the
    % maximal on that particular day
    
    if (maxserieslabel<max([firststruct.data.spines.label]))
        firstobj=eval(firststr);
        firstobj=set(firstobj,'data.maxserieslabel',max([firststruct.data.spines.label]));
        eval([firststr '=firstobj;']);
        firststruct=struct(firstobj);
        maxserieslabel=firststruct.data.maxserieslabel; 
    end 
    try
    secondobj=eval(secondstr);    
    for j=1:size(firststruct.data.dendrites,2)
        firstdendrite=firststruct.data.dendrites(j);
        seconddendrite=secondstruct.data.dendrites([secondstruct.data.dendrites.index]==firstdendrite.index);
        if isfield(firststruct.data,'spines') & isfield(firstdendrite,'offset') & isfield(firstdendrite,'stretch') ... 
           & isfield(secondstruct.data,'spines') & isfield(seconddendrite,'offset') & isfield(seconddendrite,'stretch')
            firstlabelenpassant=[firststruct.data.spines(([firststruct.data.spines.len]==0)&([firststruct.data.spines.den_ind]==j)).label];
            firstlabelterminal=[firststruct.data.spines(([firststruct.data.spines.len]>0)&([firststruct.data.spines.den_ind]==j)).label];
            firstdisenpassant=firststruct.data.dendrites(j).length([firststruct.data.spines(([firststruct.data.spines.len]==0)&([firststruct.data.spines.den_ind]==j)).dendis]);
            firstdisterminal=firststruct.data.dendrites(j).length([firststruct.data.spines(([firststruct.data.spines.len]>0)&([firststruct.data.spines.den_ind]==j)).dendis]);
            secondspines=find([secondstruct.data.spines.den_ind]==j);
            % trace en passant and terminal separately
            for i=1:size(secondspines,2)
                % trace en passant and terminal separately
                dis=100;
                if (secondstruct.data.spines(secondspines(i)).len==0) && (size(firstdisenpassant,2)>0)           
                    [dis,ind]=min(abs((firstdisenpassant-firstdendrite.offset)/firstdendrite.stretch-(secondstruct.data.dendrites(j).length(secondstruct.data.spines(secondspines(i)).dendis)-seconddendrite.offset)/seconddendrite.stretch));
                    firstind=find([firststruct.data.spines.label]==firstlabelenpassant(ind));
                    firstind=firstind(1);
                end
                if (secondstruct.data.spines(secondspines(i)).len>0) && (size(firstdisterminal,2)>0)           
                    [dis,ind]=min(abs((firstdisterminal-firstdendrite.offset)/firstdendrite.stretch-(secondstruct.data.dendrites(j).length(secondstruct.data.spines(secondspines(i)).dendis)-seconddendrite.offset)/seconddendrite.stretch));
                    firstind=find([firststruct.data.spines.label]==firstlabelterminal(ind));
                    firstind=firstind(1);
                end    
                
                if (dis<1.5)
                    secondobj=set(secondobj,['data.spines(' num2str(secondspines(i)) ').label'],firststruct.data.spines(firstind).label);
                else
                    secondobj=set(secondobj,['data.spines(' num2str(secondspines(i)) ').label'],maxserieslabel+1);
                    maxserieslabel=maxserieslabel+1;
                end
            end
        end        
    end
catch
    i
    j
end
    secondobj=set(secondobj,'data.maxserieslabel',maxserieslabel);
    eval([secondstr '=secondobj;']);
    displayProfile(secondobj);
    displayImage(secondobj);
    displayProjectionImage(secondobj);
end