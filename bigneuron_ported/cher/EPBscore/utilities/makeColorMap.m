function map = makeColorMap(color, bits)
global gh state


a = zeros(2^bits,1);
b = (0:1/(2^bits -1):1)';

switch color 
case 'red'
	map = squeeze(cat(3, b, a, a));
case 'green'
	map = squeeze(cat(3, a, b, a));
case 'blue'
	map = squeeze(cat(3, a, a, b));
case 'gray'
	map = squeeze(cat(3, b, b, b));
end

	