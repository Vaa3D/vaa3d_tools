function drawROIs(coordinates)
hold on;
for i=1:size(coordinates,2)
    h=rectangle('Position',[-coordinates(3,i)-5 -coordinates(2,i)-5 10 10]);
    set(h,'EdgeColor',[1 0 0]);
    h=text(-coordinates(3,i),-coordinates(2,i),num2str(coordinates(1,i)));
    set(h,'Color',[1 0 0]);
end
    