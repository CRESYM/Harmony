clc
clear all;


Rp = 4.3218;
Lp = 0; %0.45856;
Rs = 0.7938; 
Ls = 0.084225; 
Rm = Inf; %1e6;
Lm = Inf; %3000; 
a = 2.0; 

P = 200e6;
V = 20e3;
V2 = 1e4; 
V1 = 2 * V2 ;

Ts = 50e-6;
open ("Three_ph_T_Yparam1.slx");
open ("Three_ph_T_Yparam2.slx");

sim Three_ph_T_Yparam1.slx;
sim Three_ph_T_Yparam2.slx;

%% TFs and SS 

% get the system matrices and build the SS for test
g = Simulink.Mask.get('Three_ph_T_Yparam1/powergui/EquivalentModel1');
vars_g = g.getWorkspaceVariables;
S=vars_g.Value.S;

g1 = Simulink.Mask.get('Three_ph_T_Yparam2/powergui/EquivalentModel1');
vars_g1 = g1.getWorkspaceVariables;
S1=vars_g1.Value.S;
    
% state space from the matrices
% sys=ss(S.A,S.B,S.C,S.D,Ts);
% sys1=ss(S1.A,S1.B,S1.C,S1.D);

% sys2= d2c(sys,'tustin');
% % using ABCD >> TF >>> bode
s = tf('s'); % omega is frequency in rad/s

% TF = C * inv(s*eye(size(A)) - A) * B + D;
G= S.C*(inv(s* eye(size(S.A))-S.A)) * S.B + S.D;

G1= S1.C*(inv(s* eye(size(S1.A))-S1.A)) * S1.B + S1.D;

%% READING Y from Harmony
% Table conversion 
T3 = readmatrix('T3.csv', 'OutputType', 'string');

freq = zeros(length(T3(:,1)),1);
Y11 = zeros(length(T3(:,1)),1);
Y12 = Y11;Y13 = Y11;Y14 = Y11; Y15 = Y11; Y16 = Y11;
Y21 = Y11;Y22 = Y11;Y23 = Y11; Y24 = Y11; Y25 = Y11; Y26 = Y11;
Y31 = Y11;Y32 = Y11;Y33 = Y11; Y34 = Y11; Y35 = Y11; Y36 = Y11;
Y41 = Y11;Y42 = Y11;Y43 = Y11; Y44 = Y11; Y45 = Y11; Y46 = Y11;
Y51 = Y11;Y52 = Y11;Y53 = Y11; Y54 = Y11; Y55 = Y11; Y56 = Y11;
Y61 = Y11;Y62 = Y11;Y63 = Y11; Y64 = Y11; Y65 = Y11; Y66 = Y11;
for i = 1:length(freq)
    freq(i) = str2double(T3(i,1));
    Y11(i) = str2num(T3(i,2));
    Y12(i) = str2num(T3(i,3));
    Y13(i) = str2num(T3(i,4));
    Y14(i) = str2num(T3(i,5));
    Y15(i) = str2num(T3(i,6));
    Y16(i) = str2num(T3(i,7));
    Y21(i) = str2num(T3(i,8));
    Y22(i) = str2num(T3(i,9));
    Y23(i) = str2num(T3(i,10));
    Y24(i) = str2num(T3(i,11));
    Y25(i) = str2num(T3(i,12));
    Y26(i) = str2num(T3(i,13));
    Y31(i) = str2num(T3(i,14));
    Y32(i) = str2num(T3(i,15));
    Y33(i) = str2num(T3(i,16));
    Y34(i) = str2num(T3(i,17));
    Y35(i) = str2num(T3(i,18));
    Y36(i) = str2num(T3(i,19));
    Y41(i) = str2num(T3(i,20));
    Y42(i) = str2num(T3(i,21));
    Y43(i) = str2num(T3(i,22));
    Y44(i) = str2num(T3(i,23));
    Y45(i) = str2num(T3(i,24));
    Y46(i) = str2num(T3(i,25));
    Y51(i) = str2num(T3(i,26));
    Y52(i) = str2num(T3(i,27));
    Y53(i) = str2num(T3(i,28));
    Y54(i) = str2num(T3(i,29));
    Y55(i) = str2num(T3(i,30));
    Y56(i) = str2num(T3(i,31));
    Y61(i) = str2num(T3(i,32));
    Y62(i) = str2num(T3(i,33));
    Y63(i) = str2num(T3(i,34));
    Y64(i) = str2num(T3(i,35));
    Y65(i) = str2num(T3(i,36));
    Y66(i) = str2num(T3(i,37));
end

% Yin_harmony = (Y11 - (Y12.*Y21)./Y22);
%% Initilizaiton for Y calculate

Y11_meas = zeros(size(freq,1),1);
Y12_meas = zeros(size(freq,1),1);
Y13_meas = zeros(size(freq,1),1);
Y14_meas = Y11_meas;
Y15_meas = Y11_meas;
Y16_meas = Y11_meas;
Y21_meas = zeros(size(freq,1),1);
Y22_meas = zeros(size(freq,1),1);
Y23_meas = zeros(size(freq,1),1);
Y24_meas = Y11_meas;
Y25_meas = Y11_meas;
Y26_meas = Y11_meas;
Y31_meas = zeros(size(freq,1),1);
Y32_meas = zeros(size(freq,1),1);
Y33_meas = zeros(size(freq,1),1);
Y34_meas = Y11_meas;
Y35_meas = Y11_meas;
Y36_meas = Y11_meas;
Y41_meas = Y11_meas;
Y42_meas = Y11_meas;
Y43_meas = Y11_meas;
Y44_meas = Y11_meas;
Y45_meas = Y11_meas;
Y46_meas = Y11_meas;
Y51_meas = Y11_meas;
Y52_meas = Y11_meas;
Y53_meas = Y11_meas;
Y54_meas = Y11_meas;
Y55_meas = Y11_meas;
Y56_meas = Y11_meas;
Y61_meas = Y11_meas;
Y62_meas = Y11_meas;
Y63_meas = Y11_meas;
Y64_meas = Y11_meas;
Y65_meas = Y11_meas;
Y66_meas = Y11_meas;


%% Evaluating Y with Freq
for i = 1:length(Y11_meas)
    f = freq(i,1);
    Y11_meas(i) = evalfr(G(1,1), 2*pi*freq(i,1)*1i);
    Y12_meas(i) = evalfr(G(1,2), 2*pi*freq(i,1)*1i);
    Y13_meas(i) = evalfr(G(1,3), 2*pi*freq(i,1)*1i);
    Y21_meas(i) = evalfr(G(2,1), 2*pi*freq(i,1)*1i);
    Y22_meas(i) = evalfr(G(2,2), 2*pi*freq(i,1)*1i);
    Y23_meas(i) = evalfr(G(2,3), 2*pi*freq(i,1)*1i);
    Y31_meas(i) = evalfr(G(3,1), 2*pi*freq(i,1)*1i);
    Y32_meas(i) = evalfr(G(3,2), 2*pi*freq(i,1)*1i);
    Y33_meas(i) = evalfr(G(3,3), 2*pi*freq(i,1)*1i);
    Y41_meas(i) = evalfr(G(4,1), 2*pi*freq(i,1)*1i);
    Y42_meas(i) = evalfr(G(4,2), 2*pi*freq(i,1)*1i);
    Y43_meas(i) = evalfr(G(4,3), 2*pi*freq(i,1)*1i);
    Y51_meas(i) = evalfr(G(5,1), 2*pi*freq(i,1)*1i);
    Y52_meas(i) = evalfr(G(5,2), 2*pi*freq(i,1)*1i);
    Y53_meas(i) = evalfr(G(5,3), 2*pi*freq(i,1)*1i);
    Y61_meas(i) = evalfr(G(6,1), 2*pi*freq(i,1)*1i);
    Y62_meas(i) = evalfr(G(6,2), 2*pi*freq(i,1)*1i);
    Y63_meas(i) = evalfr(G(6,3), 2*pi*freq(i,1)*1i);

    Y14_meas(i) = evalfr(G1(1,1), 2*pi*freq(i,1)*1i);
    Y15_meas(i) = evalfr(G1(1,2), 2*pi*freq(i,1)*1i);
    Y16_meas(i) = evalfr(G1(1,3), 2*pi*freq(i,1)*1i);
    Y24_meas(i) = evalfr(G1(2,1), 2*pi*freq(i,1)*1i);
    Y25_meas(i) = evalfr(G1(2,2), 2*pi*freq(i,1)*1i);
    Y26_meas(i) = evalfr(G1(2,3), 2*pi*freq(i,1)*1i);
    Y34_meas(i) = evalfr(G1(3,1), 2*pi*freq(i,1)*1i);
    Y35_meas(i) = evalfr(G1(3,2), 2*pi*freq(i,1)*1i);
    Y36_meas(i) = evalfr(G1(3,3), 2*pi*freq(i,1)*1i);
    Y44_meas(i) = evalfr(G1(4,1), 2*pi*freq(i,1)*1i);
    Y45_meas(i) = evalfr(G1(4,2), 2*pi*freq(i,1)*1i);
    Y46_meas(i) = evalfr(G1(4,3), 2*pi*freq(i,1)*1i);
    Y54_meas(i) = evalfr(G1(5,1), 2*pi*freq(i,1)*1i);
    Y55_meas(i) = evalfr(G1(5,2), 2*pi*freq(i,1)*1i);
    Y56_meas(i) = evalfr(G1(5,3), 2*pi*freq(i,1)*1i);
    Y64_meas(i) = evalfr(G1(6,1), 2*pi*freq(i,1)*1i);
    Y65_meas(i) = evalfr(G1(6,2), 2*pi*freq(i,1)*1i);
    Y66_meas(i) = evalfr(G1(6,3), 2*pi*freq(i,1)*1i);


end

Y_meas = [Y11_meas, Y12_meas, Y13_meas, Y14_meas, Y15_meas, Y16_meas, ...
    Y21_meas, Y22_meas, Y23_meas, Y24_meas, Y25_meas, Y26_meas, ...
    Y31_meas, Y32_meas, Y33_meas, Y34_meas, Y35_meas, Y36_meas, ...
    Y41_meas, Y42_meas, Y43_meas, Y44_meas, Y45_meas, Y46_meas, ...
    Y51_meas, Y52_meas, Y53_meas, Y54_meas, Y55_meas, Y56_meas, ...
    Y61_meas, Y62_meas, Y63_meas, Y64_meas, Y65_meas, Y66_meas];

save Y_meas;

%%  ploting the Y from Harmony
close all
hold off

figure (1)
% HARMONY
semilogx(freq, 20*log10(abs(Y11)), "r", 'LineWidth', 3)
hold on
semilogx(freq, 20*log10(abs(Y14)), "y", 'LineWidth', 3)
semilogx(freq, 20*log10(abs(Y44)), "m", 'LineWidth', 3)

% Measured Y for comparison

semilogx(freq, 20*log10(abs(Y11_meas)), "b+", 'LineWidth', 1)
semilogx(freq, 20*log10(abs(Y14_meas)), "g+", 'LineWidth', 1)
semilogx(freq, 20*log10(abs(Y44_meas)), "c+", 'LineWidth', 1)

title("Three-phase real transformer",'Interpreter','latex')
legend('Harmony - $20 \log_{10} |Y_{11, 22, 33}/(1 \mbox{ S})|$',...
    'Harmony - $20 \log_{10} |Y_{14, 25, 36, 41, 52, 63}/(1 \mbox{ S})|$',...
    'Harmony - $20 \log_{10} |Y_{44, 55, 66}/(1 \mbox{ S})|$',...
    'Measured - $20 \log_{10} |Y_{11, 22, 33}/(1 \mbox{ S})|$', ...
    'Measured - $20 \log_{10} |Y_{14,25,36,41, 52, 63}/(1 \mbox{ S})|$',...
    'Measured - $20 \log_{10} |Y_{44, 55, 66}/(1 \mbox{ S})|$',...
    'Interpreter','latex')
xlabel("$f \, [\mbox{Hz}]$", 'Interpreter','latex')
ylabel("$20 \log_{10} |Y/(1 \mbox{ S})|$",'Interpreter','latex')
grid on
hold off

figure (2)
semilogx(freq, unwrap(angle(Y11)), "r", 'LineWidth', 3)
hold on
semilogx(freq, angle(Y14), "y", 'LineWidth', 3)
semilogx(freq, angle(Y44), "m", 'LineWidth', 3)

% Measured Y for comparison

semilogx(freq, angle(Y11_meas), "b+", 'LineWidth', 1)
semilogx(freq, angle(Y14_meas), "g+", 'LineWidth', 1)
semilogx(freq, angle(Y44_meas), "c+", 'LineWidth', 1)

title("Three-phase real transformer",'Interpreter','latex')
legend('Harmony - $\mbox{angle}(Y_{11, 22, 33}/(1 \mbox{ S})) \, [\mbox{rad}]$',...
    'Harmony - $\mbox{angle}(Y_{14, 25, 36, 41, 52, 63}/(1 \mbox{ S})) \, [\mbox{rad}]$',...
    'Harmony - $\mbox{angle}(Y_{44, 55, 66}/(1 \mbox{ S})) \, [\mbox{rad}]$',...
    'Measured - $\mbox{angle}(Y_{11, 22, 33}/(1 \mbox{ S})) \, [\mbox{rad}]$', ...
    'Measured - $\mbox{angle}(Y_{14,25,36,41, 52, 63}/(1 \mbox{ S})) \, [\mbox{rad}]$',...
    'Measured - $\mbox{angle}(Y_{44, 55, 66}/(1 \mbox{ S})) \, [\mbox{rad}]$',...
    'Interpreter','latex')
xlabel("$f \, [\mbox{Hz}]$", 'Interpreter','latex')
ylabel("$20 \log_{10} |Y/(1 \mbox{ S})|$",'Interpreter','latex')
grid on
hold off

