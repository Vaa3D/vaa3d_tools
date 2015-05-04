function [AreaInt,AreaSamp]=modelPMTIntegral(pulseWidth,samplingRate)
% This function compares the integral of a random waveform generated 
% using 10x sampling with reference to the pulsewidth(i.e. 1 1e-6 sec pulsewidth is sampled at 10 
% 10 MHz to one that would be sampled at the sampling rate, like we do in ScanImage.
global pulseTrain
AreaInt=0;
AreaSamp=0;
if nargin ~=2
	pulseWidth = 2e-6;		% 2 us pulse width
	samplingRate=1.25e6;	% 1.25 Mhz sampling rate
end

generatedRate=100/pulseWidth;	% How to compute the integral

% Generate the pulse 
PixelTime=10*pulseWidth;	% PixelTime in seconds
totalPointsInInterval=round(PixelTime*generatedRate);
pulseTotal=zeros(1,totalPointsInInterval);	%blank pulse
pointsUsed=0;
startPulse=1;
pulse=makeGaussian(pulseWidth*generatedRate/100,pulseWidth*generatedRate);
pulseTrain.xscale=[0 1/generatedRate];
while pointsUsed < totalPointsInInterval
	startPulse=round(rand*(totalPointsInInterval-pointsUsed))+startPulse;
	if startPulse+length(pulse) > totalPointsInInterval
		break
	end
	pulseTotal(startPulse:startPulse+length(pulse)-1)=pulse;
	startPulse=startPulse+length(pulse);
	pointsUsed=startPulse;
end
pulseTrain.data=pulseTotal;
AreaInt=trapz(pulseTotal);

% Now look at the finite sampling rate...
ind=round(linspace(1,totalPointsInInterval,round(samplingRate*PixelTime)));
AreaSamp=sum(pulseTotal(ind));
% Function that makes the Gaussian....
drawnow;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%5
function out=makeGaussian(sd,pts)
x=linspace(-3*sd,3*sd,pts);
out=exp(-x.^2/(2*sd));
