clc
clear
close all

%% =========================
%  USER SETTINGS
%  =========================
model1 = "Three_ph_T_DYparam1.slx";
model2 = "Three_ph_T_DYparam2.slx";

eqPath1 = 'Three_ph_T_DYparam1/powergui/EquivalentModel1';
eqPath2 = 'Three_ph_T_DYparam2/powergui/EquivalentModel1';

csvFile = 'T3.csv';

% Start with false first. If the raw transformed Harmony does not match
% Simulink, switch to true and inspect the ratio plot.
useScaledHarmonyForPlots = false;
showDiagnosticRatioFigure = true;

%% =========================
%  PARAMETERS
%  =========================
Rp = 4.3218;
Lp = 0;
Rs = 0.7938;
Ls = 0.084225;
Rm = Inf;
Lm = Inf;
a  = 2.0;

P  = 200e6;
V  = 20e3;
V2 = 1e4;
V1 = 2 * V2;
Ts = 50e-6;

%% =========================
%  OPEN AND SIMULATE DY MODELS
%  =========================
open(model1);
open(model2);

sim(model1);
sim(model2);

%% =========================
%  EXTRACT CONTINUOUS TF MATRICES
%  =========================
g  = Simulink.Mask.get(eqPath1);
wg = g.getWorkspaceVariables;
S  = wg.Value.S;

g1  = Simulink.Mask.get(eqPath2);
wg1 = g1.getWorkspaceVariables;
S1  = wg1.Value.S;

s = tf('s');

G  = S.C  * inv(s * eye(size(S.A))  - S.A)  * S.B  + S.D;
G1 = S1.C * inv(s * eye(size(S1.A)) - S1.A) * S1.B + S1.D;

%% =========================
%  READ YY HARMONY CSV
%  =========================
T3 = readmatrix(csvFile, 'OutputType', 'string');

nFreq = size(T3,1);
freq  = zeros(nFreq,1);
Yyy_harmony = zeros(6,6,nFreq);

for k = 1:nFreq
    freq(k) = str2double(T3(k,1));
    col = 2;
    for r = 1:6
        for c = 1:6
            Yyy_harmony(r,c,k) = parseComplexEntry(T3(k,col));
            col = col + 1;
        end
    end
end

%% =========================
%  TRANSFORM YY -> DY
%  Y_DY = blkdiag(M,I3) * Y_YY * blkdiag(Tv,I3)
%  =========================
I3 = eye(3);

Tv = (1/sqrt(3)) * [ 1  -1   0;
       0   1  -1;
      -1   0   1 ];

M  = (1/sqrt(3)) * [ 1   0  -1;
                    -1   1   0;
                     0  -1   1 ];

Lmat = blkdiag(M, I3);
Rmat = blkdiag(Tv, I3);

Y_harmony = zeros(6,6,nFreq);
for k = 1:nFreq
    Y_harmony(:,:,k) = Lmat * Yyy_harmony(:,:,k) * Rmat;
end

%% =========================
%  OPTIONAL SCALING FOR DY
%  From raw ratio plot:
%  |Y11_m/Y11_h| ≈ 1/sqrt(3)
%  while |Y14_m/Y14_h| and |Y44_m/Y44_h| ≈ 1.
%
%  Therefore only the primary-primary delta-side block is scaled:
%      Ypp -> (1/sqrt(3)) Ypp
%  =========================
Y_cmp = Y_harmony;

if useScaledHarmonyForPlots
    Y_cmp(1:3,1:3,:) = (1/sqrt(3)) * Y_cmp(1:3,1:3,:);
    Y_ref = Y_cmp;
    refLabel = 'Harmony transformed/scaled';
else
    Y_ref = Y_harmony;
    refLabel = 'Analytical';
end

%% =========================
%  EVALUATE MEASURED / SIMULINK DY MATRIX
%  =========================
Y_meas = zeros(6,6,nFreq);

for k = 1:nFreq
    jw = 2*pi*freq(k)*1i;

    % Columns 1:3 from first model
    for r = 1:6
        for c = 1:3
            Y_meas(r,c,k) = evalfr(G(r,c), jw);
        end
    end

    % Columns 4:6 from second model
    for r = 1:6
        for c = 1:3
            Y_meas(r,c+3,k) = evalfr(G1(r,c), jw);
        end
    end
end

save('Y_meas_DY_from_YY.mat', 'Y_meas', 'Y_harmony', 'Y_cmp', 'Y_ref', 'Yyy_harmony', 'freq');

%% =========================
%  EXTRACT REPRESENTATIVE ENTRIES
%  DY structure:
%    primary delta side:
%      Y11 = Y22 = Y33,  Y11 = -2Y12, Y12 = Y13
%    secondary Y side:
%      Y44 = Y55 = Y66
%    cross-block examples:
%      Y14 = -Y16, Y15 = 0
%      Y41 = -Y42, Y43 = 0
%  =========================
Y11_h = squeeze(Y_ref(1,1,:));
Y12_h = squeeze(Y_ref(1,2,:));
Y14_h = squeeze(Y_ref(1,4,:));
Y15_h = squeeze(Y_ref(1,5,:));
Y16_h = squeeze(Y_ref(1,6,:));
Y41_h = squeeze(Y_ref(4,1,:));
Y42_h = squeeze(Y_ref(4,2,:));
Y43_h = squeeze(Y_ref(4,3,:));
Y44_h = squeeze(Y_ref(4,4,:));

Y11_m = squeeze(Y_meas(1,1,:));
Y12_m = squeeze(Y_meas(1,2,:));
Y14_m = squeeze(Y_meas(1,4,:));
Y15_m = squeeze(Y_meas(1,5,:));
Y16_m = squeeze(Y_meas(1,6,:));
Y41_m = squeeze(Y_meas(4,1,:));
Y42_m = squeeze(Y_meas(4,2,:));
Y43_m = squeeze(Y_meas(4,3,:));
Y44_m = squeeze(Y_meas(4,4,:));

%% =========================
%  CHECKS
%  =========================
fprintf('\n================ DY CHECKS ================\n');

fprintf('\n--- Harmony DY structure ---\n');
fprintf('max|Y11-Y22|   = %.6g\n', max(abs(squeeze(Y_ref(1,1,:)) - squeeze(Y_ref(2,2,:)))));
fprintf('max|Y22-Y33|   = %.6g\n', max(abs(squeeze(Y_ref(2,2,:)) - squeeze(Y_ref(3,3,:)))));
fprintf('max|Y11+2Y12|  = %.6g\n', max(abs(squeeze(Y_ref(1,1,:)) + 2*squeeze(Y_ref(1,2,:)))));
fprintf('max|Y12-Y13|   = %.6g\n', max(abs(squeeze(Y_ref(1,2,:)) - squeeze(Y_ref(1,3,:)))));
fprintf('max|Y44-Y55|   = %.6g\n', max(abs(squeeze(Y_ref(4,4,:)) - squeeze(Y_ref(5,5,:)))));
fprintf('max|Y55-Y66|   = %.6g\n', max(abs(squeeze(Y_ref(5,5,:)) - squeeze(Y_ref(6,6,:)))));
fprintf('max|Y14+Y16|   = %.6g\n', max(abs(Y14_h + Y16_h)));
fprintf('max|Y15|       = %.6g\n', max(abs(Y15_h)));
fprintf('max|Y41+Y42|   = %.6g\n', max(abs(Y41_h + Y42_h)));
fprintf('max|Y43|       = %.6g\n', max(abs(Y43_h)));

fprintf('\n--- Measured DY structure ---\n');
fprintf('max|Y11-Y22|   = %.6g\n', max(abs(squeeze(Y_meas(1,1,:)) - squeeze(Y_meas(2,2,:)))));
fprintf('max|Y22-Y33|   = %.6g\n', max(abs(squeeze(Y_meas(2,2,:)) - squeeze(Y_meas(3,3,:)))));
fprintf('max|Y11+2Y12|  = %.6g\n', max(abs(squeeze(Y_meas(1,1,:)) + 2*squeeze(Y_meas(1,2,:)))));
fprintf('max|Y12-Y13|   = %.6g\n', max(abs(squeeze(Y_meas(1,2,:)) - squeeze(Y_meas(1,3,:)))));
fprintf('max|Y44-Y55|   = %.6g\n', max(abs(squeeze(Y_meas(4,4,:)) - squeeze(Y_meas(5,5,:)))));
fprintf('max|Y55-Y66|   = %.6g\n', max(abs(squeeze(Y_meas(5,5,:)) - squeeze(Y_meas(6,6,:)))));
fprintf('max|Y14+Y16|   = %.6g\n', max(abs(Y14_m + Y16_m)));
fprintf('max|Y15|       = %.6g\n', max(abs(Y15_m)));
fprintf('max|Y41+Y42|   = %.6g\n', max(abs(Y41_m + Y42_m)));
fprintf('max|Y43|       = %.6g\n', max(abs(Y43_m)));

fprintf('\n--- Reference vs measured ---\n');
fprintf('max|Y11_h - Y11_m| = %.6g\n', max(abs(Y11_h - Y11_m)));
fprintf('max|Y14_h - Y14_m| = %.6g\n', max(abs(Y14_h - Y14_m)));
fprintf('max|Y44_h - Y44_m| = %.6g\n', max(abs(Y44_h - Y44_m)));

R11 = Y11_m ./ Y11_h;
R14 = Y14_m ./ Y14_h;
R44 = Y44_m ./ Y44_h;

fprintf('\n--- Ratio summary ---\n');
fprintf('max deviation |R11-1| = %.6g\n', max(abs(abs(R11)-1)));
fprintf('max deviation |R14-1| = %.6g\n', max(abs(abs(R14)-1)));
fprintf('max deviation |R44-1| = %.6g\n', max(abs(abs(R44)-1)));
fprintf('===========================================\n\n');

%% =========================
%  PLOTS
%  =========================
figure(1)
semilogx(freq,20*log10(abs(Y11_h)),'r','LineWidth',3); hold on
semilogx(freq,20*log10(abs(Y14_h)),'y','LineWidth',3);
semilogx(freq,20*log10(abs(Y44_h)),'m','LineWidth',3);
semilogx(freq,20*log10(abs(Y11_m)),'b+','LineWidth',1);
semilogx(freq,20*log10(abs(Y14_m)),'g+','LineWidth',1);
semilogx(freq,20*log10(abs(Y44_m)),'c+','LineWidth',1);
title('Three-phase $\Delta$-Y transformer','Interpreter','latex')
legend([refLabel ' - $20\log_{10}|Y_{11}|$'], ...
       [refLabel ' - $20\log_{10}|Y_{14}|$'], ...
       [refLabel ' - $20\log_{10}|Y_{44}|$'], ...
       'Measured - $20\log_{10}|Y_{11}|$', ...
       'Measured - $20\log_{10}|Y_{14}|$', ...
       'Measured - $20\log_{10}|Y_{44}|$', ...
       'Interpreter','latex','Location','best')
xlabel('$f\,[\mathrm{Hz}]$','Interpreter','latex')
ylabel('$20\log_{10}|Y/(1\mathrm{S})|$','Interpreter','latex')
grid on
hold off

figure(2)
semilogx(freq,unwrap(angle(Y11_h)),'r','LineWidth',3); hold on
semilogx(freq,unwrap(angle(Y14_h)),'y','LineWidth',3);
semilogx(freq,unwrap(angle(Y44_h)),'m','LineWidth',3);
semilogx(freq,unwrap(angle(Y11_m)),'b+','LineWidth',1);
semilogx(freq,unwrap(angle(Y14_m)),'g+','LineWidth',1);
semilogx(freq,unwrap(angle(Y44_m)),'c+','LineWidth',1);
title('Three-phase $\Delta$-Y transformer','Interpreter','latex')
legend([refLabel ' - $\angle Y_{11}$'], ...
       [refLabel ' - $\angle Y_{14}$'], ...
       [refLabel ' - $\angle Y_{44}$'], ...
       'Measured - $\angle Y_{11}$', ...
       'Measured - $\angle Y_{14}$', ...
       'Measured - $\angle Y_{44}$', ...
       'Interpreter','latex','Location','best')
xlabel('$f\,[\mathrm{Hz}]$','Interpreter','latex')
ylabel('$\angle Y\,[\mathrm{rad}]$','Interpreter','latex')
grid on
hold off

if showDiagnosticRatioFigure
    figure(3)
    semilogx(freq,abs(R11),'r','LineWidth',2); hold on
    semilogx(freq,abs(R14),'b--','LineWidth',2);
    semilogx(freq,abs(R44),'k:','LineWidth',2);
    grid on
    legend('$|Y_{11,m}/Y_{11,h}|$','$|Y_{14,m}/Y_{14,h}|$','$|Y_{44,m}/Y_{44,h}|$', ...
        'Interpreter','latex','Location','best')
    xlabel('$f\,[\mathrm{Hz}]$','Interpreter','latex')
    ylabel('ratio magnitude','Interpreter','latex')
    title('Measured / Harmony-based DY ratios','Interpreter','latex')
    hold off
end

%% =========================
%  LOCAL FUNCTION
%  =========================
function z = parseComplexEntry(strVal)
    s = char(strVal);
    s = strtrim(s);
    s = strrep(s, ' ', '');

    z = str2num(s); %#ok<ST2NM>
    if isempty(z)
        s = strrep(s, 'i*', 'i');
        s = strrep(s, '*i', 'i');
        z = str2num(s); %#ok<ST2NM>
    end
    if isempty(z)
        error('Could not parse complex CSV entry: %s', char(strVal));
    end
end