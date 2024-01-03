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
sim("IEEE_9bus")


A = -R/L;
B = eye(3)/L;
C = eye(3);
D = zeros(3,3);

Ac = Sas*A/Sas;
Bc = Sas*B/Sas;
Cc = Sas*C/Sas;



    I = eye(size(Ac,1));
    INVR = I/(I-Ac*Ts/2);
    Ad1 = INVR*(I+Ac*Ts/2);
    Bd1 = INVR*Bc;
    Cd1 = Cc*INVR*Ts;
    Dd1 = Cd1*Bc/2;


% LF = power_loadflow('-v2','IEEE_9bus','solve')
% % 
% % 
% % 
% % 
% % % 
% % % 
% g = Simulink.Mask.get('IEEE_9bus/M1');
% vars_g = g.getWorkspaceVariables;
% SM1 = vars_g(21).Value;
% % 
% g = Simulink.Mask.get('IEEE_9bus/M2');
% vars_g = g.getWorkspaceVariables;
% SM2 = vars_g(21).Value;
% % 
% g = Simulink.Mask.get('IEEE_9bus/M3');
% vars_g = g.getWorkspaceVariables;
% SM3 = vars_g(21).Value;
% % % 
% % [real(100*LF.sm(2).S/Sn), imag(100*LF.sm(2).S/Sn), abs(LF.bus(1).Vbus), LF.sm(2).Vf, deg2rad(LF.sm(2).th0)]
% % 
% % 
% % 
% % 
% 
% SMRD2 = fCalcMacParam(2.8544e-3, ...
%                     [200E6  18e3   60 ], ...
%                     [ 1.305, 0.296, 0.252, 0.474, 0.243, 0.18 ], ...
%                     [ 1.01, 0.053, 0.0513 ], ...
%                     [0.8250    0.2219    1.0000    1.5762   deg2rad(6.37)], ...
%                     [ 3.7 0  32], Ts)
% 
% 
% 
% SMRD3 = fCalcMacParam(2.8544e-3, ...
%                     [200E6  13.8e3   60 ], ...
%                     [ 1.305, 0.296, 0.252, 0.474, 0.243, 0.18 ], ...
%                     [ 1.01, 0.053, 0.0513 ], ...
%                     [0.4250    0.1355    1.0000    1.2604  deg2rad(-0.3643)], ...
%                     [ 3.7 0  32], Ts)
% 
% [SM2.phiqd0'; SMRD2.phidq0']
% [SM2.iqd0'; SMRD2.idq0']
% 
% [SM3.phiqd0'; SMRD3.phidq0']
%[SM3.iqd0'; SMRD3.idq0']

% % % 
% SMR.RLinv = ddqq_dqdq(SM2.RLinv,1);
% SMR.Linv  = ddqq_dqdq(SM2.Linv,1);
% SMR.R = ddqq_dqdq(SM2.R,1);
% 
% 
% SMR.phiqd0_d = SM2.phiqd0_d([2,1,3,5,4],:);








