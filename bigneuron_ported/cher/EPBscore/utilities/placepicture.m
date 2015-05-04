function placepicture(coordinates,j,filename)
a=opentif(filename);
i=find(coordinates(1,:)==j);
imagesc([-coordinates(3,i)-5 -coordinates(3,i)+5],[-coordinates(2,i)+5 -coordinates(2,i)-5],a,[0 200]);