%Large Signal Modelling of MMC
clc
clear all
%Defining the parameters to be used in the code

P=96*10^(6);%Rated Power
f=50;%Rated Frequency
w1=2*pi*f;
Vdc=40*10^(3);%Rated DC Voltage
Vac=16*10^(3);%Rated AC Voltage
N=16;%submodule number per arm
Csm=20*10^(-3);%submodule capacitor
Carm=Csm/N;
Larm=5*10^(-3);%arm inductance
Rarm=0.1;%arm resistance
Lr = 0;
Rr = 0;

Reqac = Rarm/2 + Rr;
Leqac = Larm/2 + Lr;

%Defining the switching functions
m0=0.9944;m1=0.8094;theta1=0.1354;m2=0.0302;theta2=1.6686;

%generation of an arbitary switching signal
Fs = 1000; % sampling rate of 1000 Hz
t = 0:0.1/Fs:0.02; %creates a 1x1000 vector from 0 to (1-1/Fs)
t=transpose(t);

%developing the state space model for h=3
%defining the q matrix
h=3;%total harmonics considered
dor=(2*h)+1;
q=zeros(dor,dor);
for j=1:1:dor
    q(j,j)=-1*(h-j+1)*w1*1i;%harmonic matrix
end

I = eye(dor,dor);
O=zeros(dor,dor);
alpha=Reqac/Leqac;

% insertion indices, manually entered
ma_sigma = zeros(2*h+1, 1);
ma_sigma(h+1) = m0;
mb_sigma = ma_sigma;
mc_sigma = ma_sigma;

ma_sigma(h+3) = -m2/2/1i * exp(1i*theta2);
ma_sigma(h-1) = m2/2/1i * exp(-1i*theta2);

mb_sigma(h+3) = -m2/2/1i * exp(1i*theta2-2*pi*1i/3);
mb_sigma(h-1) = m2/2/1i * exp(-1i*theta2+2*pi*1i/3);

mc_sigma(h+3) = -m2/2/1i * exp(1i*theta2-4*pi*1i/3);
mc_sigma(h-1) = m2/2/1i * exp(-1i*theta2+4*pi*1i/3);

ma_delta = zeros(2*h+1, 1);
ma_delta(h+2) = -m1/2/1i*exp(1i*theta1);
ma_delta(h) = m1/2/1i*exp(-1i*theta1);
mb_delta = zeros(2*h+1, 1);
mb_delta(h) = m1/2/1i*exp(-1i*theta1+2*pi*1i/3);
mb_delta(h+2) = -m1/2/1i*exp(1i*theta1-2*pi*1i/3);
mc_delta = zeros(2*h+1, 1);
mc_delta(h) = m1/2/1i*exp(-1i*theta1+4*pi*1i/3);
mc_delta(h+2) =-m1/2/1i*exp(1i*theta1-4*pi*1i/3);

Gamma_a_sigma = taut(ma_sigma, h);
Gamma_b_sigma = taut(mb_sigma, h);
Gamma_c_sigma = taut(mc_sigma, h);
Gamma_sigma = blkdiag(Gamma_a_sigma, Gamma_b_sigma, Gamma_c_sigma);

Gamma_a_delta = taut(ma_delta, h);
Gamma_b_delta = taut(mb_delta, h);
Gamma_c_delta = taut(mc_delta, h);
Gamma_delta = blkdiag(Gamma_a_delta, Gamma_b_delta, Gamma_c_delta);

%building HSS A matrix
A11 = blkdiag(-alpha*I - q, -alpha*I - q, -alpha*I - q);
A12 = blkdiag(O,O,O);
A13 = -1/2/Leqac * Gamma_sigma;
A14 = -1/2/Leqac * Gamma_delta;
A1 = [A11,A12,A13,A14];

A21 = A12;
A22 = blkdiag(-Rarm/Larm*I - q, -Rarm/Larm*I - q, -Rarm/Larm*I - q);
A23 = -1/2/Larm * Gamma_delta;
A24 = -1/2/Larm * Gamma_sigma;
A2 = [A21, A22, A23, A24];

A31 = 1/4/Carm * Gamma_sigma;
A32 = 1/2/Carm * Gamma_delta;
A33 = blkdiag(-q,-q,-q);
A34 = A12;
A3 = [A31, A32, A33, A34];

A41 = 1/4/Carm * Gamma_delta;
A42 = 1/2/Carm * Gamma_sigma;
A43 = A12;
A44 = blkdiag(-q,-q,-q);
A4 = [A41, A42, A43, A44];

A = vertcat(A1,A2,A3,A4);

% building HSS B matrix
B1 = (1/(2*Larm))*vertcat(I,I,I);
O1 = vertcat(O,O,O);
B1=vertcat(O1, B1, O1, O1);
B2 = -1/Leqac * eye(3*dor, 3*dor);
O2 = zeros(3*dor, 3*dor);
B2 = vertcat(B2, O2, O2, O2);
B = [B1,B2];

%defining the input matrix
u1=zeros(dor,1);
u1(h+1)=Vdc;
ua = zeros(2*h+1, 1);
ua(h) = Vac/2;
ua(h+2) = Vac/2;
ub = zeros(2*h+1, 1);
ub(h) = Vac/2*exp(1*((2*pi)/3)*i);
ub(h+2) = Vac/2*exp(-1*((2*pi)/3)*i);
uc = zeros(2*h+1, 1);
uc(h) = Vac/2*exp(((4*pi)/3)*i);
uc(h+2) = Vac/2*exp(((-4*pi)/3)*i);
u=vertcat(u1,ua,ub,uc);

%steady state response
xss=-1*(A\B)*u;
%Obtaining the time domain response
x_ia_delta=xss(1:(dor),1);
x_ib_delta=xss(dor+1:(2*dor),1);
x_ic_delta=xss((2*dor)+1:(3*dor),1);
x_ia_sigma=xss((3*dor)+1:(4*dor),1);
x_ib_sigma=xss((4*dor)+1:(5*dor),1);
x_ic_sigma=xss((5*dor)+1:(6*dor),1);
x_va_delta=xss((6*dor)+1:(7*dor),1);
x_vb_delta=xss((7*dor)+1:(8*dor),1);
x_vc_delta=xss((8*dor)+1:(9*dor),1);
x_va_sigma=xss((9*dor)+1:(10*dor),1);
x_vb_sigma=xss((10*dor)+1:(11*dor),1);
x_vc_sigma=xss((11*dor)+1:(12*dor),1);

%AC side phase cuRarmRarments in phase A,B,C
[x_ia_deltat,a1]=time_domain(x_ia_delta,t,h,w1);
[x_ib_deltat,a2]=time_domain(x_ib_delta,t,h,w1);
[x_ic_deltat,a3]=time_domain(x_ic_delta,t,h,w1);

%circulating currents in phase A,B,C
[x_ia_sigmat,a4]=time_domain(x_ia_sigma,t,h,w1);
[x_ib_sigmat,a5]=time_domain(x_ib_sigma,t,h,w1);
[x_ic_sigmat,a6]=time_domain(x_ic_sigma,t,h,w1);

%capacitor differene voltages in phase A,B,C
[x_va_deltat,a7]=time_domain(x_va_delta,t,h,w1);
[x_vb_deltat,a8]=time_domain(x_vb_delta,t,h,w1);
[x_vc_deltat,a9]=time_domain(x_vc_delta,t,h,w1);

%capacitoRarm voLarmtage in the LarmoweRarm Larmeg in phase A,B,C
[x_va_sigmat,a10]=time_domain(x_va_sigma,t,h,w1);
[x_vb_sigmat,a11]=time_domain(x_vb_sigma,t,h,w1);
[x_vc_sigmat,a12]=time_domain(x_vc_sigma,t,h,w1);

figure
subplot(4,1,1)
plot(t,x_ia_deltat,t,x_ib_deltat,t,x_ic_deltat)%AC side phase cuRarmRarments in phase A,B,C
ylabel('i_{Cj}^\Delta [A]','Interpreter','tex')

subplot(4,1,2)
plot(t,x_ia_sigmat,t,x_ib_sigmat,t,x_ic_sigmat)%circulating currents in phases A, B, C
ylabel('i_{Cj}^\Sigma [A]','Interpreter','tex')

subplot(4,1,3)
plot(t,x_va_deltat,t,x_vb_deltat,t,x_vc_deltat)%capacitor difference voltage A,B,C
ylabel('v_{Cj}^\Delta [V]','Interpreter','tex')

subplot(4,1,4)
plot(t,x_va_sigmat,t,x_vb_sigmat,t,x_vc_sigmat)%capacitor average voltage in phase A,B,C
ylabel('v_{Cj}^\Sigma [V]','Interpreter','tex')
xlabel('time [s]','Interpreter','tex')


xva_ut = x_va_sigmat + x_va_deltat;
xvb_ut = x_vb_sigmat + x_vb_deltat;
xvc_ut = x_vc_sigmat + x_vc_deltat;

xva_lt = x_va_sigmat - x_va_deltat;
xvb_lt = x_vb_sigmat - x_vb_deltat;
xvc_lt = x_vc_sigmat - x_vc_deltat;

figure
subplot(2,1,1)
plot(t,xva_ut,t,xvb_ut,t,xvc_ut)%capacitor upper voltage A,B,C
ylabel('v_{Cj}^U [V]','Interpreter','tex')

subplot(2,1,2)
plot(t,xva_lt,t,xvb_lt,t,xvc_lt)%capacitor lower voltage in phase A,B,C
ylabel('v_{Cj}^L [V]','Interpreter','tex')
xlabel('time [s]','Interpreter','tex')

