
piezoDelay = 30
nSlices = 50


%  2048x2048 at 1.13x zoom is .205um pixel size.  
summaryData = zeros(1, 8)
%   line period is 
xpix = 2048
ypix  = xpix
lp = 6.23  %ms
% for dwell time 2.8us
dt = 2.8*10^-3  %
a = scanChecks(xpix, lp, dt, 0, piezoDelay, nSlices)
summaryData=[summaryData;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]
summaryData = summaryData(2:end,:)



%  1.2us dwell time
dt = 1.2*10^-3  %
lp = 2.978  %ms
a =scanChecks(xpix, lp, dt, 0, piezoDelay, nSlices)
summaryData=[summaryData;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]

%  10us dwell time
dt = 10*10^-3  %
lp = 20.971  %ms
a=scanChecks(xpix, lp, dt, 0, piezoDelay, nSlices)
summaryData=[summaryData;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]





%  now at 13x  with 2.8us dwell time
xpix = 180
lp = 0.686
dt = 2.8*10^-3  %

a=scanChecks(xpix, lp, dt,0, piezoDelay, nSlices)
summaryData=[summaryData;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]



% 13x  1.2us dwell time
dt = 1.2*10^-3  %
xpix = 180
lp = .428
a=scanChecks(xpix, lp, dt,0, piezoDelay, nSlices)
summaryData=[summaryData;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]


% 13x  10us dwell time
dt = 10*10^-3  %
xpix = 180
lp = 1.967
a=scanChecks(xpix, lp, dt,0, piezoDelay, nSlices)
summaryData=[summaryData;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]





% 6.5x  same dwell time
dt = 2.8*10^-3  %
xpix = 360
lp = 1.24 
a=scanChecks(xpix, lp, dt,0, piezoDelay, nSlices)
summaryData=[summaryData;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]

% 6.5x  dwell time = 1.2
dt = 1.2*10^-3  %
xpix = 360
lp = 0.692 
a=scanChecks(xpix, lp, dt,0, piezoDelay, nSlices)
summaryData=[summaryData;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]
% 6.5x  dwell time = 10
dt = 10*10^-3  %
xpix = 360
lp = 3.818 
a=scanChecks(xpix, lp, dt,0, piezoDelay, nSlices)
summaryData=[summaryData;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]




%  23.4x  100pix  (same dwell time)
dt = 2.8*10^-3
xpix = 100
lp = .431
fp = 44.223
a=scanChecks(xpix, lp, dt,0, piezoDelay, nSlices)
summaryData=[summaryData;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]


%  23.4x  100pix  (1.2us dwell time)
dt = 1.2*10^-3  %
xpix = 100
lp = .304
a=scanChecks(xpix, lp, dt,0, piezoDelay, nSlices)
summaryData=[summaryData;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]


%  23.4x  100pix  (10us dwell time)
dt = 10*10^-3  %
xpix = 100
lp = 1.135
a=scanChecks(xpix, lp, dt,0, piezoDelay, nSlices)
summaryData=[summaryData;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]


%  10um tile:
dt = 2.8*10^-3
xpix = nSlices
lp = 0.266
fp = 13.913
a=scanChecks(xpix, lp, dt,0, piezoDelay, nSlices)
summaryData=[summaryData;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]


dt = 1.2*10^-3  %
xpix = nSlices
lp = 0.223
a=scanChecks(xpix, lp, dt,0, piezoDelay, nSlices)
summaryData=[summaryData;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]

dt = 10*10^-3  %
xpix = nSlices
lp = 0.608
a=scanChecks(xpix, lp, dt,0, piezoDelay, nSlices)
summaryData=[summaryData;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]


%%  resonant:   duty cycle = .66
%  lowest zoom with 0.205um pixel size is 2.25
% which is 210um on a side.


% to get up to 10 us dwell time, we need 2nSlicesx averaging.
% can only get to 128x averaging
% to get to 2.8us dwell time we can use 64x averaging (~2.6us dwell time)
% but at higher zooms, multisampling can help...

%  e.g. 13x zoom,  180pix
%  0.203um pixel size
%   9x multisampling.  9*.04 = .36us dwell time equivalent
%   and now to get 1.2us dwell time, we need 3.33 averaging
%  to get 2.8 we need 8x averaging and
% to get 10us dwell time we need  28x averaging.

%  1.13x zoom and 1024x1024  does not work with resonant scanning,
%  lowest zoom with 0.205um pixel size is 2.25
% which is 210um on a side.
%  1x multisampling  
% line period = 0.063ms  = bidirectional 8kHz scan. 
% duty cycle .66 means actual imaging is 0.0416ms
%  which corresponds to  .04us  dwell time for 1024 pix


scanSummaryDataR = zeros(1, 8)


dt = 0.04*10^-3  %    
xpix = 1024
lp = 0.063
a=scanChecks(xpix, lp, dt,1, piezoDelay, nSlices)
summaryDataR=[summaryDataR;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]
summaryDataR = summaryDataR(2:end,:)


%  64x averaging to get to 2.8us dwell time.

dt = 64*0.04*10^-3  %
xpix = 1024
lp = 64*0.063
a=scanChecks(xpix, lp, dt,1, piezoDelay, nSlices)
 summaryDataR=[summaryDataR;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]

%  and at higher zooms, multisampling can help...





%  e.g. 13x zoom,  180pix
%  0.203um pixel size
%  % actual imaging is 0.0416ms, split among 180pix
 dt180 = 0.0416/180
% this is 9x multisampling.

%   and now to get 1.2us dwell time, we need 5x averaging
%  to get 2.8 we need 12x averaging and
%  let's look at 8 and 16x averaging
% no averaging:
dt=dt180
xpix = 180
lp = 0.063
a=scanChecks(xpix, lp, dt180,1, piezoDelay, nSlices)
scanSummaryDataR=[scanSummaryDataR;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]

%  8x averaging averaging:
dt = 8*dt180
xpix = 180
lp = 8*0.063
a=scanChecks(xpix, lp, dt,1, piezoDelay, nSlices)
scanSummaryDataR=[scanSummaryDataR;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]

%  16x averaging averaging:
dt = 16*dt180
xpix = 180
lp = 16*0.063
a=scanChecks(xpix, lp, dt,1, piezoDelay, nSlices)
scanSummaryDataR=[scanSummaryDataR;[xpix,xpix^2,lp,lp*xpix,dt, a.timePerPixel, a.effectiveDutyCycle, a.timePerPixel*xpix^2]]


%  by construction, these all give the same time per line * averaging, but
%  time per frame can still influence total time / stack.

%

%% plot
figure, 
dt1p2 = sortrows(summaryData(summaryData(:,5)==.0012,:), 6)
dt2p8 = sortrows(summaryData(summaryData(:,5)==.0028,:),6)
dt10 = sortrows(summaryData(summaryData(:,5)== 0.01,:),6)
subplot(2,1,1)
hold all, plot(dt1p2(:,1), dt1p2(:,6), 'DisplayName', '1.2 us dwell time')
 plot(dt2p8(:,1), dt2p8(:,6), 'DisplayName', '2.8 us dwell time')
 plot(dt10(:,1), dt10(:,6), 'DisplayName', '10.0 us dwell time')
 plot(scanSummaryDataR(:,1), scanSummaryDataR(:,6), 'o')
xlabel('x pixels')
ylabel('time (s)')
legend toggle
title({'effective scan time per pixel ',['nSlices = ',num2str(nSlices), ', piezo delay = ', num2str(piezoDelay), ' ms'] })
subplot(2,1,2)


hold all, plot(dt1p2(:,1), dt1p2(:,6)./(dt1p2(:,5)), 'DisplayName', '1.2 us dwell time')
 plot(dt2p8(:,1), dt2p8(:,6)./(dt2p8(:,5)), 'DisplayName', '2.8 us dwell time')
 plot(dt10(:,1), dt10(:,6)./(dt10(:,5)), 'DisplayName', '10.0 us dwell time')
 
 plot(scanSummaryDataR(:,1), scanSummaryDataR(:,6)./(scanSummaryDataR(:,5)), 'o')


% 
% hold all, plot(dt1p2(:,1), dt1p2(:,6)/min(dt1p2(:,6)), 'DisplayName', '1.2 us dwell time')
%  plot(dt2p8(:,1), dt2p8(:,6)/min(dt2p8(:,6)), 'DisplayName', '2.8 us dwell time')
%  plot(dt10(:,1), dt10(:,6)/min(dt10(:,6)), 'DisplayName', '10.0 us dwell time')
 
xlabel('x pixels')
ylabel('badness normalized to nominal dwell time')
legend toggle
% 
% subplot(3,1,3)
% hold all, plot(dt1p2(:,1), dt1p2(:,6)./(dt1p2(:,5)), 'DisplayName', '1.2 us dwell time')
%  plot(dt2p8(:,1), dt2p8(:,6)./(dt2p8(:,5)), 'DisplayName', '2.8 us dwell time')
%  plot(dt10(:,1), dt10(:,6)./(dt10(:,5)), 'DisplayName', '10.0 us dwell time')
%  
%  plot(scanSummaryDataR(:,1), scanSummaryDataR(:,6)./(scanSummaryDataR(:,5)), 'o')
% 
% xlabel('x pixels')
% ylabel('badness normalized to big tile')
% legend toggle
title('performance decline for smaller tiles')

%%  PLOT 0:

%   for our latest re-scanning dataset including square BB :
%    x-axis:  sparsity 
%    y-axis:  actual time
%    y-axis:  BB-time
%    y-axis:  previously calculated BB-time
%    also somehow note the expected loss of scan time



% Plot 1.

%  Fixed imaging time per tile area,  
%  plot BB_S_time, sparsity and overall time cost for S2Scan (nonimaging
%  time)

%%  another plot idea:

% want to show how sparsity, tile size, and time-per-area scaling all
% interact:


x: tile size
y1:   time-per-area jumps up for smaller tiles
y2:   area-to-image decreases with smaller tiles
y3:   final structure sparsitycan also vary from structure to structure


area-to-image * time-per-area vs tile size





