function m = compute_moments_neuron(a, orders, radius_thres)
%function m = compute_moments_neuron(a, orders, radius_thres)
% compute the regular moments with orderss (e.g. [0 0 1], or [1 0 1]) of a
% neuron
%
% radius_thres - the threshold of radius for computation, Any node outside
% this radius will not be counted
%
% by Hanchuan Peng
% 070726
% 070808
% 070809: use the line-length sampling method
%
% 080225: remove the nodes whose parents are -2, i.e. isolated nodes
% 081101: add the radius_thres para
% 081103: add isinf for radius_thres

orders = orders(:);
if length(orders)~=3, error('orders must has at least 3 elements (orders of x,y,z).'); end;

step0=0.1;

N = length(a);
m = 0;
for i=2:N, 
    
    if (a(i,7)<0), %==-2
        continue;
    end;
    
    b1=a(i,3:5); 
    b2=a(a(i,7), 3:5); 
    c=a(i,6)*1; 

    % % % The following is an incorrect computing metod,where we have no
    % reason to use different sampling aspect ratio for different
    % coordinates x,y,z. Thus I decided to sample the arc length
    
% %     if b1(1)==b2(1),
% %         
% %         tmpx = (b1(1).^orders(1));
% %         
% %         if b1(2)==b2(2),
% %             
% %             tmpxy = tmpx * (b1(2).^orders(2));
% %             
% %             if b1(3)==b2(3), %%in case only 1 point
% %                 
% %                 m = m + tmpxy * (b1(3).^orders(3));
% %                 
% %             else,
% %                 
% %                 if (b1(3)>b2(3)),
% %                     step=-step0;
% %                 else,
% %                     step=step0;
% %                 end;
% %                 
% %                 z = [b1(3):step:b2(3)];
% %                     
% %                 m = m + tmpxy .* sum([z.^orders(3)])
% %                     
% %             end;
% %             
% %         else,
% %             
% %             if b1(2)>b2(2),
% %                 step=-step0;
% %             else,
% %                 step=step0;
% %             end;
% %             
% %             y = [b1(2):step:b2(2)];
% %             y = y ./ (b2(2)-b1(2));
% %             z = (b2(3)-b1(3)) .* y;
% %             
% %             m = m + tmpx .* sum( (y.^orders(2)) .* (z.^orders(3)) );
% %             
% %         end;
% %         
% %     else,
% %         if b1(1)>b2(1),
% %             step=-step0;
% %         else,
% %             step=step0;
% %         end;
% % 
% %         x = [b1(1):step:b2(1)]./(b2(1)-b1(1));
% %         y = (b2(2)-b1(2)) .* x;
% %         z = (b2(3)-b1(3)) .* x;
% % 
% %         %% The following apparently can be further simplified by considering
% %         %% only x's power and then multiply the weights. But here I just keep
% %         %% the simplest/stupidest form for clarity. 
% % 
% %         m = m + sum( (x.^orders(1)) .* (y.^orders(2)) .* (z.^orders(3)) );
% %     end;
    
    len = sqrt(sum((b1-b2).^2));
    K = floor(len/step0)+1;
    
    xstep = (b2(1)-b1(1))./K;
    ystep = (b2(2)-b1(2))./K;
    zstep = (b2(3)-b1(3))./K;
    
    x = zeros(K,1); %%use this instead of [b1(1):xstep:b2(1)] is because this guarantee allx,y,z will the same length
    y = zeros(K,1);
    z = zeros(K,1);
    for k=1:K,
        x(k) = b1(1)+k*xstep;
        y(k) = b1(2)+k*ystep;
        z(k) = b1(3)+k*zstep;
    end;
    
    if ~isempty(radius_thres) & ~isinf(radius_thres), %%only use the interpolated points that fall into the specified radius
        d = sqrt(x.^2+y.^2+z.^2);
        ind_d = find(d<=radius_thres);
    else,
        ind_d = [1:K];
    end;
    
    m = m + sum( (x(ind_d).^orders(1)) .* (y(ind_d).^orders(2)) .* (z(ind_d).^orders(3)) );

end;

return;
