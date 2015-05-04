
function out=findTerminals(y, th, n, maxn)
% function to find bouton (synaptic terminal) locations
% INPUTS: y - vector of intesities along the axonal contour
%         th - fractional intensity elevation required to score as axon (i.e. 0.2 means 20%)
%         n  - 'size' of a terminal (say 10 for pixels ize 0.080)
%

%conv=3;
conv=maxn/n;
convCorr=4;

termInd=[];

% find local maxima, make sure they are not too close to the edge
maxInd=find(imregionalmax(y));
ind=1;
while maxInd(ind) < n+1; ind=ind+1; end
maxInd=maxInd(ind:end);

minInd=find(imregionalmax(-y));

% treat the ends differently

m1=y(max(minInd(find(minInd < maxInd(1)))));
m2=y(min(minInd(find(minInd > maxInd(1)))));
if m1 < -th+y(maxInd(1)) & m2 < -th+y(maxInd(1)); termInd=[termInd, maxInd(1)]; end

m1=y(max(minInd(find(minInd < maxInd(length(maxInd))))));
m2=y(min(minInd(find(minInd > maxInd(length(maxInd))))));
if m1 < -th+y(maxInd(length(maxInd))) & m2 < -th+y(maxInd(length(maxInd))); termInd=[termInd, maxInd(length(maxInd))]; end

for i=2:(length(maxInd)-1)
    % find the closest mins and maxes
    ind1=max(minInd(find(minInd < maxInd(i))));
    ind2=min(minInd(find(minInd > maxInd(i))));
    m11=y(ind1);
    if maxInd(i)-fix(conv*n/2) > 1
        m12=min(y((maxInd(i)-fix(conv*n/2)):maxInd(i)));
    else
        m12=min(y(1:maxInd(i)));
    end
    m21=y(ind2);
    if maxInd(i)+fix(conv*n/2) < length(y)
        m22=min(y(maxInd(i):(maxInd(i)+fix(conv*n/2))));            
    else
        m22=min(y(maxInd(i):end));
    end
    
    m1=max([m11 m12]);
    m2=max([m21 m22]);
    
    switch true
        % well-separated peak
        
       % case m1 < -th+y(maxInd(i)) & m2 < -th+y(maxInd(i))
        case m1 < -th+y(maxInd(i)) & m2 < -th+y(maxInd(i))
            termInd=[termInd, maxInd(i)];
        
        % closely spaced double peak
        case maxInd(i+1)-maxInd(i) < conv*n
            m3=y(min(minInd(find(minInd > maxInd(i+1)))));
            % peakDiff=2*abs(y(maxInd(i))-y(maxInd(i+1)))/(y(maxInd(i))+y(maxInd(i+1)));
            if (m1 < -th+y(maxInd(i)) & m3 < (1-th/convCorr)*y(maxInd(i+1))) & (m3 < -th+y(maxInd(i+1))) % & peakDiff < th
                termInd=[termInd, maxInd(i), maxInd(i+1)];
            end
            
            
            
        otherwise
            termInd
    end
    
end
    
    out=termInd;