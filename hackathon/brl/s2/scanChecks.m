%%  checking PV scan parameter relationships
function out = scanChecks(xPixels, linePeriod, dwellTime, mode, piezoDelay, nSlices)

yPixels = xPixels;
dwellTime*xPixels;
%  5.7ms
%  0.9205  percent duty cycle.
%  lost time:
lost = linePeriod-dwellTime*xPixels ; % = 0.5ms per line lost. where to?
% front porch and backporch
fp = 20*10^-3;
bp = 15*10^-3;
porches = fp+bp;
%  that's still only 35us.  
lostnoporches = lost-porches;
%  NO BIDIRECTIONAL SCANNING!   so roughly 0.46ms getting back to the start

out.effectiveDutyCycle = dwellTime*xPixels/linePeriod;
if mode ==0
out.flyBackTime = lost-porches;
end
out.timePerPixel = ((linePeriod*yPixels+piezoDelay)*nSlices)/(xPixels*yPixels*nSlices);
end

%%
