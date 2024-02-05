%Steady State response of MMC
clc
clear all
%Defining the parameters to be used in the code
P=50*10^(6);%rated power
f=50;%rated frequency
w1=2*pi*f;
Vdc=320*10^(3);%rated DC voltage
Vac=166*10^(3);%rated AC voltage
N=20;%submodules per arm
Csm=140*10^(-6);%submodule capacitance
Carm=Csm/N;
Larm=360*10^(-3);%arm inductance
Rarm=1;%arm resistance
Lr = 0;
Rr = 0;

Reqac = Rarm/2 + Rr;
Leqac = Larm/2 + Lr;
Zl=360; %+(408.84*1i);%load from rated power and assumed power factor of 0.8 lag
%Defining the switching functions
%generation of an arbitary switching signal
Fs = 1000; % sampling rate of 1000 Hz
t = 0:0.1/Fs:0.02; %creates a 1x1000 vector from 0 to (1-1/Fs)
t=transpose(t);
m1=(Vac/(sqrt(3)))/(Vdc/2);
m2=0;

%developing the state space model for h=3
%defining the q matrix
h=5; %number of harmonics
dor=(2*h)+1;
q=zeros(dor,dor);
for j=1:1:dor
    q(j,j)=-1*(h-j+1)*w1*1i;%harmonic matrix
end
%defining the state variable matrix
ia_sigma=zeros(dor,1);ib_sigma=zeros(dor,1);ic_sigma=zeros(dor,1);%circulating currents
va_sigma=zeros(dor,1);vb_sigma=zeros(dor,1);vc_sigma=zeros(dor,1);%capacitor average voltages
va_delta=zeros(dor,1);vb_delta=zeros(dor,1);vc_delta=zeros(dor,1);%capacitor difference voltages
ia_delta=zeros(dor,1);ib_delta=zeros(dor,1);ic_delta=zeros(dor,1);%AC side currents
x=vertcat(ia_delta,ib_delta,ic_delta,ia_sigma,ib_sigma,ic_sigma,va_delta,vb_delta,vc_delta,va_sigma,vb_sigma,vc_sigma);%state variables
sizx=length(x);

%defining the input matrix
u=zeros(dor,1);
for j=1:1:dor
    if(j==(h+1))
        u(j,1)=Vdc;
    else
        u(j,1)=0;
    end
end
I=eye(dor,dor);
O=zeros(dor,dor);
alpha=(Reqac+Zl)/Leqac;
%bulding HSS B matrix
B1 = (1/(2*Larm))*vertcat(I,I,I);
O1 = vertcat(O,O,O);
B=vertcat(O1, B1, O1, O1);

% insertion indices, manually entered
ma_sigma = zeros(2*h+1, 1);
ma_sigma(h+1) = 1;
mb_sigma = ma_sigma;
mc_sigma = ma_sigma;

ma_delta = zeros(2*h+1, 1);
ma_delta(h) = -m1/2*exp(0);
ma_delta(h+2) = -m1/2*exp(0);
mb_delta = zeros(2*h+1, 1);
mb_delta(h) = -m1/2*exp(2*pi*1i/3);
mb_delta(h+2) = -m1/2*exp(-2*pi*1i/3);
mc_delta = zeros(2*h+1, 1);
mc_delta(h) = -m1/2*exp(4*pi*1i/3);
mc_delta(h+2) =-m1/2*exp(-4*pi*1i/3);

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
