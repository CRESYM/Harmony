function [Xdcpnz,Ydcpnz,Zdcpnz] = fGenerateInputSignals(N)

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

for i = 1 : 1 : N
    
    Xmp(i) = rand(1);
    Xap(i) = rand(1)*2*pi;
    Xmn(i) = rand(1);
    Xan(i) = rand(1)*2*pi;
    Xmz(i) = rand(1);
    Xaz(i) = rand(1)*2*pi;
    Xdc = (-1+2*rand(1,3));

    Xpnz(:,i) = [ Xmp(i) Xap(i) Xmn(i) Xan(i) Xmz(i) Xaz(i)];


    Ymp(i) = rand(1);
    Yap(i) = rand(1)*2*pi;
    Ymn(i) = rand(1);
    Yan(i) = rand(1)*2*pi;
    Ymz(i) = rand(1);
    Yaz(i) = rand(1)*2*pi;
       Ydc = 0*(-1+2*rand(1,3));
    Ypnz(:,i) = [ Ymp(i) Yap(i) Ymn(i) Yan(i) Ymz(i) Yaz(i)];
    
    Zmp(i) = rand(1);
    Zap(i) = rand(1)*2*pi;
    Zmn(i) = rand(1);
    Zan(i) = rand(1)*2*pi;
    Zmz(i) = rand(1);
    Zaz(i) = rand(1)*2*pi;
       Zdc = 0*(-1+2*rand(1,3));
    Zpnz(:,i) = [ Zmp(i) Zap(i) Zmn(i) Zan(i) Zmz(i) Zaz(i)];

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


DC_X = 0*Ta_dqn*[1 -2 3]';
DC_X(1:5) = DC_X(1:5)/2;

Magp_dc = sqrt(DC_X(1)^2+DC_X(4)^2);
Angp_dc = atan2(DC_X(4),DC_X(1));
Magn_dc = sqrt(DC_X(2)^2+DC_X(5)^2);
Angn_dc = atan2(DC_X(5),DC_X(2));
Magz_dc = sqrt(DC_X(3)^2+DC_X(6)^2);
Angz_dc = atan2(DC_X(6),DC_X(3));

Xpnz_dc = [Magp_dc; Angp_dc; Magn_dc; Angn_dc; Magz_dc; Angz_dc];


DC_X = Ta_dqn*[1.4 -2.8 -0.45]';
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


end

