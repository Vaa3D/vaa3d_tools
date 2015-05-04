function ret=printmatrix(content,varargin)
if isempty(varargin)
    dec=2;
    ff=ceil(log10(max(max(abs(content)))))+dec+3;
    ret=[];
    for i=1:size(content,1)
%         fprintf(['%#',num2str(ff),'.',num2str(dec),'f\t'],content(i,:));
%         fprintf('\n');
        ret=[ret strrep(sprintf(['%#',num2str(ff),'.',num2str(dec),'f\t'],content(i,:)),'NaN','') sprintf('\n')];
    end
else
    header=varargin{1};
    dec=2;
    ff=ceil(log10(max(max(abs(content)))))+dec+3;
    ret=[];
    for i=1:size(content,1)
%         fprintf(['%s\t'],header{i});
%         fprintf(['%#',num2str(ff),'.',num2str(dec),'f\t'],content(i,:));
%         fprintf('\n');
        ret=[ret sprintf(['%s\t'],header{i}) strrep(sprintf(['%#',num2str(ff),'.',num2str(dec),'f\t'],content(i,:)),'NaN','') sprintf('\n')];
    end
end

%ret=sprintf(['%#',num2str(ff),'.',num2str(dec),'f\t'],content);