function [pf] = participation_factor(a)
[m,n]=size(a);
[P, D]=eig(a);
QT=inv(P);
Q=QT';
j=1;
% j is index on columns (modes)
% i is index on rows (states)
while j<=n,
i=1;
while i<=m,
pf(i,j)=Q(i,j)*P(i,j);
i=i+1;
end
j=j+1;
end
pf
end

