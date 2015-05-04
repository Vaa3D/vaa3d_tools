function [pks,locs] = findpeaks(X,varargin)
%FINDPEAKS Find local peaks in data
%   PKS = FINDPEAKS(X) finds local peaks in data set X where is a
%   vector
%
%   [PKS,LOCS]= FINDPEAKS(X) also returns the indices LOCS at which the
%   peaks occur.
%
%   [...] = FINDPEAKS(X,'MINPEAKHEIGHT',MPH) finds only those peaks that
%   are greater than MINPEAKHEIGHT MPH. Specifying a minimum peak height
%   may help in reducing the processing time. MPH is a real valued scalar.
%   The default value of MPH is -Inf.
%
%   [...] = FINDPEAKS(X,'MINPEAKDISTANCE',MPD) finds peaks that are at
%   least separated by MINPEAKDISTANCE MPD. MPD is a positive integer
%   valued scalar. This parameter may be specified to ignore smaller peaks
%   that may occur in close proximity to a large local peak. For example,
%   if a large local peak occurs at index N, then all smaller peaks in the
%   range (N-MPD, N+MPD) are ignored. If not specified, MPD is assigned a
%   value of one.
%
%   [...] = FINDPEAKS(X,'THRESHOLD',TH)finds peaks that are at least
%   greater than their neighbhors by the THRESHOLD TH. TH is real valued
%   scalar greater than or equal to zero. The default value of TH is zero.
%
%   [...] = FINDPEAKS(X,'NPEAKS',NP) specifies the number of peaks to be
%   found. NP is an integer greater than zero. When specified, the
%   procedure terminates once NP peaks are found and the NP peaks are
%   returned. If not specified, all peaks are returned.
%
%   [...] = FINDPEAKS(X,'SORTSTR',STR) specifies the direction of sorting
%   of peaks. STR can take values of 'ascend','descend' or 'none'. If not
%   specified, STR takes the value of 'none' and the peaks are returned in
%   the order of their occurrence.
%
%   See also DSPDATA/FINDPEAKS

%   Copyright 2007 The MathWorks, Inc.
%   $Revision: 1.3 $  $Date: 2011-03-09 10:20:23 $


Ph  = -inf;
Pd  = [];
Th  = 0;
Np  = [];
Str = 'none';

pks = [];
locs = [];


M = length(X);

if (M == 0)
    datamsgid = generatemsgid('emptyDataSet');
    error(datamsgid,'Data set cannot be empty.');
elseif M<3,
    pks = [];
elseif Ph == -Inf
    Data = X;
    L = M;
    % validate value of Pd and set default values for Pd and Np
    [Pd,Np] = setvalues(Pd,Np,L);
    if(Pd >=L)
        pdmsgid = generatemsgid('largeMinPeakDistance');
        error(pdmsgid,strcat('Invalid MinPeakDistance. Set MinPeakDistance in the range (',...
            num2str(0), ',', num2str(L), ').'));
    else
        % call getpeaks_contindx to reduce execution time.
        [pks,locs] = getpeaks_contindx(Data,Pd,Th,Np);
    end
else
    Indx = find(X > Ph);
    Data = X(Indx);
    L = length(Data);
    if(L==0)
        mphmsgid = generatemsgid('largeMinPeakHeight');
        warning(mphmsgid,'Invalid MinPeakHeight. There are no data points greater than MinPeakHeight.');
    elseif L<3,
        pks = [];
    else
        % validate value of Pd and set default values for Pd and Np
        [Pd,Np] = setvalues(Pd,Np,L);
        if(Pd >=Indx(L))
            pdmsgid = generatemsgid('largeMinPeakDistance');
            error(pdmsgid,strcat('Invalid MinPeakDistance. Set MinPeakDistance in the range (',...
                num2str(0), ',', num2str(Indx(L)), ').'));
        else
            [pks,locs] =getpeaks(Data,Indx,Pd,Th,Np,M);
        end
    end
end

if isempty(pks)
 %   npmsgid = generatemsgid('noPeaks');
 %   warning(npmsgid,'No peaks found.')
elseif(~strcmp(Str,'none'))
    [pks,s]  = sort(pks,Str);
    if(~isempty(locs))
        locs = locs(s);
    end
end

%--------------------------------------------------------------------------
function [Pd,Np] = setvalues(Pd,Np,L)

if ~isempty(Pd) && (~isnumeric(Pd) || ~isscalar(Pd) ||any(rem(Pd,1)) || (Pd < 1))
    Nmsgid = generatemsgid('invalidMinPeakDistance');
    error(Nmsgid,'MinPeakDistance should be an integer greater than 0.');
end

if(isempty(Pd))
    Pd = 1;
end

if(isempty(Np))
    Np = L;
end

%--------------------------------------------------------------------------
function [pks,locs] =getpeaks(Data,Indx,Pd,Th,Np,M)
% This function finds peaks in data set Data whose index set Indx is
% disjoint. Some data points were removed from the original set through
% preprocessing

m = 0;                  % counter for peaks found
L = length(Indx);

% Pre-allocate for speed
pks  = zeros(1,Np);
locs = zeros(1,Np);

endindx = Indx(L);      % last point in the index set

j = 0;
% First, the "Pd" neighbors, on either side, of the current data point are
% found. Then the current data point is compared with these neighbors to
% determine whether it is a peak.

while (j < L) && (m < Np)
    j = j+1;
    
    % leftmost neighbor index
    endL = max(1,Indx(j) - Pd);
    
    % Update the leftmost neighbor index if there is a peak within "Pd"
    % neighbors of leftmost index
    if(m > 0)
        x = min([locs(m)+ Pd  endindx-1]);
        if(Indx(j)<=x)
            k = find(Indx > x ,1,'first');
            j = k;
            endL = Indx(j)- Pd;
        end
    end
    
    % rightmost neighbor index
    endR = min(Indx(j) + Pd,endindx);
    
    % Find nearest points to leftmost and rightmost neighbor indices. This
    % is necessary because index set Indx may be disjoint.
    a =  find(endL<=Indx,1,'first');
    b =  find(endR>=Indx,1,'last');
    
    % create neighbor data set
    temp = Data(a:b);
    
    % set current data point to -Inf in the neighbor data set
    aa = find(Indx(j) == Indx(a:b));
    temp(aa) = -Inf;
    
    % Compare current data point with all neighbors
    if(all((Data(j) > temp+Th)) && (Indx(j) ~=1)&& (Indx(j)~=M))
        m = m+1;
        pks(m)  = Data(j);  % peaks
        locs(m) = Indx(j);  % loctions of peaks
    end
end

% return all peaks found
if m~=0
    locs = locs(locs > 0);
    pks  = pks(1:length(locs));
else
    locs = [];
    pks = [];
end

% %--------------------------------------------------------------------------
function [pks,locs] =getpeaks_contindx(Data,Pd,Th,Np)
% This function finds peaks in data set Data whose index set is not
% disjoint. No data points were removed from the original set through
% preprocessing. Calling this function instead of getpeaks reduces
% execution time.

m = 0;                  % counter for peaks found
L = length(Data);

j = 0;

% Pre-allocate for speed
pks  = zeros(1,Np);
locs = zeros(1,Np);

% First, the "Pd" neighbors, on either side, of the current data point are
% found. Then the current data point is compared with these "Pd" neighbors
% to determine whether it is a peak.
while (j < L) && (m < Np)
    j =j+1;
    
    % leftmost neighbor index
    endL = max(1,j- Pd);
    
    % Update the leftmost neighbor index if there is a peak within
    % "Pd" neighbors of leftmost index
    if(m > 0)
        if (j < min([locs(m)+Pd  L-1]));
            j = min([locs(m)+Pd  L-1]);
            endL = j-Pd;
        end
    end
    
    % rightmost neighbor index
    endR = min(L,j+ Pd);
    
    % create neighbor data set
    temp = Data(endL:endR);
    
    % set current data point to -Inf in the neighbor data set
    aa = find(j == endL:endR);
    temp(aa) = -Inf;
    
    % Compare current data point with all neighbors
    if(all((Data(j) > temp+Th)) && (j ~=1) && j~=L)
        m = m+1;
        pks(m)  = Data(j);
        locs(m) = j;
    end
end

% return all peaks found
if m~=0
    pks  = pks(1:m);
    locs = locs(1:m);
else
    pks  = [];
    locs = [];
end


% [EOF]
