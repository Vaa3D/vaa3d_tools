function minima = finddeepminima(x,depth)
%FINDMAXIMA  Find location of local minima
%  From David Sampson
%  See also FINDMAXIMA

maxima=findmaxima(x);
minima = findmaxima(-x);
x(maxima)-x(minima);
