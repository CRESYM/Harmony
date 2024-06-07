clear all;
close all;

restoredefaultpath;

load('bsim')
load('ssim')
load('usim')
load('ysim')
load('idsim')
load('S')

swOnRes = S.SwitchResistance;
swOffRes = 1e6*ones(size(swOnRes));
swType = S.SwitchType;

Ad = S.A;
Bd = S.B;
Cd = S.C;
Dd = S.D;
% 
ncb = 8;
npe = 12;
nsw = ncb+npe;

%%%%

%%%

nrSamp = size(ysim,1);
tm = 1:1:nrSamp;
% 
% plot(t,ysim(:,ncb+1),t,idsim(:,1))

is = zeros(size(idsim));
swVec = zeros(size(ssim));
swVec(:,1:ncb) = bsim;


y = zeros(size(ysim));


for t = 1 : nrSamp


[y(t,:)] = fDSS(usim(t,nsw+1:end)', Ad, Bd, Cd, Dd, S.SwitchResistance, 1e6*ones(size(swOnRes)), S.SwitchType, ssim(t,1:ncb) )';


end


%range = round(size(ysim,1)/2-5000):size(ysim,1)/2;
%range = round(size(ysim,1)/20-20000):size(ysim,1)/20;
range = 1:size(ysim,1)/10;

for i = 1 : 49 %nsw
    %subplot(4,5,i)
    subplot(7,7,i)
    %plot(tm(range),idsim(range,i),tm(range),is(range,i),'--','LineWidth',1);
    plot(tm(range),ysim(range,i),tm(range),y(range,i),'--','LineWidth',1);
end


% sc = 1;
% k = 1;
% 
% range = round(size(ysim,1)/2-5000):size(ysim,1)/2;
% 
% for i = 1 : size(Cd,1)
%     subplot(4,13,i)
%     plot(tm(range),ysim(range,i),tm(range),y(range,i),'--','LineWidth',1);
%     %plot(tm(1:end/2),ssim(1:end/2,i),tm(1:end/2),swVec(1:end/2,i),'--','LineWidth',1);
% 
% end

% for h = 1 : npe
%     i = h + ncb ;
%     subplot(3,4,h)
%     plot(tm(1:end/2),idsim(1:end/2,2*i),tm(1:end/2),id(1:end/2,2*i),'--','LineWidth',1);
%     %plot(tm(1:end/sc),ssim(1:end/sc,k*i),tm(1:end/sc),swVec(1:end/sc,k*i),'--','LineWidth',1);
% 
%     %ylim([-0.5 1.5])
% end

% range = 1:size(ysim,1)/5;
% 
% for i = 1 : ncb %nsw
%     %subplot(4,5,i)
%     subplot(2,4,i)
%     %plot(tm(range),idsim(range,i),tm(range),is(range,i),'--','LineWidth',1);
%     plot(tm(range),ssim(range,i),tm(range),bsim(range,i),'--','LineWidth',1);
% end
% 






