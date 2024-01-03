t = linspace(-pi,pi, 350);
X =      t .* sin( pi * .872*sin(t)./t);
Y = -abs(t) .* cos(pi * sin(t)./t);
plot(X,Y);
fill(X, Y, 'r');
axis square;
set(gcf, 'Position', get(0,'Screensize')); 
title('For Hafi(8==============D)', 'FontSize', 28);