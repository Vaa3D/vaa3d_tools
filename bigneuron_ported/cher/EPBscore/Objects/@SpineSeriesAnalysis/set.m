function a = set(a,varargin)
% SET Set asset properties and return the updated object
property_argin = varargin;
while length(property_argin) >= 2,
    prop = property_argin{1};
    val = property_argin{2};
    property_argin = property_argin(3:end);
    % a little kluge
    if (strcmp(prop,'struct'))
        a=val;
        a = class(a,'SpineSeriesAnalysis');
    else
        if isnumeric(val)     
            eval(['a.' prop '=' num2str(val) ';']);
        else
            eval(['a.' prop '=''' val ''';']);
        end
    end
end