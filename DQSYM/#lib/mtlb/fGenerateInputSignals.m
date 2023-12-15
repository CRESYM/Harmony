function [Xdcpnz, Ydcpnz, Zdcpnz, Xdcpnz_c, Ydcpnz_c, Zdcpnz_c] = fGenerateInputSignals(N,DC,seq_x,seq_y)



    if(nargin<4)
                seq_y = "pnz";

        if(nargin < 3)
            if(DC>1) DC = 1; end
            seq_x = "pnz";
            if(nargin < 2)
                DC = 0;
            end
        end
    end



Xmp = zeros(1,N);
Xap = zeros(1,N);
Xmn = zeros(1,N);
Xan = zeros(1,N); 
Xmz = zeros(1,N);
Xaz = zeros(1,N);

Ymp = zeros(1,N);
Yap = zeros(1,N);
Ymn = zeros(1,N);
Yan = zeros(1,N);
Ymz = zeros(1,N);
Yaz = zeros(1,N);

Zmp = zeros(1,N);
Zap = zeros(1,N);
Zmn = zeros(1,N);
Zan = zeros(1,N);
Zmz = zeros(1,N);
Zaz = zeros(1,N);


Xpnz = zeros(6,N);
Ypnz = zeros(6,N);
Zpnz = zeros(6,N);

Xpnz_c = zeros(3,N);
Ypnz_c = zeros(3,N);
Zpnz_c = zeros(3,N);

a = -0.5+0.866i;
a2 = -0.5-0.866i;
Sas = [1,a,a2;1,a2,a;1,1,1]/3;
Ssa = inv(Sas);

xp = 0; xn = 0; xz = 0;
yp = 0; yn = 0; yz = 0;

if(contains(seq_x,"p")) xp = 1; end
if(contains(seq_x,"n")) xn = 1; end
if(contains(seq_x,"z")) xz = 1; end

if(contains(seq_y,"p")) yp = 1; end
if(contains(seq_y,"n")) yn = 1; end
if(contains(seq_y,"z")) yz = 1; end

for i = 1 : 1 : N
    
    Xmp(i) = xp*rand(1);
    Xap(i) = rand(1)*2*pi;
    Xmn(i) = xn*rand(1);
    Xan(i) = rand(1)*2*pi;
    Xmz(i) = xz*rand(1);
    Xaz(i) = rand(1)*2*pi;
    Xdc = (-1+2*rand(1,3));

    Xpnz(:,i) = [ Xmp(i) Xap(i) Xmn(i) Xan(i) Xmz(i) Xaz(i)];
    Xpnz_c(:,i) = [Xmp(i)*( cos(Xap(i)) + 1i*sin(Xap(i)) ); Xmn(i)*( cos(Xan(i)) + 1i*sin(Xan(i)) ); Xmz(i)*( cos(Xaz(i)) + 1i*sin(Xaz(i)) ); ];

    Ymp(i) = yp*rand(1);
    Yap(i) = rand(1)*2*pi;
    Ymn(i) = yn*rand(1);
    Yan(i) = rand(1)*2*pi;
    Ymz(i) = yz*rand(1);
    Yaz(i) = rand(1)*2*pi;
       Ydc = 0*(-1+2*rand(1,3));
    Ypnz(:,i) = [ Ymp(i) Yap(i) Ymn(i) Yan(i) Ymz(i) Yaz(i)];
    Ypnz_c(:,i) = [Ymp(i)*( cos(Yap(i)) + 1i*sin(Yap(i)) ); Ymn(i)*( cos(Yan(i)) + 1i*sin(Yan(i)) ); Ymz(i)*( cos(Yaz(i)) + 1i*sin(Yaz(i)) ); ];

    
    Zmp(i) = rand(1);
    Zap(i) = rand(1)*2*pi;
    Zmn(i) = rand(1);
    Zan(i) = rand(1)*2*pi;
    Zmz(i) = rand(1);
    Zaz(i) = rand(1)*2*pi;
       Zdc = 0*(-1+2*rand(1,3));
    Zpnz(:,i) = [ Zmp(i) Zap(i) Zmn(i) Zan(i) Zmz(i) Zaz(i)];
    Zpnz_c(:,i) = [Zmp(i)*( cos(Zap(i)) + 1i*sin(Zap(i)) ); Zmn(i)*( cos(Zan(i)) + 1i*sin(Zan(i)) ); Zmz(i)*( cos(Zaz(i)) + 1i*sin(Zaz(i)) ); ];

end



Tad_p2 = (2/3)*[0 sin(0-2*pi/3) sin(0+2*pi/3);  
                1 cos(0-2*pi/3) cos(0+2*pi/3);];

Tad_z2 = (1/3)*[0 0 0;  
                1 1 1;];

Tda_p2 = [0                 1;
          sin(0-2*pi/3)     cos(0-2*pi/3);
          sin(0+2*pi/3)     cos(0+2*pi/3)];

Tda_z2 = [0     1;
          0     1;
          0     1];

Tdqn_a = [Tda_p2(:,1)  Tda_p2(:,1)  Tda_z2(:,1)  Tda_p2(:,2)  Tda_p2(:,2)  2*Tda_z2(:,2)]/2;
Ta_dqn = [Tad_p2(1,:); Tad_p2(1,:); Tad_z2(1,:); Tad_p2(2,:); Tad_p2(2,:); Tad_z2(2,:)];


DC_X = DC*Ta_dqn*[1 -2 3]';
DC_X(1:5) = DC_X(1:5)/2;


Magp_dc = sqrt(DC_X(1)^2+DC_X(4)^2);
Angp_dc = atan2(DC_X(4),DC_X(1));
Magn_dc = sqrt(DC_X(2)^2+DC_X(5)^2);
Angn_dc = atan2(DC_X(5),DC_X(2));
Magz_dc = sqrt(DC_X(3)^2+DC_X(6)^2);
Angz_dc = atan2(DC_X(6),DC_X(3));

Xpnz_dc = [Magp_dc; Angp_dc; Magn_dc; Angn_dc; Magz_dc; Angz_dc];


DC_X = DC*Ta_dqn*[1.4 -2.8 -0.45]';
DC_X(1:5) = DC_X(1:5)/2;


Magp_dc = sqrt(DC_X(1)^2+DC_X(4)^2);
Angp_dc = atan2(DC_X(4),DC_X(1));
Magn_dc = sqrt(DC_X(2)^2+DC_X(5)^2);
Angn_dc = atan2(DC_X(5),DC_X(2));
Magz_dc = sqrt(DC_X(3)^2+DC_X(6)^2);
Angz_dc = atan2(DC_X(6),DC_X(3));

Ypnz_dc = [Magp_dc; Angp_dc; Magn_dc; Angn_dc; Magz_dc; Angz_dc];

DC_X = 0*Ta_dqn*[1.1 -1.8 -2.45]';
DC_X(1:5) = DC_X(1:5)/2;

Magp_dc = sqrt(DC_X(1)^2+DC_X(4)^2);
Angp_dc = atan2(DC_X(4),DC_X(1));
Magn_dc = sqrt(DC_X(2)^2+DC_X(5)^2);
Angn_dc = atan2(DC_X(5),DC_X(2));
Magz_dc = sqrt(DC_X(3)^2+DC_X(6)^2);
Angz_dc = atan2(DC_X(6),DC_X(3));

Zpnz_dc = [Magp_dc; Angp_dc; Magn_dc; Angn_dc; Magz_dc; Angz_dc];


Xdcpnz = [Xpnz_dc Xpnz];
Ydcpnz = [Ypnz_dc Ypnz];
Zdcpnz = [Zpnz_dc Zpnz];

Xdcpnz_c = [DC*Sas*[1 -2 3]' Xpnz_c];
Ydcpnz_c = [DC*Sas*[1.4 -2.8 -0.45]' Ypnz_c];
Zdcpnz_c = [0*Zpnz_c(:,1) Zpnz_c];


end

