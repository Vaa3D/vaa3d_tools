function obj=fiducialalign(obj)
global self;
global objects;
if (obj.state.firstimage) & (obj.state.secondimage)
    firststr=getappdata(obj.state.firstimage,'object');
    firststruct=struct(eval(firststr));
    secondstr=getappdata(obj.state.secondimage,'object');
    secondstruct=struct(eval(secondstr));
    for i=1:size(firststruct.data.dendrites,2)
        firstdendrite=firststruct.data.dendrites(i);
        seconddendrite=secondstruct.data.dendrites([secondstruct.data.dendrites.index]==firstdendrite.index);
        if isfield(firststruct.data,'marks') & isfield(firstdendrite,'maxint') & isfield(firstdendrite,'meanback') ... 
           & isfield(secondstruct.data,'marks') & isfield(seconddendrite,'maxint') & isfield(seconddendrite,'meanback')
            firstmarks=firststruct.data.marks([firststruct.data.marks.den_ind]==firstdendrite.index);
            secondmarks=secondstruct.data.marks([secondstruct.data.marks.den_ind]==seconddendrite.index);
            % calculate offset and stretch factor
            offset=mean([secondmarks.dendis])-mean([firstmarks.dendis]);
            stretch=sqrt(var([secondmarks.dendis])/var([firstmarks.dendis]));
            offset=offset-mean([firstmarks.dendis])*(stretch-1);
            
            if ~isfield(firstdendrite,'offset') | isempty(firstdendrite.offset)
                firstobj=eval(firststr);
                firstobj=set(firstobj,['data.dendrites(' num2str(i) ').offset'],0);
                firstobj=set(firstobj,['data.dendrites(' num2str(i) ').stretch'],1);
                eval([firststr '=firstobj;']);
                firststruct=struct(firstobj);
            end 
            secondobj=eval(secondstr);
            secondobj=set(secondobj,['data.dendrites(' num2str(i) ').offset'],firststruct.data.dendrites(i).offset*stretch+offset);
            secondobj=set(secondobj,['data.dendrites(' num2str(i) ').stretch'],firststruct.data.dendrites(i).stretch*stretch);
            eval([secondstr '=secondobj;']); 
            
            
            figure;
            firstmaxint=double(firstdendrite.maxintcorrected);
            secondmaxint=double(seconddendrite.maxintcorrected);
            secondx=((1:size(secondmaxint,2))-offset)./stretch;
            plot(firstmaxint,'b');
            hold on;
            plot(secondx,secondmaxint,'r');
            legend(num2str(double(firstdendrite.meanback)),num2str(double(seconddendrite.meanback)));
            minmaxint=min(min(firstmaxint),min(secondmaxint));
            maxmaxint=max(max(firstmaxint),max(secondmaxint));
            axis([min(0,min(secondx)-1) max(size(firstmaxint,2),max(secondx)+1) minmaxint-(maxmaxint-minmaxint)*0.05 maxmaxint+(maxmaxint-minmaxint)*0.25]);
            firstind=find([firststruct.data.spines.den_ind]==firstdendrite.index);
            firstpeaks=[firststruct.data.spines(firstind).dendis];
            for i=1:size(firstpeaks,2)
                h=text(firstpeaks(i),0.1*(maxmaxint-minmaxint)+firstmaxint(firstpeaks(i)),num2str(firststruct.data.spines(firstind(i)).label));       
                set(h,'Color',[0 0 1]);
            end
            
            
            
            secondind=find([secondstruct.data.spines.den_ind]==seconddendrite.index);
            secondpeaks=[secondstruct.data.spines(secondind).dendis];
            for i=1:size(secondpeaks,2)
                h=text((secondpeaks(i)-offset)/stretch,0.1*(maxmaxint-minmaxint)+secondmaxint(secondpeaks(i)),num2str(secondstruct.data.spines(secondind(i)).label));       
                set(h,'Color',[1 0 0]);
            end
            figure;
            plot(double(firstdendrite.maxintcuberaw),'b');
            hold on;
            plot(secondx,double(seconddendrite.maxintcuberaw),'r');
        end
    end
end

