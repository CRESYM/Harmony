clear all;
restoredefaultpath;
addpath(genpath('Init'));
addpath(genpath('..\..\#lib'));


CS = CaseStudyData();

clc


Fnom = 50;
w = 2*pi*Fnom;

%
% Parameters file for SPS model: HVDC_MMC.slx
%
%load sound                     % Sound file used by the Mode of Operation Panel
%
Fnom= 50;                      % Nominal system frequency (Hz)
Pnom= 1000e6;                  % Converter 3-phase rated power (MVA)
%Vnom_prim= 400e3;              % Nominal primary voltage (V)
Vnom_prim = 333e3; %333e3
Vnom_sec= 333e3;               % Nominal secondary voltage (V)
Nb_PM=36;                      % Number of power module per arm
Vnom_dc= 2*Vnom_sec;                % DC nominal voltage (V) [640e3]
C_PM= 1.758e-3; % Power module capacitor (F)
% Energy in kJ/MVA
W_kJ_MVA= 0.5 * C_PM * (Vnom_dc/Nb_PM)^2 * Nb_PM * 6 / (Pnom/1e6)/1e3;
Vc0_PM=0;                     % Capacitors initial voltage (V)

Vb_ac = sqrt(2/3)*Vnom_sec;
Ib_ac = (2/3)*Pnom/Vb_ac;%sqrt(2/3)*Pnom/Vnom_sec;
Vb_dc = Vnom_dc;
Ib_dc = Pnom/Vb_dc;

acdcV = 1/(Vnom_dc/2*sqrt(3)/sqrt(2)/Vnom_sec);
acdcI = Ib_dc/Ib_ac;
MMC_par.acdcV = acdcV;
MMC_par.acdcI = acdcI;

%%
% Sequencer timing:
Tbrk1_On=0.1;                 % Closing time of breaker 1 (converter energizing)
Tbrk2_On=1.0;                 % Closing time (s) of breaker 2 (across start-up resistor)
%
Tdeblock=1.5;                 % Converter de-block time (s)
Ton_VDCreg=1.5;               % VDC regulator turn-on time (s) - VDC Regulation
Tramping_Vdc_ref=2;           % Start time Vdc_ref ramping to nominal (s)
Slope_Vdc_ref=Vnom_dc/5;      % Sloge Vdc_ref ramping (V/s)
%
Ton_PQreg=2;                  % Preg & Qreg regulators turn-on time (s) - PQ regulation
Tramping_Pref=Ton_PQreg+0*0.2;  % Start time Pref ramping(s)
Slope_Pref=0.5;               % Sloge Pref ramping (V/s)
Tramping_Qref=Ton_PQreg+1.5;  % Start time Pref ramping(s)
Slope_Qref=0.5;               % Sloge Pref ramping (V/s)
%
Ton_Converter2=3;             % Converter 2 equivalent switched-on time (s)
%%
Tfault= 9999;             % DC Fault timing (s)
Rfault=1;                 % DC Fault resistance (Ohms)
%
%%
% PWM Output pulses selector
% pp=0;
% for p=1:2:72
%     pp=pp+1;
%     SelectPulses1(p)=pp;
%     SelectPulses1(p+1)=pp+36;
% end
%
%
% Transformer impedance
Lxfo= 0.12;         % Total Leakage inductance (pu)
Rxfo= 0.003;        % Total winding resistance (pu)
%
%Zbase= Vnom_sec^2/Pnom;
%
Larm_pu=0.15;
Rarm_pu=Larm_pu/100;
%
Zb_ac= Vb_ac/Ib_ac;
Zb_dc = Vnom_dc/Ib_dc/2;
%Zb_ac= Vnom_sec^2/Pnom;
Larm=Larm_pu*(Zb_ac/(2*pi*Fnom));
Rarm=Rarm_pu*Zb_ac;

Lt=Lxfo*(Zb_ac/(2*pi*Fnom));
Rt=Rxfo*Zb_ac;

w=2*pi*Fnom;
wc2=(2*w)^2;
Cfilter=1/(Larm*wc2);      % Capacitor value for 2th harmonic filter(F)
Rfilter=1/(Cfilter*w)*30;  % Resistance value for 2th harmonic filter (Ohm)
Topen_Filter=1e6;          % Breaker opening time for second-harmonic filters (s)
%
% *****************************************************************
%                         CONTROL PARAMETERS
% *****************************************************************
%
% Modulator Parameters
Fc=Fnom*3.37;        % Carriers frequency (Hz)
%
% dq and Vdc measurement filter cut-off frequency:
Fn_filter=1000;
Zeta_filter=1;
%
% Active power regulator (Preg)
Kp_Preg= 0.5/3;                % Proportional gain
Ki_Preg= 1.0;                  % Integral gain
Limits_Preg = [ 1.2, 0.8 ] ;   % Output (Vdc_ref) Upper/Lower limits (pu)

%
% Reactive power regulator (Qreg)
Kp_Qreg= 0.5/3;                % Proportional gain
Ki_Qreg= 1.0;                  % Integral gain
Limits_Qreg = [ 0.25, -0.25 ]; % Output (Iq_ref) Upper/Lower limit (pu)
%
% VDC regulator (VDCreg)
Kp_VDCreg=4;                   % Proportional gain
Ki_VDCreg=100;                 % Integral gain
Limits_VDCreg= [ 2.0  -2.0];   % Output Idref [Upper Lower] limits (pu)
%
% Current regulator (Ireg)
Kp_Ireg= 0.6;                  % Proportional gain
Ki_Ireg= 6;                    % Integral gain
Limits_Ireg= [ 2.0  -2.0];     % Output Vdq_conv [Upper Lower] limits (pu)
% Feedforward coefficients:
Lff=Larm_pu/2+Lxfo; %Larm_pu/2;
Rff= Rarm_pu/2+Rxfo; %Rarm_pu/2;
%
% ******************************
% Power system parameters
% ******************************
%
Psc= Pnom*20;     % Short circuit power (MVA)
X_R= 7;           % X/R ratio
P_Ld1= Psc/30;   % load (primary bus) (MW)
R_startup= 400;   % Startup resistance (Ohm)
%
% Cable data
R_cable = 0.5;      % ohm
L_cable= 15e-3;   % (H)
%
% Grounding reference (series RC)
Rg= 100;              % (Ohms)
Cg= 50e-9;            % (F)

% ******************************
% CCSC parameters
% ******************************
Kp_CCSC = 0.8;
Ki_CCSC = 50;

Carm = C_PM/Nb_PM;

w=2*pi*Fnom;
Larm_pu=0.15;
Rarm_pu=Larm_pu/100;
Carm_pu = (Carm*Zb_ac);

Larm=Larm_pu*(Zb_ac/w);
Rarm=Rarm_pu*Zb_ac;

Larm_pu_0 = Larm_pu/w;
Rarm_pu_0 = Rarm_pu;


file = 'AC_Grid.xlsx';
[DCss, rlc] = fNET2SS(file);

% file = 'NET.xlsx';
% 
% bus = xlsread(file,'bus');
% line = xlsread(file,'line');
% mac_con = xlsread(file,'mac_con');
% turb = xlsread(file,'turb');
% convOrig = (xlsread(file,'conv'))';
% 
% 
% [bus_sol, line] = loadflow(bus,line,1e-5,30,1.0,'y',2,1);
% 
% NET = fGetNet( bus, line, w , 1);
% [lstCONV]  = fGetConvLst( convOrig, bus_sol, CS.wn);  

% dcbus = xlsread('DCNET.xlsx','bus');
% dcline = xlsread('DCNET.xlsx','line');
% 
% 
% DCNET = fGetNet( dcbus, dcline, 0 , 1);
% DCNET.C = -DCNET.C;
% DCNET.D = -DCNET.D;
% 
% DCNET.InputName = {'Idc','Vdc_e'};
% DCNET.OutputName = {'Vdc','Icable'};



% MMCs = lstCONV{1};
% MMC  = MMCs;
% 
% 
% Pac = MMCs.Pac;
% Qac = MMCs.Qac;
% Vac = MMCs.Vac;
% 
% 
DCM = 1;
QCM = 0;

% %PLL = lstCONV{1}.PLL
% 
% 
% % L11
% L11=zeros(8,8);
% % L12
% L12=zeros(8,8);
% % L21
% L21=zeros(8,8);
% % L22
% % L22
% L22=zeros(8,8);
% L22(1,2)=(1/(Carm*w)); L22(2,1)=(-1/(Carm*w)); L22(3,4)=(-1/(2*Carm*w)); L22(4,3)=(1/(2*Carm*w)); L22(5,6)=(-1/(3*Carm*w)); L22(6,5)=(1/(3*Carm*w)); L22(7,8)=(-1/(4*Carm*w)); L22(8,7)=(1/(4*Carm*w)); L22(8,7)=(1/(4*Carm*w)); 
% 
% 
% InputName = {'Zd1+','Zq1+','Zd2-','Zq2-','Zd3-','Zq3-','Zd4-','Zq4-'};
% OutputName = {'iZd1+','iZq1+','iZd2-','iZq2-','iZd3-','iZq3-','iZd4-','iZq4-'};
% 
% Int = ss(L11,L12,L21,L22);
% Int.InputName = InputName;
% Int.OutputName = OutputName;
% 
% 
% Z_Vp = DQ_mult_neg_dq(2.29, [-0.05387, -0.06347], [-0.1171,-0.01247], 0.5, -0.4512,-0.05148);

%XXX = CCM_AutoConnect({NET,MMC.SS,DCNET})


Ws = 2*pi*Fnom;
HRM = 2;
HrmOrd = HRM;


Ts=50e-6;
Ts_Power= Ts;    % SPS Simulation time step(s)
Ts_Control=Ts;   % Control system time step (s)
Ts_EMT = Ts;
Ts_DQN = Ts;

Pac = 0.9
Qac = 0.2


DC_cable_R_shunt = 1000;
DC_cable_C_shunt = 0.1e-6;



% open_system('HVDC_MMC1_C');
% set_param('HVDC_MMC1_C', 'DecoupledContinuousIntegration', 'on')


