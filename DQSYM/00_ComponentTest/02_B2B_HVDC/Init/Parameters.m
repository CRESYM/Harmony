MMC_par = struct();

MMC_par.Fnom= 50;                      % Nominal system frequency (Hz)
MMC_par.Pnom= 1000e6;                  % Converter 3-phase rated power (MVA)
MMC_par.Vnom_prim= 400e3;              % Nominal primary voltage (V)
MMC_par.w = 2*pi*MMC_par.Fnom;

MMC_par.Vnom_sec = 400e3;               % Nominal secondary voltage (V) [333e3]
MMC_par.Vb_ac = sqrt(2/3)*MMC_par.Vnom_sec;
%MMC_par.Ib_ac = sqrt(2/3)*MMC_par.Pnom/MMC_par.Vnom_sec;
MMC_par.Ib_ac = (2/3)*MMC_par.Pnom/MMC_par.Vb_ac;

MMC_par.Nb_PM=36;                      % Number of power module per arm

MMC_par.Vnom_dc = 2*MMC_par.Vnom_sec;                % DC nominal voltage (V)
MMC_par.Ib_dc = MMC_par.Pnom/MMC_par.Vnom_dc;
MMC_par.C_PM = 1.758e-3; % Power module capacitor (F) C_cell
% Energy in kJ/MVA
MMC_par.W_kJ_MVA= 0.5 * MMC_par.C_PM * (MMC_par.Vnom_dc/MMC_par.Nb_PM)^2 * MMC_par.Nb_PM * 6 / (MMC_par.Pnom/1e6)/1e3;
MMC_par.Vc0_PM=0;                     % Capacitors initial voltage (V)

MMC_par.Zb_ac= MMC_par.Vb_ac/MMC_par.Ib_ac;
MMC_par.Zb_dc = MMC_par.Vnom_dc/MMC_par.Ib_dc/2;
%
MMC_par.Carm = MMC_par.C_PM/MMC_par.Nb_PM;
MMC_par.Carm_pu = (MMC_par.Carm*MMC_par.Zb_ac);


% dq and Vdc measurement filter cut-off frequency:
MMC_par.Fn_filter=1000;
MMC_par.Zeta_filter=1;
%
% Active power regulator (Preg)
MMC_par.Kp_Preg= 0.5/3;                % Proportional gain
MMC_par.Ki_Preg= 1.0;                  % Integral gain
MMC_par.Limits_Preg = [ 1.2, 0.8 ] ;   % Output (Vdc_ref) Upper/Lower limits (pu)

%
% Reactive power regulator (Qreg)
MMC_par.Kp_Qreg= 0.5/3;                % Proportional gain
MMC_par.Ki_Qreg= 1.0;                  % Integral gain
MMC_par.Limits_Qreg = [ 0.25, -0.25 ]; % Output (Iq_ref) Upper/Lower limit (pu)
%
% VDC regulator (VDCreg)
MMC_par.Kp_VDCreg=4;                   % Proportional gain
MMC_par.Ki_VDCreg=100;                 % Integral gain
MMC_par.Limits_VDCreg= [ 2.0  -2.0];   % Output Idref [Upper Lower] limits (pu)
%
% Current regulator (Ireg)
MMC_par.Kp_Ireg= 0.6;                  % Proportional gain
MMC_par.Ki_Ireg= 6;                    % Integral gain
MMC_par.Limits_Ireg= [ 2.0  -2.0];     % Output Vdq_conv [Upper Lower] limits (pu)

% CCSC (CCSC)
MMC_par.Kp_CCSC= 0.8;                  % Proportional gain
MMC_par.Ki_CCSC= 50;                    % Integral gain

% CCSC (PLL)
MMC_par.Kp_PLL = 180;                  % Proportional gain
MMC_par.Ki_PLL = 3200;                    % Integral gain


% Feedforward coefficients:

MMC_par.Larm_pu=0.15;
MMC_par.Rarm_pu=MMC_par.Larm_pu/100;

MMC_par.Lff= MMC_par.Larm_pu/2;
MMC_par.Rff= MMC_par.Rarm_pu/2;

MMC_par.Lxfo = 0.12;         % Total Leakage inductance (pu)
MMC_par.Rxfo = 0.003;        % Total winding resistance (pu)

MMC_par.Larm_pu_0 = MMC_par.Larm_pu/(2*pi*MMC_par.Fnom);
MMC_par.Rarm_pu_0 = MMC_par.Rarm_pu;

MMC_par.acdcV = 1/(MMC_par.Vnom_dc/2*sqrt(3)/sqrt(2)/MMC_par.Vnom_sec);
MMC_par.acdcI = MMC_par.Ib_dc/MMC_par.Ib_ac;

MMC_par.R_startup= 400;   % Startup resistance (Ohm)
% Cable data
MMC_par.R_cable = 0.5;      % ohm
MMC_par.L_cable= 15e-3;   % (H)
%
% Grounding reference (series RC)
MMC_par.Rg= 100;              % (Ohms)
MMC_par.Cg= 50e-9;            % (F)


MMC_par.Larm=MMC_par.Larm_pu*(MMC_par.Zb_ac/MMC_par.w);
MMC_par.Rarm=MMC_par.Rarm_pu*MMC_par.Zb_ac;

% Sequencer timing:
MMC_par.Tbrk1_On=0.1;                 % Closing time of breaker 1 (converter energizing)
MMC_par.Tbrk2_On=1.0;                 % Closing time (s) of breaker 2 (across start-up resistor)
%
MMC_par.Tdeblock=1.5;                 % Converter de-block time (s)
MMC_par.Ton_VDCreg=1.5;               % VDC regulator turn-on time (s) - VDC Regulation
MMC_par.Tramping_Vdc_ref=2;           % Start time Vdc_ref ramping to nominal (s)
MMC_par.Slope_Vdc_ref=MMC_par.Vnom_dc/5;      % Sloge Vdc_ref ramping (V/s)
%
MMC_par.Ton_PQreg=4;                  % Preg & Qreg regulators turn-on time (s) - PQ regulation
MMC_par.Tramping_Pref=MMC_par.Ton_PQreg+0.2;  % Start time Pref ramping(s)
MMC_par.Slope_Pref=0.5;               % Sloge Pref ramping (V/s)
MMC_par.Tramping_Qref=MMC_par.Ton_PQreg+3.5;  % Start time Pref ramping(s)
MMC_par.Slope_Qref=0.5;               % Sloge Pref ramping (V/s)
%
MMC_par.Ton_Converter2=4;             % Converter 2 equivalent switched-on time (s)
%%
MMC_par.Tfault= 9999;             % DC Fault timing (s)
MMC_par.Rfault=1;                 % DC Fault resistance (Ohms)

