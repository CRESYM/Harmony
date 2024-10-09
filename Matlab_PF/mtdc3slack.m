function  mpc = mtdc3slack
%% system MVA base
mpc.baseMVA = 100;

%% dc grid topology
mpc.pol=2;  % numbers of poles (1=monopolar grid, 2=bipolar grid)

%% bus data
%	bus_i	type	Pd	Qd	Gs	Bs	area	Vm      Va	baseKV	zone	Vmax	Vmin
mpc.bus = [
	1       3       0	0	0   0   1    1	0	345     1       1.1     0.9;
	2       1       0	0	0   0   1    1  0	345     1       1.1     0.9;
	3       1       0	0	0   0   1    1  0	345     1       1.1     0.9;
];

%% converters
% %   busdc_i busac_i gridac    type_dc type_ac P_g   Q_g   Vtar    rtf     xtf     bf     rc     xc     basekVac    Vmmax   Vmmin   Imax    status   LossA LossB  LossCrec LossCinv  droop      Pdcset    Vdcset  dVdcset
mpc.conv = [ 
    1  2  1       1       1       -60    -40    1     0.0015  0.1121  0.0887 0.0001   0.16428  345         1.1     0.9     1.2     1       1.103 0.887  2.885   4.371  0  0  0  0;
    2  3  1       2       2         0      0    1     0.0015  0.1121  0.0887 0.0001   0.16428  345         1.1     0.9     1.2     1       1.103 0.887  2.885   4.371  0  0  0  0;
    3  5  1       1       1        35      5    1     0.0015  0.1121  0.0887 0.0001   0.16428  345         1.1     0.9     1.2     1       1.103 0.887  2.885   4.371  0  0  0  0;
];


%% branch data
%	fbus	tbus	r	x	b	rateA	rateB	rateC	ratio	angle	status angmin	angmax
mpc.branch = [
    1       2       0.052   0   0    100     100     100      0       0       1   -0   0;
    2       3       0.052   0   0    100     100     100      0       0       1   -0   0;
    1       3       0.073   0   0    100     100     100     0       0       1   -0   0;
];