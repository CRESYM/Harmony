clc
clear
close all

%% =========================
%  USER SETTINGS
%  =========================
model1 = "Three_ph_T_YDparam1.slx";
model2 = "Three_ph_T_YDparam2.slx";

eqPath1 = 'Three_ph_T_YDparam1/powergui/EquivalentModel1';
eqPath2 = 'Three_ph_T_YDparam2/powergui/EquivalentModel1';

csvFile = 'T3.csv';

% true  -> best visual agreement with native Simulink YD
% false -> strict Harmony YY -> YD transformed reference
useScaledHarmonyForPlots = true;

% show optional diagnostic ratio figure
showDiagnosticRatioFigure = true;

%% =========================
%  PARAMETERS
%  =========================
Rp = 4.3218;
Lp = 0;
Rs = 0.7938;
Ls = 0.084225;

% IMPORTANT:
% these must match the values used when the Harmony YY CSV was generated
Rm = Inf;
Lm = Inf;

a  = 2.0;

P  = 200e6;
V  = 20e3;
V2 = 1e4;
V1 = 2 * V2;

Ts = 50e-6;

%% =========================
%  OPEN AND SIMULATE YD MODELS
%  =========================
open(model1);
open(model2);

sim(model1);
sim(model2);

%% =========================
%  EXTRACT CONTINUOUS TRANSFER MATRICES
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

% YY matrix from Harmony, one 6x6 matrix per frequency
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
%  TRANSFORM HARMONY YY -> YD
%  =========================
I3 = eye(3);

Tv = (1/sqrt(3)) * [ 1  -1   0;
       0   1  -1;
      -1   0   1 ];

M  = (1/sqrt(3)) * [ 1   0  -1; -1   1   0; 0  -1   1 ];

Lmat = blkdiag(I3, M);
Rmat = blkdiag(I3, Tv);

Y_harmony = zeros(6,6,nFreq);

for k = 1:nFreq
    Y_harmony(:,:,k) = Lmat * Yyy_harmony(:,:,k) * Rmat;
end

%% =========================
%  OPTIONAL SCALING TO MATCH SIMULINK YD CONVENTION
%  =========================
Y_cmp = Y_harmony;

Y_cmp(1:3,1:3,:) = Y_cmp(1:3,1:3,:);
Y_cmp(1:3,4:6,:) = Y_cmp(1:3,4:6,:);
Y_cmp(4:6,1:3,:) = Y_cmp(4:6,1:3,:);
Y_cmp(4:6,4:6,:) = Y_cmp(4:6,4:6,:);



%% =========================
%  EVALUATE MEASURED / SIMULINK YD MATRIX
%  =========================
Y_meas = zeros(6,6,nFreq);

for k = 1:nFreq
    jw = 2*pi*freq(k)*1i;

    % Columns 1:3 from G
    for r = 1:6
        for c = 1:3
            Y_meas(r,c,k) = evalfr(G(r,c), jw);
        end
    end

    % Columns 4:6 from G1
    for r = 1:6
        for c = 1:3
            Y_meas(r,c+3,k) = evalfr(G1(r,c), jw);
        end
    end
end

if useScaledHarmonyForPlots
    Y_ref = Y_cmp;
    Y_meas = 1/3 * Y_meas;
    refLabel = 'Analytical scaled';
else
    Y_ref = Y_harmony;
    refLabel = 'Analytical';
end

save('Y_meas_YD_final.mat', 'Y_meas', 'Y_harmony', 'Y_cmp', 'Y_ref', 'Yyy_harmony', 'freq');

%% =========================
%  EXTRACT REPRESENTATIVE YD ENTRIES
%  =========================
% Reference (Harmony-based YD)
Y11_h = squeeze(Y_ref(1,1,:));
Y14_h = squeeze(Y_ref(1,4,:));
Y44_h = squeeze(Y_ref(4,4,:));
Y45_h = squeeze(Y_ref(4,5,:));

% Measured (native Simulink YD)
Y11_m = squeeze(Y_meas(1,1,:));
Y14_m = squeeze(Y_meas(1,4,:));
Y44_m = squeeze(Y_meas(4,4,:));
Y45_m = squeeze(Y_meas(4,5,:));

% Raw transformed Harmony checks
Y14_raw = squeeze(Y_harmony(1,4,:));
Y15_raw = squeeze(Y_harmony(1,5,:));
Y16_raw = squeeze(Y_harmony(1,6,:));
Y44_raw = squeeze(Y_harmony(4,4,:));
Y45_raw = squeeze(Y_harmony(4,5,:));

%% =========================
%  CHECKS
%  =========================
fprintf('\n================ FINAL YD CHECKS ================\n');

fprintf('\n--- Raw Harmony YD structure ---\n');
fprintf('max|Y14+Y15|   = %.6g\n', max(abs(Y14_raw + Y15_raw)));
fprintf('max|Y16|       = %.6g\n', max(abs(Y16_raw)));
fprintf('max|Y44+2Y45|  = %.6g\n', max(abs(Y44_raw + 2*Y45_raw)));

fprintf('\n--- Measured YD structure ---\n');
fprintf('max|Y14+Y15|   = %.6g\n', max(abs(squeeze(Y_meas(1,4,:)) + squeeze(Y_meas(1,5,:)))));
fprintf('max|Y16|       = %.6g\n', max(abs(squeeze(Y_meas(1,6,:)))));
fprintf('max|Y44+2Y45|  = %.6g\n', max(abs(squeeze(Y_meas(4,4,:)) + 2*squeeze(Y_meas(4,5,:)))));

fprintf('\n--- Reference vs measured ---\n');
fprintf('max|Y11_h - Y11_m|   = %.6g\n', max(abs(Y11_h - Y11_m)));
fprintf('max|Y14_h - Y14_m|   = %.6g\n', max(abs(Y14_h - Y14_m)));
fprintf('max|Y44_h - Y44_m|   = %.6g\n', max(abs(Y44_h - Y44_m)));
fprintf('max|2Y45_h-2Y45_m|   = %.6g\n', max(abs(2*Y45_h - 2*Y45_m)));

R11 = Y11_m ./ Y11_h;
R14 = Y14_m ./ Y14_h;
R44 = Y44_m ./ Y44_h;

fprintf('\n--- Ratio summary ---\n');
fprintf('max deviation |R11-1| = %.6g\n', max(abs(abs(R11)-1)));
fprintf('max deviation |R14-1| = %.6g\n', max(abs(abs(R14)-1)));
fprintf('max deviation |R44-1| = %.6g\n', max(abs(abs(R44)-1)));

fprintf('=================================================\n\n');

%% =========================
%  FINAL FIGURE 1: MAGNITUDE
%  =========================
figure(1)

semilogx(freq, 20*log10(abs(Y11_h)), 'r', 'LineWidth', 3); hold on
semilogx(freq, 20*log10(abs(Y14_h)), 'y', 'LineWidth', 3);
semilogx(freq, 20*log10(abs(Y44_h)), 'm', 'LineWidth', 3);

semilogx(freq, 20*log10(abs(Y11_m)), 'b+', 'LineWidth', 1);
semilogx(freq, 20*log10(abs(Y14_m)), 'g+', 'LineWidth', 1);
semilogx(freq, 20*log10(abs(Y44_m)), 'c+', 'LineWidth', 1);

title('Three-phase Y-$\Delta$ transformer','Interpreter','latex')
legend([refLabel ' - $20\log_{10}|Y_{11}/(1\mathrm{S})|$'], ...
       [refLabel ' - $20\log_{10}|Y_{14}/(1\mathrm{S})|$'], ...
       [refLabel ' - $20\log_{10}|Y_{44}/(1\mathrm{S})|$'], ...
       'Measured - $20\log_{10}|Y_{11}/(1\mathrm{S})|$', ...
       'Measured - $20\log_{10}|Y_{14}/(1\mathrm{S})|$', ...
       'Measured - $20\log_{10}|Y_{44}/(1\mathrm{S})|$', ...
       'Interpreter','latex', 'Location','best')
xlabel('$f\,[\mathrm{Hz}]$','Interpreter','latex')
ylabel('$20\log_{10}|Y/(1\mathrm{S})|$','Interpreter','latex')
grid on
hold off

%% =========================
%  FINAL FIGURE 2: PHASE
%  =========================
figure(2)

semilogx(freq, unwrap(angle(Y11_h)), 'r', 'LineWidth', 3); hold on
semilogx(freq, unwrap(angle(Y14_h)), 'y', 'LineWidth', 3);
semilogx(freq, unwrap(angle(Y44_h)), 'm', 'LineWidth', 3);

semilogx(freq, unwrap(angle(Y11_m)), 'b+', 'LineWidth', 1);
semilogx(freq, unwrap(angle(Y14_m)), 'g+', 'LineWidth', 1);
semilogx(freq, unwrap(angle(Y44_m)), 'c+', 'LineWidth', 1);

title('Three-phase Y-$\Delta$ transformer','Interpreter','latex')
legend([refLabel ' - $\angle Y_{11}\,[\mathrm{rad}]$'], ...
       [refLabel ' - $\angle Y_{14}\,[\mathrm{rad}]$'], ...
       [refLabel ' - $\angle Y_{44}\,[\mathrm{rad}]$'], ...
       'Measured - $\angle Y_{11}\,[\mathrm{rad}]$', ...
       'Measured - $\angle Y_{14}\,[\mathrm{rad}]$', ...
       'Measured - $\angle Y_{44}\,[\mathrm{rad}]$', ...
       'Interpreter','latex', 'Location','best')
xlabel('$f\,[\mathrm{Hz}]$','Interpreter','latex')
ylabel('$\angle Y\,[\mathrm{rad}]$','Interpreter','latex')
grid on
hold off

%% =========================
%  OPTIONAL DIAGNOSTIC FIGURE
%  =========================
if showDiagnosticRatioFigure
    figure(3)

    semilogx(freq, abs(R11), 'r', 'LineWidth', 2); hold on
    semilogx(freq, abs(R14), 'b--', 'LineWidth', 2);
    semilogx(freq, abs(R44), 'k:', 'LineWidth', 2);
    grid on
    legend('$|Y_{11,m}/Y_{11,h}|$', '$|Y_{14,m}/Y_{14,h}|$', '$|Y_{44,m}/Y_{44,h}|$', ...
           'Interpreter','latex', 'Location','best')
    xlabel('$f\,[\mathrm{Hz}]$','Interpreter','latex')
    ylabel('ratio magnitude','Interpreter','latex')
    title('Measured / Harmony-based YD ratios','Interpreter','latex')
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