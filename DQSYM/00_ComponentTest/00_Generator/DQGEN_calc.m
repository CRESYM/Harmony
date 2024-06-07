close all;
clear all;
restoredefaultpath;
addpath(genpath('..\..\#lib'));


N = 3;
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

%LF = power_loadflow('-v2','DQGEN','solve')

run('DQGEN')
sim('DQGEN')



% [Adc,Bdc,Cdc,Ddc1,Ddc2,InitVec_c] = oDP.fDiscSSTest(SYS.A,SYS.B,SYS.C,SYS.D,SYS.x0,Ts,N)
% 
% 
% I = eye(size(SYS.A,1));
% INVR = I/(I-SYS.A*Ts/2);
% Cd = INVR*SYS.C*Ts
% SYS.x01 = InitVec'



% SM2 = fCalcMacParam2(3.10E-03, ...
%                     [ 555e6,  24e3, 60 ], ...
%                     [1.81, 0.3, 0.23, 1.76, 0.65, 0.25, 0.15], ...
%                     [ 1.3201, 0.0231, 0.3371, 0.0295 ], ...
%                     [0.5396   -0.7334    1.0000    1.0315    0.4264], ...
%                     [ 0.8788  0 1 ], ...
%                     Ts)
% 
% g = Simulink.Mask.get('DQGEN2/Mac_EMT');
% vars_g = g.getWorkspaceVariables;
% SM = vars_g(21).Value;
% SM.L = eye(SM.nState)/SM.Linv;
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

% [SM.phiqd0'; SM2.phidq0']
% [SM.iqd0'; SM2.idq0']
% 
% [real(LF.bus(1).Sbus)/5.55, imag(LF.bus(1).Sbus)/5.55, abs(LF.bus(1).Vbus), LF.sm.Vf, deg2rad(43.35)]