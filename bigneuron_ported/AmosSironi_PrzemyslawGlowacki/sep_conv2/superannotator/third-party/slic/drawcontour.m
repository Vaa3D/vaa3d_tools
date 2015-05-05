function drawcontour(MASK, I)



L = bwlabel(MASK);



[B,L] = bwboundaries(L,8, 'holes');


imshow(I); hold on;

for k = 1:length(B)
    boundary = B{k};
    plot(boundary(:,2), boundary(:,1), 'g', 'LineWidth', 2)
end

hold off;

