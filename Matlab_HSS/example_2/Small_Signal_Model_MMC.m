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
L=5*10^(-3);%arm inductance
R=0.1;%arm resistance


%generation of an arbitary switching signal
Fs = 1000; % sampling rate of 1000 Hz
t = 0:0.1/Fs:0.02; %creates a 1x1000 vector from 0 to (1-1/Fs)
t=transpose(t);

%Defining the switching functions m2=0.0302
m0=0.9944;m1=0.8094;theta1=0.1354;m2=0.0302;theta2=1.6686;ps1p=exp(((4*pi)/3)*i);ps2p=exp(((2*pi)/3)*i);ps1n=exp(-1*((4*pi)/3)*i);ps2n=exp(-1*((2*pi)/3)*i);
%defining common-mode component of switching functions
%Phase a
fac1n=(m2/(2*i))*exp(-1*theta2*i);fac1p=(m2/(2*i))*exp(1*theta2*i);
vcma=[0;-1*fac1n;0;m0;0;fac1p;0];
%Phase b
vcmb=[0;-1*fac1n*ps1p;0;m0;0;fac1p*ps1n;0];
%Phase c
vcmc=[0;-1*fac1n*ps1n;0;m0;0;fac1p*ps1p;0];
%defining differential-mode component of switching functions
%Phase a
fac2n=(m1/(2*i))*exp(-1*theta1*i);fac2p=(m1/(2*i))*exp(1*theta1*i);
vdma=[0;0;fac2n;0;-1*fac2p;0;0];
%Phase b
vdmb=[0;0;fac2n*ps2p;0;-1*fac2p*ps2n;0;0];
%Phase c
vdmc=[0;0;fac2n*ps2n;0;-1*fac2p*ps2p;0;0];

%developing the state space model for h=3
%defining the q matrix
h=3;%total harmonics considered
dor=(2*h)+1;
q=zeros(dor,dor);
for j=1:1:dor
    q(j,j)=-1*(h-j+1)*w1*1i;%harmonic matrix
end
%defining the state variable matrix
ica=zeros(dor,1);icb=zeros(dor,1);icc=zeros(dor,1);%circulating currents
iga=zeros(dor,1);igb=zeros(dor,1);igc=zeros(dor,1);%AC side currents
vas=zeros(dor,1);vbs=zeros(dor,1);vcs=zeros(dor,1);%sum of upper and lower leg capacitor voltages
vad=zeros(dor,1);vbd=zeros(dor,1);vcd=zeros(dor,1);%difference of upper and lower leg capacitor voltages

x=vertcat(ica,icb,icc,iga,igb,igc,vas,vbs,vcs,vad,vbd,vcd);%state variable matrix

sizx=length(x);

%defining the input matrix
u1=zeros(dor,1);
for j=1:1:dor
    if(j==(h+1))
        u1(j,1)=Vdc;
    else
        u1(j,1)=0;
    end
end
ua=[0,0,0.5*Vac,0,0.5*Vac,0,0];ub=[0,0,0.5*Vac*exp(((2*pi)/3)*i),0,0.5*Vac*exp(-1*((2*pi)/3)*i),0,0];uc=[0,0,0.5*Vac*exp(-1*((2*pi)/3)*i),0,0.5*Vac*exp(((2*pi)/3)*i),0,0];
ua=transpose(ua);ub=transpose(ub);uc=transpose(uc);
u=vertcat(u1,ua,ub,uc);

I=eye(dor,dor);
O=zeros(dor,dor);

%building the B matrix in HSS domain
b1=horzcat(O,(1/(2*L))*I,O,O);b2=horzcat(O,O,(1/(2*L))*I,O);b3=horzcat(O,O,O,(1/(2*L))*I);
b4=horzcat((-1*(2/L))*I,O,O,O);b5=horzcat((-1*(2/L))*I,O,O,O);b6=horzcat((-1*(2/L))*I,O,O,O);
b7=horzcat(O,O,O,O);b8=horzcat(O,O,O,O);b9=horzcat(O,O,O,O);
b10=horzcat(O,O,O,O);b11=horzcat(O,O,O,O);b12=horzcat(O,O,O,O);
B=vertcat(b4,b5,b6, b1,b2,b3, b7,b8,b9,b10,b11,b12);
%building the A matrix in HSS domain
a1=horzcat((-1*(R/L))*I-q,O,O,O,O,O,-1*(taut(vcma,h)/(4*L)),O,O,(taut(vdma,h)/(4*L)),O,O);
a2=horzcat(O,(-1*(R/L))*I-q,O,O,O,O,O,-1*(taut(vcmb,h)/(4*L)),O,O,(taut(vdmb,h)/(4*L)),O);
a3=horzcat(O,O,(-1*(R/L))*I-q,O,O,O,O,O,-1*(taut(vcmc,h)/(4*L)),O,O,(taut(vdmc,h)/(4*L)));
a4=horzcat(O,O,O,(-1*(R/L))*I-q,O,O,(-1*taut(vdma,h))/(2*L),O,O,(-1*taut(vcma,h))/(2*L),O,O);
a5=horzcat(O,O,O,O,(-1*(R/L))*I-q,O,O,(-1*taut(vdmb,h))/(2*L),O,O,(-1*taut(vcmb,h))/(2*L),O);
a6=horzcat(O,O,O,O,O,(-1*(R/L))*I-q,O,O,(-1*taut(vdmc,h))/(2*L),O,O,(-1*taut(vcmc,h))/(2*L));
a7=horzcat(taut(vcma,h)/Carm,O,O,taut(vdma,h)/(2*Carm),O,O,O-q,O,O,O,O,O);
a8=horzcat(O,taut(vcmb,h)/Carm,O,O,taut(vdmb,h)/(2*Carm),O,O,O-q,O,O,O,O);
a9=horzcat(O,O,taut(vcmc,h)/Carm,O,O,taut(vdmc,h)/(2*Carm),O,O,O-q,O,O,O);
a10=horzcat(taut(vdma,h)/Carm,O,O,taut(vcma,h)/(2*Carm),O,O,O,O,O,O-q,O,O);
a11=horzcat(O,taut(vdmb,h)/Carm,O,O,taut(vcmb,h)/(2*Carm),O,O,O,O,O,O-q,O);
a12=horzcat(O,O,taut(vdmc,h)/Carm,O,O,taut(vcmc,h)/(2*Carm),O,O,O,O,O,O-q);
A=vertcat(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12);



%steady state response
xss=-1*(A\B)*u;
%Obtaining the time domain response
ic_pha=xss(1:(dor),1);
ic_phb=xss(dor+1:(2*dor),1);
ic_phc=xss((2*dor)+1:(3*dor),1);
ig_pha=xss((3*dor)+1:(4*dor),1);
ig_phb=xss((4*dor)+1:(5*dor),1);
ig_phc=xss((5*dor)+1:(6*dor),1);
vcm_pha=xss((6*dor)+1:(7*dor),1);
vcm_phb=xss((7*dor)+1:(8*dor),1);
vcm_phc=xss((8*dor)+1:(9*dor),1);
vdm_pha=xss((9*dor)+1:(10*dor),1);
vdm_phb=xss((10*dor)+1:(11*dor),1);
vdm_phc=xss((11*dor)+1:(12*dor),1);

%circulatings currents in phase A,B,C
[ic_phat,r1]=time_domain(ic_pha,t,h,w1);
[ic_phbt,r2]=time_domain(ic_phb,t,h,w1);
[ic_phct,r3]=time_domain(ic_phc,t,h,w1);
%AC side currents phase A,B,C
[ig_phat,r4]=time_domain(ig_pha,t,h,w1);
[ig_phbt,r5]=time_domain(ig_phb,t,h,w1);
[ig_phct,r6]=time_domain(ig_phc,t,h,w1);
%Common Mode capacitor voltages in phase A,B,C
[vcm_phat,r7]=time_domain(vcm_pha,t,h,w1);
[vcm_phbt,r8]=time_domain(vcm_phb,t,h,w1);
[vcm_phct,r9]=time_domain(vcm_phc,t,h,w1);
%Differential Mode capacitor voltages phase A,B,C
[vdm_phat,r10]=time_domain(vdm_pha,t,h,w1);
[vdm_phbt,r11]=time_domain(vdm_phb,t,h,w1);
[vdm_phct,r12]=time_domain(vdm_phc,t,h,w1);

subplot(4,1,1)
plot(t,ic_phat,t,ic_phbt,t,ic_phct)%circulatings currents in phase A,B,C
subplot(4,1,2)
plot(t,ig_phat,t,ig_phbt,t,ig_phct)%AC side currents phase A,B,C
subplot(4,1,3)
plot(t,vcm_phat,t,vcm_phbt,t,vcm_phct)%Common Mode capacitor voltages in phase A,B,C
subplot(4,1,4)
plot(t,vdm_phat,t,vdm_phbt,t,vdm_phct)%Differential Mode capacitor voltages phase A,B,C








%designing the control system in an integrated fashion so that the topology
%and the controller are inegrated in one state-space representation

ts=1/10000;t1=0.00005;kpc=1*10^(-4);kpo=1*10^(-4);kpv1=2;kpv2=2;kiv1=10;kiv2=10;

%creating the A_{c} matrix in HSS domain

v1=-1*6*(1/t1);v2=-1*12*(1/(t1*t1));v3=-1*(8/3)*(kpc/Vdc)*kiv1;v4=-1*(8/3)*(kpc/Vdc)*kiv2;v5=(8/3)*(1/ts)*(kpc/Vdc)*kpv1;v6=-1*(8/3)*(1/ts)*(kpc/Vdc)*kpv2;
vcosa=[0;0;0.5;0;0.5;0;0];vcosb=[0;0;0.5*exp(-((4*pi)/3)*1i);0;0.5*exp(((4*pi)/3)*1i);0;0];vcosc=[0;0;0.5*exp(-((2*pi)/3)*1i);0;0.5*exp(((2*pi)/3)*1i);0;0];

ac1=horzcat(v1*I,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,v2*I,O,O,O,O,O);
ac2=horzcat(O,v1*I,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,v2*I,O,O,O,O);
ac3=horzcat(O,O,v1*I,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,v2*I,O,O,O);


ac4=horzcat(O,O,O,v1*I,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,v2*I,O,O);
ac5=horzcat(O,O,O,O,v1*I,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,v2*I,O);
ac6=horzcat(O,O,O,O,O,v1*I,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,v2*I);


ac7=horzcat(O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,-1*I,O,O,O,O,O);
ac8=horzcat(O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,-1*I,O,O,O,O);
ac9=horzcat(O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,-1*I,O,O,O);


ac10=horzcat(O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,I,O,O);
ac11=horzcat(O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,I,O);
ac12=horzcat(O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,I);


ac13=horzcat(O,O,O,O,O,O,v3*I,O,O,v4*taut(vcosa,h),O,O,-1*(4/3)*(1/ts)*I,O,O,O,O,O,v5*I,O,O,v6*taut(vcosa,h),O,O);
ac14=horzcat(O,O,O,O,O,O,O,v3*I,O,O,v4*taut(vcosb,h),O,O,-1*(4/3)*(1/ts)*I,O,O,O,O,O,v5*I,O,O,v6*taut(vcosb,h),O);
ac15=horzcat(O,O,O,O,O,O,O,O,v3*I,O,O,v4*taut(vcosc,h),O,O,-1*(4/3)*(1/ts)*I,O,O,O,O,O,v5*I,O,O,v6*taut(vcosc,h));


ac16=horzcat(O,O,O,O,O,O,v3*I,O,O,v4*taut(vcosa,h),O,O,O,O,O,-1*(4/3)*(1/ts)*I,O,O,v5*I,O,O,v6*taut(vcosa,h),O,O);
ac17=horzcat(O,O,O,O,O,O,O,v3*I,O,O,v4*taut(vcosb,h),O,O,O,O,O,-1*(4/3)*(1/ts)*I,O,O,v5*I,O,O,v6*taut(vcosb,h),O);
ac18=horzcat(O,O,O,O,O,O,O,O,v3*I,O,O,v4*taut(vcosc,h),O,O,O,O,O,-1*(4/3)*(1/ts)*I,O,O,v5*I,O,O,v6*taut(vcosc,h));


ac19=horzcat(I,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O);
ac20=horzcat(O,I,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O);
ac21=horzcat(O,O,I,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O);

ac22=horzcat(O,O,O,I,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O);
ac23=horzcat(O,O,O,O,I,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O);
ac24=horzcat(O,O,O,O,O,I,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O);

ac=vertcat(ac1,ac2,ac3,ac4,ac5,ac6,ac7,ac8,ac9,ac10,ac11,ac12,ac13,ac14,ac15,ac16,ac17,ac18,ac19,ac20,ac21,ac22,ac23,ac24);

%creating the B_{c} matrix in HSS domain

bc1=horzcat(O,O,O,-1*v2*I,O,O,O,O,O,O,O,O);
bc2=horzcat(O,O,O,O,-1*v2*I,O,O,O,O,O,O,O);
bc3=horzcat(O,O,O,O,O,-1*v2*I,O,O,O,O,O,O);

bc4=horzcat(-1*v2*I,O,O,O,O,O,O,O,O,O,O,O);
bc5=horzcat(O,-1*v2*I,O,O,O,O,O,O,O,O,O,O);
bc6=horzcat(O,O,-1*v2*I,O,O,O,O,O,O,O,O,O);

bc7=horzcat(O,O,O,O,O,O,O,O,O,O,O,O);
bc8=horzcat(O,O,O,O,O,O,O,O,O,O,O,O);
bc9=horzcat(O,O,O,O,O,O,O,O,O,O,O,O);

bc10=horzcat(O,O,O,O,O,O,O,O,O,O,O,O);
bc11=horzcat(O,O,O,O,O,O,O,O,O,O,O,O);
bc12=horzcat(O,O,O,O,O,O,O,O,O,O,O,O);

bc13=horzcat(O,O,O,O,O,O,-1*(8/3)*(kpc/Vdc)*(1/ts)*I,O,O,(8/3)*(kpc/Vdc)*(1/ts)*I,O,O);
bc14=horzcat(O,O,O,O,O,O,O,-1*(8/3)*(kpc/Vdc)*(1/ts)*I,O,O,(8/3)*(kpc/Vdc)*(1/ts)*I,O);
bc15=horzcat(O,O,O,O,O,O,O,O,-1*(8/3)*(kpc/Vdc)*(1/ts)*I,O,O,(8/3)*(kpc/Vdc)*(1/ts)*I);

bc16=horzcat(O,O,O,O,O,O,1*(8/3)*(kpc/Vdc)*(1/ts)*I,O,O,(8/3)*(kpc/Vdc)*(1/ts)*I,O,O);
bc17=horzcat(O,O,O,O,O,O,O,1*(8/3)*(kpc/Vdc)*(1/ts)*I,O,O,(8/3)*(kpc/Vdc)*(1/ts)*I,O);
bc18=horzcat(O,O,O,O,O,O,O,O,1*(8/3)*(kpc/Vdc)*(1/ts)*I,O,O,(8/3)*(kpc/Vdc)*(1/ts)*I);

bc19=horzcat(O,O,O,O,O,O,O,O,O,O,O,O);
bc20=horzcat(O,O,O,O,O,O,O,O,O,O,O,O);
bc21=horzcat(O,O,O,O,O,O,O,O,O,O,O,O);

bc22=horzcat(O,O,O,O,O,O,O,O,O,O,O,O);
bc23=horzcat(O,O,O,O,O,O,O,O,O,O,O,O);
bc24=horzcat(O,O,O,O,O,O,O,O,O,O,O,O);

bc=vertcat(bc1,bc2,bc3,bc4,bc5,bc6,bc7,bc8,bc9,bc10,bc11,bc12,bc13,bc14,bc15,bc16,bc17,bc18,bc19,bc20,bc21,bc22,bc23,bc24);

%creating the B_{p1}*C_{c} matrix in HSS form

hc1=horzcat(O,O,O,O,O,O,-((2*kpc*kiv1)/(4*L*Vdc))*taut(vcm_pha,h),O,O,-1*((2*kpc*kiv2)/(4*L*Vdc))*taut((taut(vcm_pha,h)*vcosa),h),O,O,(1/(4*L))*taut(vdm_pha-vcm_pha,h),O,O,-1*(1/(4*L))*taut(vdm_pha+vcm_pha,h),O,O,...
((2*kpc*kpv1)/(4*L*Vdc))*taut(vcm_pha,h),O,O,-1*((2*kpc*kpv2)/(4*L*Vdc))*taut((taut(vcm_pha,h)*vcosa),h),O,O);
hc2=horzcat(O,O,O,O,O,O,O,-((2*kpc*kiv1)/(4*L*Vdc))*taut(vcm_phb,h),O,O,-1*((2*kpc*kiv2)/(4*L*Vdc))*taut((taut(vcm_phb,h)*vcosb),h),O,O,(1/(4*L))*taut(vdm_phb-vcm_phb,h),O,O,-1*(1/(4*L))*taut(vdm_phb+vcm_phb,h),O,O,...
((2*kpc*kpv1)/(4*L*Vdc))*taut(vcm_phb,h),O,O,-1*((2*kpc*kpv2)/(4*L*Vdc))*taut((taut(vcm_phb,h)*vcosb),h),O);
hc3=horzcat(O,O,O,O,O,O,O,O,-((2*kpc*kiv1)/(4*L*Vdc))*taut(vcm_phc,h),O,O,-1*((2*kpc*kiv2)/(4*L*Vdc))*taut((taut(vcm_phc,h)*vcosc),h),O,O,(1/(4*L))*taut(vdm_phc-vcm_phc,h),O,O,-1*(1/(4*L))*taut(vdm_phc+vcm_phc,h),O,O,...
((2*kpc*kpv1)/(4*L*Vdc))*taut(vcm_phc,h),O,O,-1*((2*kpc*kpv2)/(4*L*Vdc))*taut((taut(vcm_phc,h)*vcosc),h));

hc4=horzcat(O,O,O,O,O,O,-1*((2*kpc*kiv1)/(2*L*Vdc))*taut(vdm_pha,h),O,O,-1*((2*kpc*kiv2)/(2*L*Vdc))*taut((taut(vdm_pha,h)*vcosa),h),O,O,-1*(1/(2*L))*taut((vdm_pha+vcm_pha),h),O,O,1*(1/(2*L))*taut((-vdm_pha+vcm_pha),h)...
    ,O,O,((2*kpc*kpv1)/(2*Vdc*L))*taut(vdm_pha,h),O,O,-1*((2*kpc*kpv2)/(2*L*Vdc))*taut((taut(vdm_pha,h)*vcosa),h),O,O);
hc5=horzcat(O,O,O,O,O,O,O,-1*((2*kpc*kiv1)/(2*L*Vdc))*taut(vdm_phb,h),O,O,-1*((2*kpc*kiv2)/(2*L*Vdc))*taut((taut(vdm_phb,h)*vcosb),h),O,O,-1*(1/(2*L))*taut((vdm_phb+vcm_phb),h),O,O,1*(1/(2*L))*taut((-vdm_phb+vcm_phb),h)...
    ,O,O,((2*kpc*kpv1)/(2*Vdc*L))*taut(vdm_phb,h),O,O,-1*((2*kpc*kpv2)/(2*L*Vdc))*taut((taut(vdm_phb,h)*vcosb),h),O);
hc6=horzcat(O,O,O,O,O,O,O,O,-1*((2*kpc*kiv1)/(2*L*Vdc))*taut(vdm_phc,h),O,O,-1*((2*kpc*kiv2)/(2*L*Vdc))*taut((taut(vdm_phc,h)*vcosc),h),O,O,-1*(1/(2*L))*taut((vdm_phc+vcm_phc),h),O,O,1*(1/(2*L))*taut((-vdm_phc+vcm_phc),h)...
    ,O,O,((2*kpc*kpv1)/(2*Vdc*L))*taut(vdm_phc,h),O,O,-1*((2*kpc*kpv2)/(2*L*Vdc))*taut((taut(vdm_phc,h)*vcosc),h));

hc7=horzcat(O,O,O,O,O,O,1*((2*kpc*kiv1)/(Carm*Vdc))*taut(ic_pha,h),O,O,1*((2*kpc*kiv2)/(Carm*Vdc))*taut((taut(ic_pha,h)*vcosa),h),O,O,1*(1/(Carm))*taut((ic_pha+(ig_pha*0.5)),h),O,O,1*(1/(Carm))*taut((ic_pha-(ig_pha*0.5)),h)...
    ,O,O,-1*((2*kpc*kpv1)/(1*Vdc*Carm))*taut(ic_pha,h),O,O,1*((2*kpc*kpv2)/(1*Carm*Vdc))*taut((taut(ic_pha,h)*vcosa),h),O,O);
hc8=horzcat(O,O,O,O,O,O,O,1*((2*kpc*kiv1)/(Carm))*taut(ic_phb,h),O,O,1*((2*kpc*kiv2)/(Carm*Vdc))*taut((taut(ic_phb,h)*vcosb),h),O,O,1*(1/(Carm))*taut((ic_phb+(ig_phb*0.5)),h),O,O,1*(1/(Carm))*taut((ic_phb-(ig_phb*0.5)),h)...
    ,O,O,-1*((2*kpc*kpv1)/(1*Vdc*Carm))*taut(ic_phb,h),O,O,1*((2*kpc*kpv2)/(1*Carm*Vdc))*taut((taut(ic_phb,h)*vcosb),h),O);
hc9=horzcat(O,O,O,O,O,O,O,O,1*((2*kpc*kiv1)/(Carm))*taut(ic_phc,h),O,O,1*((2*kpc*kiv2)/(Carm*Vdc))*taut((taut(ic_phc,h)*vcosc),h),O,O,1*(1/(Carm))*taut((ic_phc+(ig_phc*0.5)),h),O,O,1*(1/(Carm))*taut((ic_phc-(ig_phc*0.5)),h)...
    ,O,O,-1*((2*kpc*kpv1)/(1*Vdc*Carm))*taut(ic_phc,h),O,O,1*((2*kpc*kpv2)/(1*Carm*Vdc))*taut((taut(ic_phc,h)*vcosc),h));

hc10=horzcat(O,O,O,O,O,O,1*((2*kpc*kiv1)/(Carm*Vdc*2))*taut(ig_pha,h),O,O,1*((2*kpc*kiv2)/(2*Carm*Vdc))*taut((taut(ig_pha,h)*vcosa),h),O,O,1*(1/(Carm))*taut((ic_pha+(ig_pha*0.5)),h),O,O,1*(1/(Carm))*taut((-ic_pha+(ig_pha*0.5)),h)...
    ,O,O,-1*((2*kpc*kpv1)/(2*Vdc*Carm))*taut(ig_pha,h),O,O,1*((2*kpc*kpv2)/(2*Carm*Vdc))*taut((taut(ig_pha,h)*vcosa),h),O,O);
hc11=horzcat(O,O,O,O,O,O,O,1*((2*kpc*kiv1)/(Carm*Vdc*2))*taut(ig_phb,h),O,O,1*((2*kpc*kiv2)/(2*Carm*Vdc))*taut((taut(ig_phb,h)*vcosb),h),O,O,1*(1/(Carm))*taut((ic_phb+(ig_phb*0.5)),h),O,O,1*(1/(Carm))*taut((-ic_phb+(ig_phb*0.5)),h)...
    ,O,O,-1*((2*kpc*kpv1)/(2*Vdc*Carm))*taut(ig_phb,h),O,O,1*((2*kpc*kpv2)/(2*Carm*Vdc))*taut((taut(ig_phb,h)*vcosb),h),O);
hc12=horzcat(O,O,O,O,O,O,O,O,1*((2*kpc*kiv1)/(Carm*Vdc*2))*taut(ig_phc,h),O,O,1*((2*kpc*kiv2)/(2*Carm*Vdc))*taut((taut(ig_phc,h)*vcosc),h),O,O,1*(1/(Carm))*taut((ic_phc+(ig_phc*0.5)),h),O,O,1*(1/(Carm))*taut((-ic_phc+(ig_phc*0.5)),h)...
    ,O,O,-1*((2*kpc*kpv1)/(2*Vdc*Carm))*taut(ig_phc,h),O,O,1*((2*kpc*kpv2)/(2*Carm*Vdc))*taut((taut(ig_phc,h)*vcosc),h));

hc=vertcat(hc1,hc2,hc3,hc4,hc5,hc6,hc7,hc8,hc9,hc10,hc11,hc12);

%building the topology A matrix

at1=horzcat((-1*(R/L))*I,O,O,O,O,O,-1*(taut(vcma,h)/(4*L)),O,O,(taut(vdma,h)/(4*L)),O,O);
at2=horzcat(O,(-1*(R/L))*I,O,O,O,O,O,-1*(taut(vcmb,h)/(4*L)),O,O,(taut(vdmb,h)/(4*L)),O);
at3=horzcat(O,O,(-1*(R/L))*I,O,O,O,O,O,-1*(taut(vcmc,h)/(4*L)),O,O,(taut(vdmc,h)/(4*L)));
at4=horzcat(O,O,O,(-1*(R/L))*I,O,O,(-1*taut(vdma,h))/(2*L),O,O,(-1*taut(vcma,h))/(2*L),O,O);
at5=horzcat(O,O,O,O,(-1*(R/L))*I,O,O,(-1*taut(vdmb,h))/(2*L),O,O,(-1*taut(vcmb,h))/(2*L),O);
at6=horzcat(O,O,O,O,O,(-1*(R/L))*I,O,O,(-1*taut(vdmc,h))/(2*L),O,O,(-1*taut(vcmc,h))/(2*L));
at7=horzcat(taut(vcma,h)/Carm,O,O,taut(vdma,h)/(2*Carm),O,O,O,O,O,O,O,O);
at8=horzcat(O,taut(vcmb,h)/Carm,O,O,taut(vdmb,h)/(2*Carm),O,O,O,O,O,O,O);
at9=horzcat(O,O,taut(vcmc,h)/Carm,O,O,taut(vdmc,h)/(2*Carm),O,O,O,O,O,O);
at10=horzcat(taut(vdma,h)/Carm,O,O,taut(vcma,h)/(2*Carm),O,O,O,O,O,O,O,O);
at11=horzcat(O,taut(vdmb,h)/Carm,O,O,taut(vcmb,h)/(2*Carm),O,O,O,O,O,O,O);
at12=horzcat(O,O,taut(vdmc,h)/Carm,O,O,taut(vcmc,h)/(2*Carm),O,O,O,O,O,O);
At=vertcat(at1,at2,at3,at4,at5,at6,at7,at8,at9,at10,at11,at12);

%building the B_{p1}*D_{c} matrix

bd1=horzcat(((2*kpc)/(Vdc*4*L))*taut(vcm_pha,h),O,O,((2*kpo)/(Vdc*4*L))*taut(vdm_pha,h),O,O,O,O,O,O,O,O);
bd2=horzcat(O,((2*kpc)/(Vdc*4*L))*taut(vcm_phb,h),O,O,((2*kpo)/(Vdc*4*L))*taut(vdm_phb,h),O,O,O,O,O,O,O);
bd3=horzcat(O,O,((2*kpc)/(Vdc*4*L))*taut(vcm_phc,h),O,O,((2*kpo)/(Vdc*4*L))*taut(vdm_phc,h),O,O,O,O,O,O);

bd4=horzcat(((2*kpc)/(Vdc*2*L))*taut(vdm_pha,h),O,O,((-1*2*kpo)/(Vdc*2*L))*taut(vcm_pha,h),O,O,O,O,O,O,O,O);
bd5=horzcat(O,((2*kpc)/(Vdc*2*L))*taut(vdm_phb,h),O,O,((-1*2*kpo)/(Vdc*2*L))*taut(vcm_phb,h),O,O,O,O,O,O,O);
bd6=horzcat(O,O,((2*kpc)/(Vdc*2*L))*taut(vdm_phc,h),O,O,((-1*2*kpo)/(Vdc*2*L))*taut(vcm_phc,h),O,O,O,O,O,O);

bd7=horzcat(((-1*2*kpc)/(Vdc*Carm))*taut(ic_pha,h),O,O,((2*kpo)/(Vdc*2*Carm))*taut(ig_pha,h),O,O,O,O,O,O,O,O);
bd8=horzcat(O,((-1*2*kpc)/(Vdc*Carm))*taut(ic_phb,h),O,O,((2*kpo)/(Vdc*2*Carm))*taut(ig_phb,h),O,O,O,O,O,O,O);
bd9=horzcat(O,O,((-1*2*kpc)/(Vdc*Carm))*taut(ic_phc,h),O,O,((2*kpo)/(Vdc*2*Carm))*taut(ig_phc,h),O,O,O,O,O,O);

bd10=horzcat(((-1*2*kpc)/(2*Vdc*Carm))*taut(ig_pha,h),O,O,((2*kpo)/(Vdc*1*Carm))*taut(ic_pha,h),O,O,O,O,O,O,O,O);
bd11=horzcat(O,((-1*2*kpc)/(2*Vdc*Carm))*taut(ic_phb,h),O,O,((2*kpo)/(Vdc*1*Carm))*taut(ig_phb,h),O,O,O,O,O,O,O);
bd12=horzcat(O,O,((-1*2*kpc)/(2*Vdc*Carm))*taut(ic_phc,h),O,O,((2*kpo)/(Vdc*1*Carm))*taut(ig_phc,h),O,O,O,O,O,O);

bd=vertcat(bd1,bd2,bd3,bd4,bd5,bd6,bd7,bd8,bd9,bd10,bd11,bd12);

alpha_mat=At+bd;

alpha1=horzcat(ac,bc);
alpha2=horzcat(hc,alpha_mat);
alpha=vertcat(alpha1,alpha2);


N=blkdiag(q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q,q);

Ahss=alpha-N;
und=max(real(eig(Ahss)));

BOU=zeros(8*3*dor,28);

BO1=horzcat((1/(2*L))*I,O,O,O);
BO2=horzcat(O,-1*(2/(L))*I,O,O);
BO3=horzcat(O,O,-1*(2/(L))*I,O);
BO4=horzcat(O,O,O,-1*(2/(L))*I);

BOL=zeros(42,28);

BO=vertcat(BOU,BO1,BO1,BO1,BO2,BO3,BO4,BOL);

D=zeros(252,28);
C=eye(252,252);

sys=ss(Ahss,BO,C,D);
% plot(real(eig(Ahss)),imag(eig(Ahss)),'o');


