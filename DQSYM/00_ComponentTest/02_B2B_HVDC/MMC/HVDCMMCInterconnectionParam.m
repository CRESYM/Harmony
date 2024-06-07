clear all;
close all;
restoredefaultpath;
addpath(genpath('..\..\..\#lib'));

%pssp
% Parameters file for SPS model: HVDC_MMC.slx
%
%
Fnom= 50;                      % Nominal system frequency (Hz)
Pnom= 1000e6;                  % Converter 3-phase rated power (MVA)
Vnom_prim= 400e3;              % Nominal primary voltage (V)
Vnom_sec= 333e3;               % Nominal secondary voltage (V)
Nb_PM=36;                      % Number of power module per arm
Vnom_dc= 640e3;                % DC nominal voltage (V)
C_PM= 1.758e-3; % Power module capacitor (F)
% Energy in kJ/MVA
W_kJ_MVA= 0.5 * C_PM * (Vnom_dc/Nb_PM)^2 * Nb_PM * 6 / (Pnom/1e6)/1e3;
Vc0_PM=0;                     % Capacitors initial voltage (V)
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
Ton_PQreg=4;                  % Preg & Qreg regulators turn-on time (s) - PQ regulation
Tramping_Pref=Ton_PQreg+0.2;  % Start time Pref ramping(s)
Slope_Pref=0.5;               % Sloge Pref ramping (V/s)
Tramping_Qref=Ton_PQreg+3.5;  % Start time Pref ramping(s)
Slope_Qref=0.5;               % Sloge Pref ramping (V/s)
%
Ton_Converter2=4;             % Converter 2 equivalent switched-on time (s)
%%
Tfault= 9999;             % DC Fault timing (s)
Rfault=1;                 % DC Fault resistance (Ohms)
%
%%
% PWM Output pulses selector
pp=0;
for p=1:2:72
    pp=pp+1;
    SelectPulses1(p)=pp;
    SelectPulses1(p+1)=pp+36;
end
%
Ts_Power= 20e-6;    % SPS Simulation time step(s)
Ts_Control=40e-6;   % Control system time step (s)
Ts=Ts_Control;
%
% Transformer impedance
Lxfo= 0.12;         % Total Leakage inductance (pu)
Rxfo= 0.003;        % Total winding resistance (pu)
%
Zbase= Vnom_sec^2/Pnom;
%
Larm_pu=0.15;
Rarm_pu=Larm_pu/100;
%
Zbase= Vnom_sec^2/Pnom;
Larm=Larm_pu*(Zbase/(2*pi*Fnom));
Rarm=Rarm_pu*Zbase;
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
Lff=Larm_pu/2;
Rff= Rarm_pu/2;


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

D_R  = 0.001;
D_Vf = 0;
D_Rs = 1e8; %1e4
D_Cs = inf;



%%% NEW


Vb_ac = sqrt(2/3)*Vnom_sec;
Ib_ac = (2/3)*Pnom/Vb_ac;%sqrt(2/3)*Pnom/Vnom_sec;
Vb_dc = Vnom_dc;
Ib_dc = Pnom/Vb_dc;

Carm = C_PM/Nb_PM;

Zb_ac= Vb_ac/Ib_ac;
Zb_dc = Vnom_dc/Ib_dc/2;
%Zb_ac= Vnom_sec^2/Pnom;
Larm=Larm_pu*(Zb_ac/(2*pi*Fnom));
Rarm=Rarm_pu*Zb_ac;

HRM = 2;

load('ACDC')
load('S')

% % g = Simulink.Mask.get('DQGEN2/Mac_EMT');
% S  = GetDataForEquivalentModel('Discrete',gcbh);
% 
% DSS = Simulink.Mask.get('HVDCMMCInterconnection/DSS');
% DSS = DSS.getWorkspaceVariables;
% 
% Ad1 = DSS(2).Value;
% Bd1 = DSS(4).Value;
% Cd1 = DSS(6).Value;
% Dd1 = DSS(9).Value;

% file = 'AC_Grid.xlsx';
% [SYS, rlc] = fNET2SS(file);
% Simulink.BlockDiagram.getAlgebraicLoops(mdl);
% sps_AC = power_analyze('HVDCMMCInterconnection','structure')
% sps_DC = power_analyze('HVDCMMCInterconnection','structure')

% Limits_Preg = Limits_Preg * 5;
% Limits_Qreg = Limits_Qreg * 2;
% Limits_VDCreg = Limits_VDCreg * 2;
% Limits_Ireg = Limits_Ireg * 2;
% 
% 
% Ts = 10e-6;
% Ts_Power = Ts;
% Ts_Control = Ts;

% sps = power_analyze('power_circ2ss_sps','structure')
% [A,B,C,D,x0,states,inputs,outputs,uss,xss,yss,frequencies,Hlin] =...
% power_analyze('sys')
% sps = power_analyze('sys','sort')
% sps = power_analyze('sys','ss')
% power_analyze('sys','net')
% SW = power_analyze('sys','getSwitchStatus')
% sps = power_analyze('sys','setSwitchStatus',SW)
