function out=getneighborhoodpixels(imageArray,x,y,z,dx,dy,dz)
left=max(x-dx,1);
right=min(x+dx,size(imageArray,1));
up=max(y-dy,1);
down=min(y+dy,size(imageArray,2));
top=max(z-dz,1);
bottom=min(z+dz,size(imageArray,3)); 
out=imageArray(up:down,left:right,top:bottom);
