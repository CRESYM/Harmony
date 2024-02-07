clear all;
close all;
restoredefaultpath;
addpath(genpath('..\..\#lib'));


N = 7;
f = 50;

N_vec = 10:2:200;

RES = zeros(6,numel(N_vec));

%Ts_dqn = 20*Ts;
%Ts = 1/(100*N*f);
Ts = 50e-6;

Ts_dqn = 1.0000e-03

Ts_dqn = Ts;
Ts_emt = Ts;

Ws = 2*pi*f;

oDP = cDQN(f);

[Xdcpnz, Ydcpnz, Zdcpnz, Xdcpnz_c, Ydcpnz_c, Zdcpnz_c] = fGenerateInputSignals(N,0);


Ta_dqn = oDP.Ta_dqn;

%%%%%%%%%%%%%%%%%%%%%%%% DP LINE %%%%%%%%%%%%%%%%%%%%%%%%
sym = 1; mut = 1;
if(sym)
    Ra = 1.3;         Rb = Ra; Rc = Ra;
    La = 1e-3;      Lb = La; Lc = La;
else
    Ra = 1.64;      Rb = 1.4*Ra; Rc = 2.8*Ra;
    La = 1e-3;      Lb = 2*La; Lc = 0.5*La;
end

if(mut)
    Rab = Ra*0.85;   Rac = Rab; Rbc = Rab;
    Lab = La*0.92;   Lac = Lab; Lbc = Lab;
else
    Rab = 0;        Rac = Rab; Rbc = Rab;
    Lab = 0;        Lac = Lab; Lbc = Lab;
end

    %Ra   Rb   Rc
Rd =[Ra   0    0 ; %Ra
     0    Rb   0;  %Rb
     0    0    Rc ]; %Rc
    %Ra   Rb   Rc
Rm =[0    Rab  Rac ; %Ra
     Rab  0    Rbc;  %Rb
     Rac  Rbc  0 ]; %Rc

    %Ra   Rb   Rc
Ld =[La   0    0 ; %Ra
     0    Lb   0;  %Rb
     0    0    Lc ]; %Rc
    %Ra   Rb   Rc
Lm =[0    Lab  Lac ; %Ra
     Lab  0    Lbc;  %Rb
     Lac  Lbc  0 ]; %Rc

R = Rd + Rm;
L = Ld + Lm;

% HrmDbl = 1;
% HrmOrd = 2*N;
% % %%%%%%%%%%%%%%%%%%%%%%%% END DP LINE %%%%%%%%%%%%%%%%%%%%%%%%
modelName = 'DQN_ABC_Test';
warning('off','all');
open_system(modelName)
%set_param('DQN_ABC_Test', 'DecoupledContinuousIntegration', 'on')
set_param(modelName,'Profile','on');
%out = sim(modelName);

selMat = zeros(1*N+1,6);
selMat(1,:) = 1;
selMat(2,:) = abs(cos(0:pi/2:5*pi/2));
selMat(3,:) = abs(sin(0:pi/2:5*pi/2));

HRM = N;

Carm = 3.76;

Sas = oDP.Sas;
Ssa = oDP.Ssa;


X_DQN = f012C2DQN(Xdcpnz_c);

Rpnz  = diag(diag(real(blkdiag(Ssa,Ssa)*blkdiag(R,R)/blkdiag(Ssa,Ssa))))
iLpnz = diag(diag(real([Sas/(L)/Sas zeros(3,3); zeros(3,3) Sas/(L)/Sas])));

oDP = cDQN(f);

sim('DQN_ABC_Test')





% out.XY_DQN-fMultDQNc(Xdcpnz_c, Ydcpnz_c, N, 1)
% fDQN012c(Z_DQN)-Zdcpnz_c

%fMultDQNc(Xdcpnz_c, Ydcpnz_c, N, 1)RRR
%fDQN012c(Zdc)-Zdcpnz_c(:,1)  fDQN012c(Z_DQN())-Zdcpnz_c
% Ydcpnz_c(:,1)
% Xdcpnz_c(1,2)
% 
% abs(Xdcpnz_c(1,i+1))
% 
% -imag(0.5*Xdcpnz_c(1,i+1)'*Ydcpnz_c(2,j+1))-imag(0.5*Xdcpnz_c(3,i+1)'*Ydcpnz_c(1,j+1))-imag(0.5*Xdcpnz_c(2,i+1)'*Ydcpnz_c(3,j+1))
% 
% + imag(0.5*Xdcpnz_c(2,j+1)'*Ydcpnz_c(1,i+1)) + imag(0.5*Xdcpnz_c(1,j+1)'*Ydcpnz_c(3,i+1)) + imag(0.5*Xdcpnz_c(3,j+1)'*Ydcpnz_c(2,i+1))
% 
% [0 1 0; 1 0 0; 0 0 1]*Ydcpnz_c(:,j+1)