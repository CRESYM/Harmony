%Steady State Response of MMC
clc
clear all
%Defining the parameters to be used in the code
P=50*10^(6);%Rated Power
f=50;%Rated Frequency
w1=2*pi*f;
Vdc=320*10^(3);%Rated DC Voltage
Vac=166*10^(3);%Rated AC Voltage
N=20;%submodule number per arm
Csm=140*10^(-6);%submodule capacitor
Carm=Csm/N;
L=360*10^(-3);%arm inductance
R=1;%arm resistance
zl=360;%load from rated power and assumed power factor of 0.8 lag
%Defining the switching functions
%generation of an arbitary switching signal
Fs = 1000; % sampling rate of 1000 Hz
t = 0:0.1/Fs:0.02; %creates a 1x1000 vector from 0 to (1-1/Fs)
t=transpose(t);
m1=(Vac/(sqrt(3)))/(Vdc/2);m2=0;
%nu=0.5*(1-(m1*cos(2*pi*f*t+(pi/3)))-(m2*cos(4*pi*f*t+(pi/3)))); 
%nl=0.5*(1+(m1*cos(2*pi*f*t+(pi/3)))-(m2*cos(4*pi*f*t+(pi/3))));
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
vau=zeros(dor,1);vbu=zeros(dor,1);vcu=zeros(dor,1);%upper leg voltages
val=zeros(dor,1);vbl=zeros(dor,1);vcl=zeros(dor,1);%lower leg voltages
iga=zeros(dor,1);igb=zeros(dor,1);igc=zeros(dor,1);%AC side currents
x=vertcat(ica,icb,icc,vau,vbu,vcu,val,vbl,vcl,iga,igb,igc);%state variable matrix
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
alpha=(R+(2*zl))/L;
%building the B matrix in HSS domain
%B 1st row
B1=(1/(2*L))*I;
%B 2nd row
B2=(1/(2*L))*I;
%B 1st row
B3=(1/(2*L))*I;
%B other rows
B4=zeros(sizx-(3*dor),dor);
B=vertcat(B1,B2,B3,B4);
%creating upper switching insertion vectors
nua=[0;0;-1*(m1/4);0.5;-1*(m1/4);0;0];nub=[0;0;(m1*(1-(sqrt(3)*1i)))/8;0.5;(m1*(1+(sqrt(3)*1i)))/8;0;0];nuc=[0;0;(m1*(1+(sqrt(3)*1i)))/8;0.5;(m1*(1-(sqrt(3)*1i)))/8;0;0];
%creating lower switching insertion vectors
nla=[0;0;m1/4;0.5;m1/4;0;0];nlb=[0;0;(m1*(-1+(sqrt(3)*1i)))/8;0.5;(-1*m1*((1+(sqrt(3)*1i)))/8);0;0];nlc=[0;0;(-1*m1*((1+(sqrt(3)*1i)))/8);0.5;(m1*(-1+(sqrt(3)*1i)))/8;0;0];
%building the A matrix in HSS domain
%THE  Block A1
A11=-(R/L)*I-q;A12=O;A13=O;A14=taut(nua,h)/(-1*2*L);A15=O;A16=O;A17=taut(nla,h)/(-1*2*L);A18=O;A19=O;A110=O;A111=O;A112=O;
A1=horzcat(A11,A12,A13,A14,A15,A16,A17,A18,A19,A110,A111,A112);
%THE  Block A2
A22=-(R/L)*I-q;A21=O;A23=O;A25=taut(nub,h)/(-1*2*L);A24=O;A26=O;A28=taut(nlb,h)/(-1*2*L);A27=O;A29=O;A210=O;A211=O;A212=O;
A2=horzcat(A21,A22,A23,A24,A25,A26,A27,A28,A29,A210,A211,A212);
%THE  Block A3
A33=-(R/L)*I-q;A32=O;A31=O;A36=taut(nuc,h)/(-1*2*L);A34=O;A35=O;A39=taut(nlc,h)/(-1*2*L);A37=O;A38=O;A310=O;A311=O;A312=O;
A3=horzcat(A31,A32,A33,A34,A35,A36,A37,A38,A39,A310,A311,A312);
%THE  Block A4
A41=taut(nua,h)/Carm;A42=O;A43=O;A44=-1*q;A45=O;A46=O;A47=O;A48=O;A49=O;A410=taut(nua,h)/(2*Carm);A411=O;A412=O;
A4=horzcat(A41,A42,A43,A44,A45,A46,A47,A48,A49,A410,A411,A412);
%THE  Block A5
A52=taut(nub,h)/Carm;A51=O;A53=O;A55=-1*q;A54=O;A56=O;A58=O;A57=O;A59=O;A510=O;A511=taut(nub,h)/(2*Carm);A512=O;
A5=horzcat(A51,A52,A53,A54,A55,A56,A57,A58,A59,A510,A511,A512);
%THE  Block A6
A62=O;A61=O;A63=taut(nuc,h)/Carm;A66=-1*q;A64=O;A65=O;A68=O;A67=O;A69=O;A610=O;A612=taut(nuc,h)/(2*Carm);A611=O;
A6=horzcat(A61,A62,A63,A64,A65,A66,A67,A68,A69,A610,A611,A612);
%THE  Block A7
A71=taut(nla,h)/Carm;A72=O;A73=O;A77=-1*q;A75=O;A76=O;A74=O;A78=O;A79=O;A710=taut(nla,h)/(-1*2*Carm);A711=O;A712=O;
A7=horzcat(A71,A72,A73,A74,A75,A76,A77,A78,A79,A710,A711,A712);
%THE  Block A8
A82=taut(nlb,h)/Carm;A81=O;A83=O;A88=-1*q;A84=O;A86=O;A85=O;A87=O;A89=O;A810=O;A811=taut(nlb,h)/(-1*2*Carm);A812=O;
A8=horzcat(A81,A82,A83,A84,A85,A86,A87,A88,A89,A810,A811,A812);
%THE  Block A9
A92=O;A91=O;A93=taut(nlc,h)/Carm;A99=-1*q;A94=O;A95=O;A98=O;A97=O;A96=O;A910=O;A912=taut(nlc,h)/(-1*2*Carm);A911=O;
A9=horzcat(A91,A92,A93,A94,A95,A96,A97,A98,A99,A910,A911,A912);
%THE  Block A10
A102=O;A101=O;A103=O;A109=O;A104=taut(nua,h)/(-1*L);A105=O;A108=O;A107=taut(nla,h)/L;A106=O;A1010=(-1*alpha*I)-q;A1012=O;A1011=O;
A10=horzcat(A101,A102,A103,A104,A105,A106,A107,A108,A109,A1010,A1011,A1012);
%THE  Block A10
A112=O;A111=O;A113=O;A119=O;A115=taut(nub,h)/(-1*L);A114=O;A117=O;A118=taut(nlb,h)/L;A116=O;A1111=(-1*alpha*I)-q;A1112=O;A1110=O;
A11=horzcat(A111,A112,A113,A114,A115,A116,A117,A118,A119,A1110,A1111,A1112);
%THE  Block A12
A122=O;A121=O;A123=O;A124=O;A126=taut(nuc,h)/(-1*L);A125=O;A128=O;A129=taut(nlc,h)/L;A127=O;A1212=(-1*alpha*I)-q;A1210=O;A1211=O;
A12=horzcat(A121,A122,A123,A124,A125,A126,A127,A128,A129,A1210,A1211,A1212);
%A matrix
A=vertcat(A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12);
%steady state response
xss=-1*(A\B)*u;
%Obtaining the time domain response
xc_pha=xss(1:(dor),1);
xc_phb=xss(dor+1:(2*dor),1);
xc_phc=xss((2*dor)+1:(3*dor),1);
xcuv_pha=xss((3*dor)+1:(4*dor),1);
xcuv_phb=xss((4*dor)+1:(5*dor),1);
xcuv_phc=xss((5*dor)+1:(6*dor),1);
xclv_pha=xss((6*dor)+1:(7*dor),1);
xclv_phb=xss((7*dor)+1:(8*dor),1);
xclv_phc=xss((8*dor)+1:(9*dor),1);
xiga=xss((9*dor)+1:(10*dor),1);
xigb=xss((10*dor)+1:(11*dor),1);
xigc=xss((11*dor)+1:(12*dor),1);

%circulatings currents in phase A,B,C
[xc_phat,a1]=time_domain(xc_pha,t,h,w1);
[xc_phbt,a2]=time_domain(xc_phb,t,h,w1);
[xc_phct,a3]=time_domain(xc_phc,t,h,w1);
%capacitor voltage in the upper leg in phase A,B,C
[xcuv_phat,a4]=time_domain(xcuv_pha,t,h,w1);
[xcuv_phbt,a5]=time_domain(xcuv_phb,t,h,w1);
[xcuv_phct,a6]=time_domain(xcuv_phc,t,h,w1);
%capacitor voltage in the lower leg in phase A,B,C
[xclv_phat,a7]=time_domain(xclv_pha,t,h,w1);
[xclv_phbt,a8]=time_domain(xclv_phb,t,h,w1);
[xclv_phct,a9]=time_domain(xclv_phc,t,h,w1);
%AC side phase currents in phase A,B,C
[xigat,a10]=time_domain(xiga,t,h,w1);
[xigbt,a11]=time_domain(xigb,t,h,w1);
[xigct,a12]=time_domain(xigc,t,h,w1);

xcsum_t = (xcuv_phat + xclv_phat)/2;
xcdiff_t = (xcuv_phat - xclv_phat)/2;

subplot(4,1,1)
plot(t,xc_phat,t,xc_phbt,t,xc_phct)%circulatings currents in phase A,B,C
subplot(4,1,2)
plot(t,xcsum_t,t,xcuv_phbt,t,xcuv_phct)%capacitor voltage in the upper leg in phase A,B,C
subplot(4,1,3)
plot(t,xcdiff_t,t,xclv_phbt,t,xclv_phct)%capacitor voltage in the lower leg in phase A,B,C
subplot(4,1,4)
plot(t,xigat,t,xigbt,t,xigct)%AC side phase currents in phase A,B,C

%Introducing Control Loop for AC voltage Control

kp=40;kr=10;kf=20;%control parameters of the PR controller 

%defining variables for the controller 
%SET for fc1x
%SET 1
fc1a=(((kf-kp)*(xcuv_pha-xclv_pha))/(2*L*Vdc))*zl;
fc1b=(((kf-kp)*(xcuv_phb-xclv_phb))/(2*L*Vdc))*zl;
fc1c=(((kf-kp)*(xcuv_phc-xclv_phc))/(2*L*Vdc))*zl;
%SET 2
fc2a=(xcuv_pha-xclv_pha)/(2*L*Vdc);
fc2b=(xcuv_phb-xclv_phb)/(2*L*Vdc);
fc2c=(xcuv_phc-xclv_phc)/(2*L*Vdc);
%SET 3
fc3a=kp*((xcuv_pha-xclv_pha)/(2*L*Vdc));
fc3b=kp*((xcuv_phb-xclv_phb)/(2*L*Vdc));
fc3c=kp*((xcuv_phc-xclv_phc)/(2*L*Vdc));
%SET for Fvu1x
%SET 1
fvu1a=((nua)/(2*Carm))-(((kf-kp)/(Carm*Vdc))*(xc_pha+(xiga/2))*zl);
fvu1b=((nub)/(2*Carm))-(((kf-kp)/(Carm*Vdc))*(xc_phb+(xigb/2))*zl);
fvu1c=((nuc)/(2*Carm))-(((kf-kp)/(Carm*Vdc))*(xc_phc+(xigc/2))*zl);
%SET 2
fvu2a=-1*(1/(Carm*Vdc))*(xc_pha+(xiga/2));
fvu2b=-1*(1/(Carm*Vdc))*(xc_phb+(xigb/2));
fvu2c=-1*(1/(Carm*Vdc))*(xc_phc+(xigc/2));
%SET 3
fvu3a=kp*fvu2a;
fvu3b=kp*fvu2b;
fvu3c=kp*fvu2c;
%SET for Fvl1x
%SET 1
fvl1a=((nua)/(2*Carm))+(((kf-kp)/(Carm*Vdc))*(xc_pha-(xiga/2))*zl);
fvl1b=((nub)/(2*Carm))+(((kf-kp)/(Carm*Vdc))*(xc_phb-(xigb/2))*zl);
fvl1c=((nuc)/(2*Carm))+(((kf-kp)/(Carm*Vdc))*(xc_phc-(xigc/2))*zl);
%SET 2
fvl2a=1*(1/(Carm*Vdc))*(xc_pha-(xiga/2));
fvl2b=1*(1/(Carm*Vdc))*(xc_phb-(xigb/2));
fvl2c=1*(1/(Carm*Vdc))*(xc_phc-(xigc/2));
%SET 3
fvl3a=kp*fvl2a;
fvl3b=kp*fvl2b;
fvl3c=kp*fvl2c;
%SET for Fi1x
%SET 1
fi1a=-((R+(2*zl))/L)+((kf-kp)*(xcuv_pha+xclv_pha)*zl)/(L*Vdc);
fi1b=-((R+(2*zl))/L)+((kf-kp)*(xcuv_phb+xclv_phb)*zl)/(L*Vdc);
fi1c=-((R+(2*zl))/L)+((kf-kp)*(xcuv_phc+xclv_phc)*zl)/(L*Vdc);
%SET 2
fi2a=(xcuv_pha+xclv_pha)/(L*Vdc);
fi2b=(xcuv_phb+xclv_phb)/(L*Vdc);
fi2c=(xcuv_phc+xclv_phc)/(L*Vdc);
%SET 3
fi3a=kp*fi2a;
fi3b=kp*fi2b;
fi3c=kp*fi2c;

%B matrix for small signal model

bss1=horzcat((1/(2*L))*I,taut(fc3a,h),O,O);
bss2=horzcat((1/(2*L))*I,O,taut(fc3b,h),O);
bss3=horzcat((1/(2*L))*I,O,O,taut(fc3c,h));
bss4=horzcat(O,taut(fvu3a,h),O,O);
bss5=horzcat(O,O,taut(fvu3b,h),O);
bss6=horzcat(O,O,O,taut(fvu3c,h));
bss7=horzcat(O,taut(fvl3a,h),O,O);
bss8=horzcat(O,O,taut(fvl3b,h),O);
bss9=horzcat(O,O,O,taut(fvl3c,h));
bss10=horzcat(O,taut(fi3a,h),O,O);
bss11=horzcat(O,O,taut(fi3b,h),O);
bss12=horzcat(O,O,O,taut(fi3c,h));
bss13=horzcat(O,kr*I,O,O);
bss14=horzcat(O,O,O,O);
bss15=horzcat(O,O,kr*I,O);
bss16=horzcat(O,O,O,O);
bss17=horzcat(O,O,O,kr*I);
bss18=horzcat(O,O,O,O);
bss=vertcat(bss1,bss2,bss3,bss4,bss5,bss6,bss7,bss8,bss9,bss10,bss11,bss12,bss13,bss14,bss15,bss16,bss17,bss18);

%A matrix for small signal analysis

ass1=horzcat((-1*(R/L)*I)-q,O,O,-1*(taut(nua,h)/(2*L)),O,O,-1*(taut(nla,h)/(2*L)),O,O,taut(fc1a,h),O,O,taut(fc2a,h),O,O,O,O,O);
ass2=horzcat(O,(-1*(R/L)*I)-q,O,O,-1*(taut(nub,h)/(2*L)),O,O,-1*(taut(nlb,h)/(2*L)),O,O,taut(fc1b,h),O,O,O,taut(fc2b,h),O,O,O);
ass3=horzcat(O,O,(-1*(R/L)*I)-q,O,O,-1*(taut(nuc,h)/(2*L)),O,O,-1*(taut(nlc,h)/(2*L)),O,O,taut(fc1c,h),O,O,O,O,taut(fc2c,h),O);
ass4=horzcat(taut(nua,h)/Carm,O,O,-1*q,O,O,O,O,O,taut(fvu1a,h),O,O,taut(fvu2a,h),O,O,O,O,O);
ass5=horzcat(O,taut(nub,h)/Carm,O,O,-1*q,O,O,O,O,O,taut(fvu1b,h),O,O,O,taut(fvu2b,h),O,O,O);
ass6=horzcat(O,O,taut(nuc,h)/Carm,O,O,-1*q,O,O,O,O,O,taut(fvu1c,h),O,O,O,O,taut(fvu2c,h),O);
ass7=horzcat(taut(nla,h)/Carm,O,O,O,O,O,-1*q,O,O,taut(fvl1a,h),O,O,taut(fvl2a,h),O,O,O,O,O);
ass8=horzcat(O,taut(nlb,h)/Carm,O,O,O,O,O,-1*q,O,O,taut(fvl1b,h),O,O,O,taut(fvl2b,h),O,O,O);
ass9=horzcat(O,O,taut(nlc,h)/Carm,O,O,O,O,O,-1*q,O,O,taut(fvl1c,h),O,O,O,O,taut(fvl2c,h),O);
ass10=horzcat(O,O,O,-1*(taut(nua,h)/(2*L)),O,O,-1*(taut(nla,h)/(2*L)),O,O,taut(fi1a,h)-q,O,O,taut(fi2a,h),O,O,O,O,O);
ass11=horzcat(O,O,O,O,-1*(taut(nub,h)/(2*L)),O,O,-1*(taut(nlb,h)/(2*L)),O,O,taut(fi1b,h)-q,O,O,O,taut(fi2b,h),O,O,O);
ass12=horzcat(O,O,O,O,O,-1*(taut(nuc,h)/(2*L)),O,O,-1*(taut(nlc,h)/(2*L)),O,O,taut(fi1c,h)-q,O,O,O,O,taut(fi2c,h),O);
ass13=horzcat(O,O,O,O,O,O,O,O,O,-1*kr*zl*I,O,O,-1*q,-1*(w1^2)*I,O,O,O,O);
ass14=horzcat(O,O,O,O,O,O,O,O,O,O,O,O,I,-1*q,O,O,O,O);
ass15=horzcat(O,O,O,O,O,O,O,O,O,O,-1*kr*zl*I,O,O,O,-1*q,-1*(w1^2)*I,O,O);
ass16=horzcat(O,O,O,O,O,O,O,O,O,O,O,O,O,O,I,-1*q,O,O);
ass17=horzcat(O,O,O,O,O,O,O,O,O,O,O,-1*kr*zl*I,O,O,O,O,-1*q,-1*(w1^2)*I);
ass18=horzcat(O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,I,-1*q);
ass=vertcat(ass1,ass2,ass3,ass4,ass5,ass6,ass7,ass8,ass9,ass10,ass11,ass12,ass13,ass14,ass15,ass16,ass17,ass18);

%D matrix for small signal analysis

dss1=horzcat(O,-1*(kp/Vdc)*I,O,O);
dss2=horzcat(O,O,-1*(kp/Vdc)*I,O);
dss3=horzcat(O,O,O,-1*(kp/Vdc)*I);
dss4=horzcat(O,(kp/Vdc)*I,O,O);
dss5=horzcat(O,O,(kp/Vdc)*I,O);
dss6=horzcat(O,O,O,(kp/Vdc)*I);
dss=vertcat(dss1,dss2,dss3,dss4,dss5,dss6);

%C matrix for small signal analysis

css1=horzcat(O,O,O,O,O,O,O,O,O,((kp-kf)/Vdc)*zl*I,O,O,-1*(1/Vdc)*I,O,O,O,O,O);
css2=horzcat(O,O,O,O,O,O,O,O,O,O,((kp-kf)/Vdc)*zl*I,O,O,-1*(1/Vdc)*I,O,O,O,O);
css3=horzcat(O,O,O,O,O,O,O,O,O,O,O,((kp-kf)/Vdc)*zl*I,O,O,-1*(1/Vdc)*I,O,O,O);
css4=horzcat(O,O,O,O,O,O,O,O,O,-1*((kp-kf)/Vdc)*zl*I,O,O,1*(1/Vdc)*I,O,O,O,O,O);
css5=horzcat(O,O,O,O,O,O,O,O,O,O,-1*((kp-kf)/Vdc)*zl*I,O,O,1*(1/Vdc)*I,O,O,O,O);
css6=horzcat(O,O,O,O,O,O,O,O,O,O,O,-1*((kp-kf)/Vdc)*zl*I,O,O,1*(1/Vdc)*I,O,O,O);
css=vertcat(css1,css2,css3,css4,css5,css6);


sys1=ss(ass,bss,css,dss);


TF=tf(sys1);
pf=participation_factor(ass);

xax=1:1:126;
plot(xax,pf(:,1),xax,pf(:,2),xax,pf(:,3));




