
function [out, Rel, Abs]=findTerminals(y, thRel, thAbs, maxwidth,minwidth)
% function to find boutons (synaptic terminals) 
% INPUTS: y - vector of intesities along the axonal contour; units are
%         backbone intensities
%         thRel - fractional intensity elevation required to score as bouton (i.e. 0.2 means 20%)
%         width - expected minimum width of a terminal, approximately 2 psfs wide (say 10 for pixels size 0.080)
%         thAbs - minimum allowed absolute intensity of varicosity in backbone units (e.g. 1.0);
%         backbone is the intensity minus the medianfiltered intensity
%

%conv=3;         % double peak within 3 psf
dip=2;          % correcting the threshold; relaxing the thRel facro by a factor of dip

termInd=[]; % the array containing the indeces of the terminals
Rel=[];
Abs=[];

% find local maxima
maxInd=find(imregionalmax(y));


% if maxima are closer than width, use only one

% throw away small maxima; protect against noise
maxInd=maxInd(find(y(maxInd)>thAbs));

% find local maxima
minInd=find(imregionalmax(-y));

if isempty(maxInd)
    out=[];
    
    return;
end

% on the left edge, throw away maxima that are too close to the edge
%ind=1;
%while (ind<size(maxInd,1)) & maxInd(ind) < fix(minwidth+1); ind=ind+1; end
maxInd(maxInd<fix(minwidth+1))=[];

%now flip it around; throw away fringe stuff on the right
y=fliplr(y);
maxInd=fliplr(maxInd);
% ind=1;
% while maxInd(ind) < fix(minwidth+1); ind=ind+1; end
% maxInd=maxInd(ind:end);
maxInd(maxInd<fix(minwidth+1))=[];

y=fliplr(y);
maxInd=fliplr(maxInd);
%plot(maxInd, y(maxInd), 'ro');


ind=find((maxInd(2:end)-maxInd(1:(end-1)) < minwidth));
while ~ isempty(ind)
    for i=1:length(ind)
        if maxInd(ind(i)) > 0
            if y(maxInd(ind(i))) < y(maxInd(ind(i)+1))
                maxInd(ind(i))=0;
            else
                maxInd(ind(i)+1)=0;
            end
        end
    end
    maxInd=maxInd(find(maxInd>0));
    ind=find((maxInd(2:end)-maxInd(1:(end-1)) < minwidth));
end

%plot(maxInd, y(maxInd), 'mx');


for i=1:(length(maxInd))
    % find the closest mins and maxes
    minLeft=max(minInd(find(minInd < maxInd(i))));
    minRight=min(minInd(find(minInd > maxInd(i))));
    yLeftMin=y(minLeft);
    % now look a bit further -- to allow for double peaks
    if maxInd(i)-maxwidth > 1
        yLeftMin2=min(y((maxInd(i)-maxwidth):maxInd(i)));
    else
        yLeftMin2=min(y(1:maxInd(i)));
    end
    
    yRightMin=y(minRight);
    if maxInd(i)+maxwidth < length(y)
        yRightMin2=min(y(maxInd(i):(maxInd(i)+maxwidth)));            
    else
        yRightMin2=min(y(maxInd(i):end));
    end
    
    yLeft=min([yLeftMin yLeftMin2]);
    yRight=min([yRightMin yRightMin2]);
    
    % logical conditions
    thLeft= (yLeft < (1-thRel)*y(maxInd(i)));
    thRight= (yRight < (1-thRel)*y(maxInd(i)));
    dipLeft= (yLeft < (1-thRel/dip)*y(maxInd(i)));
    dipRight= (yRight < (1-thRel/dip)*y(maxInd(i)));
    
    if ((yLeft < (1-thRel)*y(maxInd(i)) | yRight < (1-thRel)*y(maxInd(i))))
        try
        switch true
            % well-separated peak   
            case thLeft & thRight
                termInd=[termInd, maxInd(i)];
                Rel=[Rel, y(maxInd(i))-min(yLeft,yRight)];
                Abs=[Abs, maxInd(i)];
                % double peak right
            case thLeft & dipRight
                termInd=[termInd, -maxInd(i)];
                Rel=[Rel, y(maxInd(i))-min(yLeft,yRight)];
                Abs=[Abs,maxInd(i)];
                % double peak left
            case thRight & dipLeft
                if (isempty(termInd)) || (abs(termInd(end))>(maxInd(i)-maxwidth*1.5))                                  % there is a real peak to the left
                    termInd=[termInd, maxInd(i)];
                end
                Rel=[Rel, y(maxInd(i))-min(yLeft,yRight)];
                Abs=[Abs,maxInd(i)];
            otherwise
%                disp('throwing out a peak');
%                maxInd(i)
        end
    catch
        i
    end
    end
end

for i=1:max(size(termInd))
    if ((termInd(i)<0) && ((i==max(size(termInd))) || (abs(termInd(i+1))<(termInd(i)+maxwidth*1.5))))               % there is a real peak to the right
        termInd(i)=-termInd(i);
    end
end


% merge "double peaks" which are within maxdistance and has tiny dip in
% between

for i=1:max(size(termInd))-1
    if (termInd(i)>0) && (abs(termInd(i+1)-termInd(i))<maxwidth)
        % if the dip in between big enough?
        minBetween=min(y(minInd((minInd>termInd(i)&(minInd < termInd(i+1))))));
        if (minBetween> (1-thRel/dip)*y(termInd(i))) | (minBetween > (1-thRel/dip)*y(termInd(i+1)))
            if y(termInd(i))>y(termInd(i+1))
                termInd(i+1)=-termInd(i+1);
            else
                termInd(i)=-termInd(i);
            end
        end
    end
end
termInd=termInd(termInd>0);

%plot(termInd, y(termInd), 'rx');
out=termInd;