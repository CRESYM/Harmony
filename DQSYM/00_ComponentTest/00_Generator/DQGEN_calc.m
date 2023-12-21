close all;
clear all;
restoredefaultpath;
addpath(genpath('..\..\#lib'));


N = 1;
Ts = 50e-6;
Ts_dqn = Ts;
%Ts_dqn = 1e-3;

f = 60;
Ws = 2*pi*f;

oDP = cDQN(f);


Sn = 555e6;
Vn = 24e3;

Ib = sqrt(2/3)*Sn/Vn;
Vb = sqrt(2/3)*Vn;

file = 'DQGEN.xlsx';
[SYS, rlc] = fNET2SS(file);

LF = power_loadflow('-v2','DQGEN','solve')

sim('DQGEN')


% SMRD = fCalcMacParam(3.10E-03, ...
%                     [ 555e6,  24e3, 60 ], ...
%                     [1.81, 0.3, 0.23, 1.76, 0.65, 0.25, 0.15], ...
%                       [ 0.8788  0 1 ])

% g = Simulink.Mask.get('DQGEN/Mac_EMT');
% vars_g = g.getWorkspaceVariables;
% SM = vars_g(21).Value;
% 
% SMR = struct();
% % SMR.RLinv = ddqq_dqdq(SM.RLinv);
% % SMR.Linv  = ddqq_dqdq(SM.Linv);
% % SMR.phidq0 = [SM.phiqd0(1:3)' SM.phiqd0(5) SM.phiqd0(4) SM.phiqd0(6) ];
% SMR.RLinv = ddqq_dqdq(SM.RLinv,1);
% SMR.Linv  = ddqq_dqdq(SM.Linv,1);
% SMR.L     = eye(6)/SMR.Linv;
% SMR.phidq0 = [SM.phiqd0(2) SM.phiqd0(1) SM.phiqd0(3) SM.phiqd0(5) SM.phiqd0(4) SM.phiqd0(6) ];
% SMR.R = diag(diag(SMR.RLinv*SMR.L));
% SMR.tho = SM.tho;
% 
% 
% SMR.RLinvW = SMR.RLinv;
% SMR.RLinvW(2,1) = +1;
% SMR.RLinvW(1,2) = -1;