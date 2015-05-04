function placepicture(coordinates,i,filename)
a=opentif(filename);
imagesc([coordinates(3,i)-5 coordinates(3,i)+5],[-coordinates(2,i)-5 -coordinates(2,i)+5],a);
