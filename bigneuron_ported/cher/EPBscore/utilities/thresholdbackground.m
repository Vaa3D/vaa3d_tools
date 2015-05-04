%THRESHOLD   Thresholding
%
% SYNOPSIS:
%  image_out = thresholdbackground(image_in,parameter)
%
% PARAMETERS:

%  parameter ('background'): Distance to the peak where we cut-off, in
%                            terms of the half-width at half the maximum.
%                            Inf selects the default value, which is 2.
%  Created by Sen 11/26/03 out of the thresholing module in dip_image
%  package

function [out,threshold_new] = threshold(in,parameter)
      if length(parameter)~=1 | parameter<=0
         error('Positive scalar value expected as parameter for ''background'' method.')
      end
      if ~isfinite(parameter)
         parameter = 2;        % Default value
      end

      % A smooth histogram
      min_val = min(in);
      max_val = max(in);
      interval = (max_val-min_val)/199;
      border = 16;
      max_val = max_val+border*interval;
      min_val = min_val-border*interval;
      [histogram,bins] = hist(in,200);
      hh=[0.0014795   0.0038042   0.0087535    0.018023    0.033208     0.05475    0.080775     0.10664     0.12598     0.13318     0.12598     0.10664    0.080775     0.05475    0.033208    0.018023   0.0087535   0.0038042   0.0014795];
      histogram=conv(histogram,hh);
     

      % Find peak
      [max_value,max_element] = max(histogram);

      % Is the peak on the left or right side of the histogram?
      %cumhist = cumsum(histogram);
      %cumhist = cumhist/cumhist(end);
      
      
         sigma = find(histogram(1:max_element) < (max_value/2));
         if length(sigma) ~= 0
            sigma = max_element - sigma(end);
         else
            sigma = 1;
         end
%         threshold_new = bins(max_element + sigma*parameter);
        threshold_new=(max_value-min(in))*parameter+min(in);
%         if thresind>200
%             threshold_new=max(in)+0.1;    
%         else    
%             threshold_new=bins(thresind);
%         end
      out = in >= threshold_new;

  