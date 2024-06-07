clear all;
close all;

load('ACDC3')
load('xsim')
load('usim')
load('ysim')

nsw = 6;


Ad = ACDC3.Adiscrete;
Bd = ACDC3.Bdiscrete;
Cd = ACDC3.Cdiscrete;
Dd = ACDC3.Ddiscrete;

arows = size(Ad,1)/3; %signal size
acols = size(Ad,2)/3; %signal size

brows = size(Bd,1)/3; %signal size
bcols = size(Bd,2)/3; %signal size

crows = size(Cd,1)/3; %signal size
ccols = size(Cd,2)/3; %signal size

drows = size(Dd,1)/3; %signal size
dcols = size(Dd,2)/3; %signal size

Ad11 = Ad(1:nsw,1:nsw);        if(isempty(Ad11)) Ad11 = zeros(3,3); end
Ad12 = Ad(1:nsw,nsw+1:end);    if(isempty(Ad12)) Ad12 = zeros(3,3); end
Ad21 = Ad(nsw+1:end,1:nsw);    if(isempty(Ad21)) Ad21 = zeros(3,3); end
Ad22 = Ad(nsw+1:end,nsw+1:end); if(isempty(Ad22)) Ad22 = zeros(3,3); end

Bd11 = Bd(1:nsw,1:nsw);        if(isempty(Bd11)) Bd11 = zeros(3,3); end
Bd12 = Bd(1:nsw,nsw+1:end);    if(isempty(Bd12)) Bd12 = zeros(3,3); end
Bd21 = Bd(nsw+1:end,1:nsw);    if(isempty(Bd21)) Bd21 = zeros(3,3); end
Bd22 = Bd(nsw+1:end,nsw+1:end); if(isempty(Bd22)) Bd22 = zeros(3,3*bcols-nsw); end

Cd11 = Cd(1:nsw,1:nsw);        if(isempty(Cd11)) Cd11 = zeros(3,3); end
Cd12 = Cd(1:nsw,nsw+1:end);    if(isempty(Cd12)) Cd12 = zeros(3,3); end
Cd21 = Cd(nsw+1:end,1:nsw);    if(isempty(Cd21)) Cd21 = zeros(3,3); end
Cd22 = Cd(nsw+1:end,nsw+1:end); if(isempty(Cd22)) Cd22 = zeros(3*crows-nsw,3*ccols); end

Dd11 = Dd(1:nsw,1:nsw);        if(isempty(Dd11)) Dd11 = zeros(3,3); end
Dd12 = Dd(1:nsw,nsw+1:end);    if(isempty(Dd12)) Dd12 = zeros(3,3); end
Dd21 = Dd(nsw+1:end,1:nsw);    if(isempty(Dd21)) Dd21 = zeros(3,3); end
Dd22 = Dd(nsw+1:end,nsw+1:end); if(isempty(Dd22)) Dd22 = zeros(3,3); end



t = 0:1:size(xsim,1);

nrSamp = size(xsim,1);

xn = zeros(nrSamp,size(Ad,1));
un = zeros(nrSamp,size(Bd,2));
yn = zeros(nrSamp,size(Cd,1));


y_old = zeros(size(Cd,1),1);

us_old = 0;
uns_old = 0;

swFlag = 0;
swCnt = 0;

kb1 = 1e6;
kb2 = 1e6;

k = [ones(3,1)*kb1; ones(3,1)*kb2];
D11Ik = eye(nsw)/(eye(nsw) - Dd11./k);
Dd21k = [Dd21(:,1:3)/kb1 Dd21(:,4:6)/kb2];
% Dd21k = Dd21./k;

E = D11Ik * Cd11;
F = D11Ik * Cd12;
G = Dd21k*D11Ik*Cd11 + Cd21;
H = Dd21k*D11Ik*Cd12 + Cd22;
I = D11Ik*Dd12;
J = (Dd22+Dd21k*D11Ik*Dd12);

Bd11k = Bd11./k;
Bd21k = [Bd21(:,1:3)/kb1 Bd21(:,4:6)/kb2];
%Bd21k = Bd21./k;

Ads = [Ad11+Bd11k*E Ad12+Bd11k*F ; Ad21+Bd21k*E Ad22+Bd21k*F ];
Bds = [(Bd11k*I + Bd12); Bd21k*I + Bd22];
Cds = [ D11Ik * Cd11 D11Ik * Cd12 ; Dd21k*D11Ik*Cd11 + Cd21 Dd21k*D11Ik*Cd12 + Cd22];
Dds = [ D11Ik*Dd12; (Dd22+Dd21k*D11Ik*Dd12)];

for i = 1 : nrSamp
    BRK1 = bvecsim(i,1);
    BRK2 = bvecsim(i,2);
    
    if(i>1)
        if(bvecsim(i,1) ~= bvecsim(i-1,1))
            swFlag = 1;
        end
        us = zeros(nsw,1);
        uns = usim(i-1,nsw+1:end)';
    else
        x_old = zeros(size(Ad,1),1);
        us = zeros(nsw,1);
        uns = usim(i,nsw+1:end)';
    end

    if(swFlag)
        kb1 = 1e6*abs(1-BRK1)+0.01*abs(0-BRK1);
        kb2 = 1e6*abs(1-BRK2)+0.01*abs(0-BRK2);
    
        k = [ones(3,1)*kb1; ones(3,1)*kb2];
        Dd11k = [Dd11(:,1:3)/kb1 Dd11(:,4:6)/kb2];
        D11Ik = eye(nsw)/(eye(nsw) - Dd11k);
        Dd21k = [Dd21(:,1:3)/kb1 Dd21(:,4:6)/kb2];
        % Dd21k = Dd21./k;
        
        E = D11Ik * Cd11;
        F = D11Ik * Cd12;
        % G = Dd21k*D11Ik*Cd11 + Cd21;
        % H = Dd21k*D11Ik*Cd12 + Cd22;
        I = D11Ik*Dd12;
        % J = (Dd22+Dd21k*D11Ik*Dd12);
        
        
        Bd11k = [Bd11(:,1:3)/kb1 Bd11(:,4:6)/kb2];
        %Bd11k = Bd11./k;
        Bd21k = [Bd21(:,1:3)/kb1 Bd21(:,4:6)/kb2];
        %Bd21k = Bd21./k;


        Ads = [Ad11+Bd11k*E Ad12+Bd11k*F ; Ad21+Bd21k*E Ad22+Bd21k*F ];
        Bds = [(Bd11k*I + Bd12); Bd21k*I + Bd22];
        Cds = [ D11Ik * Cd11 D11Ik * Cd12 ; Dd21k*D11Ik*Cd11 + Cd21 Dd21k*D11Ik*Cd12 + Cd22];
        Dds = [ D11Ik*Dd12; (Dd22+Dd21k*D11Ik*Dd12)];
        swFlag = 0;
    end




    if(i>1)

        % y = [D11I*Dd12; (Dd22+Dd21*D11I*Dd12)]*uns;  yn(i,:) = y';      y_old = y;  

        % x = [Ad11+Bd11k*E Ad12+Bd11k*F ; Ad21+Bd21k*E Ad22+Bd21k*F ]*x_old + ...
        %     [(Bd11k*I + Bd12); Bd21k*I + Bd22]*uns;                                                     xn(i,:) = x';       x_old = x;
        % y = [ D11Ik * Cd11 D11Ik * Cd12 ; Dd21k*D11Ik*Cd11 + Cd21 Dd21k*D11Ik*Cd12 + Cd22]*x + ...
        %     [ D11Ik*Dd12; (Dd22+Dd21k*D11Ik*Dd12)]*uns;                                                 yn(i,:) = y';       y_old = y; 

        x = Ads*x_old   + Bds*uns;      xn(i,:) = x';         x_old = x;                                                    
        y = Cds*x       + Dds*uns;      yn(i,:) = y';         y_old = y; 

        un(i,:) = [y(1:6)./k; uns]';
  
    else

        x = Ads*x_old   + Bds*uns;      xn(i,:) = 0*x';         x_old = 0*x;                                                    
        y = Cds*x       + Dds*uns;      yn(i,:) = y';           y_old = y; 
    

        % x = [Ad11+Bd11k*E Ad12+Bd11k*F ; Ad21+Bd21k*E Ad22+Bd21k*F ]*x_old + ...
        %     [(Bd11k*I + Bd12); Bd21k*I + Bd22]*uns;                                                     xn(i,:) = 0*x';       x_old = 0*x;
        % y = [ D11Ik * Cd11 D11Ik * Cd12 ; Dd21k*D11Ik*Cd11 + Cd21 Dd21k*D11Ik*Cd12 + Cd22]*x_old + ...
        %     [ D11Ik*Dd12; (Dd22+Dd21k*D11Ik*Dd12)]*uns;                                                 yn(i,:) = y';       y_old = y; 
        % 
        % un(i,:) = [y(1:6)./k; uns]';

        % xn = xsim(i,:);
        % un = usim(i,:);
        % yn = ysim(i,:);

    end


end


    % % xs = xsim.Data(i,:)';
    % % ys = ysim.Data(i,:)';
    % 
    % 
    % 
    % if(i==1)     
    %     x_old = zeros(size(Ad,1),1);
    %     us = usim(i,1:6)';
    %     uns = usim(i,nsw+1:end)';
    % else
    %     us = zeros(nsw,1);
    %     uns = usim(i-1,nsw+1:end)';
    % end
    % 
    % 
    % 
    % if(BRK1 == 0 && BRK2 == 0)
    % 
    %     if(i==2)
    %         us(1:3) = y_old(1:3) / 1e6;
    %         us(4:6) = y_old(4:6) / 1e6; 
    % 
    % 
    %         x = 0 + Bd*[us; uns];     xn(i,:) = x';    x_old = x;
    %         Cd1 = [Cd zeros(33,7)];
    %         Dd1 = [Dd zeros(33,16)];
    % 
    %         y = ((blkdiag(eye(17), 0*eye(16))-Dd1)\(Cd)*x); yn(i,:) = y';      y_old = y;   
    % 
    %         %y = Cd*x + Dd*[us; uns];      
    % 
    % 
    % 
    %         un(i,:) = [us; uns]';
    % 
    % 
    % 
    %     elseif(i==1)
    %         un(i,:) = [us; uns]'; us_old = us;
    %         x = Ad*x_old + Bd*[us; uns];     xn(i,:) = 0*x';    x_old = 0*x;
    %         y = (Cd*x*0 + Dd*[us; uns])';    yn(i,:) = y';      y_old = y;
    % 
    % 
    % 
    %     end
    % 
    % 
    %     % x_old = x;
    %     % us_old = us;
    %     % uns_old = uns;
    % 
    % 
    % 
    % elseif(BRK1 == 1 && BRK2 == 0)
    % 
    %     % if(swFlag)
    %     %     y_old(1:3) = y_old(1:3) / 1e6;
    %     % 
    %     % end
    %     % 
    %     % us(1:3) = y_old(1:3) / 0.01;
    %     % us(4:6) = y_old(4:6) / 1e6; un(i,:) = [us; uns]';
    %     % 
    %     % if(swFlag)
    %     %     x = Ad*x_old + Bd*[us; uns];     xn(i,:) = x';
    %     %     y = ( (Cd*x_old + Dd*[us_old; uns_old])+(Cd*x + Dd*[us; uns]) ) / 2;
    %     %     y(4:6) = y(4:6) * 2;
    %     %     swFlag = 0;
    %     % else
    %     %     x = Ad*x_old + Bd*[us; uns];     xn(i,:) = x';
    %     %     y = Cd*x + Dd*[us; uns];         
    %     % end
    %     % 
    %     % 
    %     % 
    %     % y_old = y;
    %     % 
    %     %  y_old(1:3) = y_old(1:3) / 1e6;
    %     % yn(i,:) = y_old';
    % elseif(BRK1 == 1 && BRK2 == 1)
    % 
    % 
    % else
    % 
    % end
    % 
    % 
    % 
    % 




    % if(swFlag)
    %     if(swCnt == 0)
    %         y = ( (Cd*x_old + Dd*[us_old; uns_old])+(Cd*x + Dd*[us; uns]) ) / 2;
    %     else
    %         y = ( (Cd*x_old + Dd*[us_old; uns_old])+(Cd*x + Dd*[us; uns]) );
    %     end
    %     y(1:3) = y(1:3) / 1e6;     yn(i,:) = y';
    %     swCnt = swCnt + 1;
    % 
    %     if(swCnt==2)
    %         swFlag = 0;
    %     end
    % else
    %     y = Cd*x + Dd*[us; uns];     
    % end



    % if(BRK1 == 0)
    %     us(1:3) = y_old(1:3) / 1e6;
    % else
    %     if(swFlag && swCnt == 0)
    %         us(1:3) = y_old(1:3) / 1e6 / 0.01;
    %     elseif(swFlag && swCnt == 1)
    %         us(1:3) = 2*y_old(1:3) / 0.01;
    %     else
    %         us(1:3) = y_old(1:3) / 0.01;
    %     end
    % end
    % 
    % if(BRK2 == 0)
    %     us(4:6) = y_old(4:6) / 1e6;
    % else
    %     us(4:6) = y_old(4:6) / 0.01;
    % end
