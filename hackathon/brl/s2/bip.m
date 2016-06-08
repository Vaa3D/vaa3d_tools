function out = bip(axishandle)

if nargin ==0 | isempty(axishandle)
    axishandle = gca;
end


% fix the font size of text

texth = findobj(axishandle, 'type', 'text');

for i = 1:numel(texth)
    set(texth(i), 'fontsize', 16);
end


set(axishandle, 'FontSize', 16)

set(get(gca,'xlabel'), 'FontSize', 16)
set(get(gca,'ylabel'), 'FontSize', 16)
set(get(gca,'zlabel'), 'FontSize', 16)
set(get(gca,'title'), 'FontSize', 16)


linehandles = findobj(axishandle, 'type', 'line');
for i=1:numel(linehandles)
    set(linehandles(i), 'linewidth', 2)
end

% increase the figure size

fh = get(axishandle, 'parent');
set(fh,'Position', [1010 700 1000 700])