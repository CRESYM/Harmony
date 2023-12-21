close all;
clear all;
restoredefaultpath;
addpath(genpath('..\..\#lib'));



N = 1;
Ts = 50e-6;
Ts_dqn = Ts;

f = 60;
Ws = 2*pi*f;

oDP = cDQN(f);


Sn = 100e6;
Vn = 230e3;

Ib = sqrt(2/3)*Sn/Vn;
Vb = sqrt(2/3)*Vn;


Zb = Vb/Ib


file = 'IEEE_9bus.xlsx';
[SYS, rlc] = fNET2SS(file);

run("IEEE_9bus")

% LF = power_loadflow('-v2','IEEE_9bus','solve')
% 
% 
% 
% 
% % 
% % 
% g = Simulink.Mask.get('IEEE_9bus/M1');
% vars_g = g.getWorkspaceVariables;
% SM1 = vars_g(21).Value;
% % 
% g = Simulink.Mask.get('IEEE_9bus/M2');
% vars_g = g.getWorkspaceVariables;
% SM2 = vars_g(21).Value;
% 
% g = Simulink.Mask.get('IEEE_9bus/M3');
% vars_g = g.getWorkspaceVariables;
% SM3 = vars_g(21).Value;
% % 
% [real(100*LF.sm(2).S/Sn), imag(100*LF.sm(2).S/Sn), abs(LF.bus(1).Vbus), LF.sm(2).Vf, deg2rad(LF.sm(2).th0)]
% 
% 
% 
% 
% SMRD2 = fCalcMacParam(2.8544e-3, ...
%                     [200E6  18000   60 ], ...
%                     [ 1.305, 0.296, 0.252, 0.474, 0.243, 0.18 ], ...
%                     [ 1.01, 0.053, 0.0513 ], ...
%                     [real(100*LF.sm(2).S/Sn), imag(100*LF.sm(2).S/Sn), abs(LF.bus(1).Vbus), LF.sm(2).Vf, deg2rad(6.37)], ...
%                     [ 3.7 0  32])
% % 
% SMR.RLinv = ddqq_dqdq(SM2.RLinv,1);
% SMR.Linv  = ddqq_dqdq(SM2.Linv,1);
% SMR.R = ddqq_dqdq(SM2.R,1);
% 
% 
% SMR.phiqd0_d = SM2.phiqd0_d([2,1,3,5,4],:);








