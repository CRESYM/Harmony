function export_wt4_y(freq_csv, out_csv)
%EXPORT_WT4_Y  MATLAB WT4_RTDS_Implementation_v2 reduced-L Ydq(f).
    freq = readmatrix(freq_csv);
    freq = freq(:);
    freq = freq(isfinite(freq) & freq > 0);
    if isempty(freq)
        error('No usable frequencies in %s', freq_csv);
    end

    [Y, meta] = compute_WT4_rtds(freq);

    fid = fopen(out_csv, 'w');
    if fid < 0
        error('Cannot write %s', out_csv);
    end
    cleaner = onCleanup(@() fclose(fid));
    fprintf(fid, 'freq_Hz,Re_Ydd,Im_Ydd,Re_Ydq,Im_Ydq,Re_Yqd,Im_Yqd,Re_Yqq,Im_Yqq\n');
    for k = 1:numel(freq)
        Yk = squeeze(Y(k,:,:));
        fprintf(fid, '%.16g,%.16g,%.16g,%.16g,%.16g,%.16g,%.16g,%.16g,%.16g\n', ...
            freq(k), ...
            real(Yk(1,1)), imag(Yk(1,1)), ...
            real(Yk(1,2)), imag(Yk(1,2)), ...
            real(Yk(2,1)), imag(Yk(2,1)), ...
            real(Yk(2,2)), imag(Yk(2,2)));
    end

    fprintf('MATLAB WT4 RTDS reduced-L  P=%.6f W  VLL=%.3f V  f0=%.1f Hz  n=%d\n', ...
        meta.P_unit_W, meta.VLL_rms, meta.f0_Hz, numel(freq));
end

function [Y, meta] = compute_WT4_rtds(freq)
    p.PWT = 2.5e6; p.Sbase = 2.5e6; p.Vdc = 1200; p.VLL = 690;
    p.f0 = 50; p.w0 = 2*pi*p.f0; p.fsw = 2000; p.Tdel = 1.5/p.fsw;
    p.Rseries = 0; p.Lf = 0.12e-3;
    p.kpi_pu = 1; p.kii_pu = 50; p.kpPLL_pu = 30; p.kiPLL_pu = 200;
    p.omega_n_LPF = 1.23e6; p.zeta_LPF = 4.74e-13;

    p.Vb = p.VLL*sqrt(2/3);
    p.Ib = 2*p.Sbase/(3*p.Vb);
    p.Zb = p.Vb/p.Ib;
    p.Vd0 = p.Vb; p.Vq0 = 0;
    p.Id0 = -p.PWT/(1.5*p.Vd0); p.Iq0 = 0;
    p.Dd0 = (p.Vd0 - p.Rseries*p.Id0 - p.w0*p.Lf*p.Iq0)/p.Vdc;
    p.Dq0 = (p.Vq0 - p.Rseries*p.Iq0 + p.w0*p.Lf*p.Id0)/p.Vdc;

    freq = freq(:).'; N = numel(freq);
    Ydq = complex(zeros(2,2,N));
    I2 = eye(2); J = [0 -1; 1 0];
    for k = 1:N
        s = 1j*2*pi*freq(k);
        Ddq = s*I2 + p.w0*J;
        Zout = p.Rseries*I2 + p.Lf*Ddq;
        Yout = inv(Zout);
        Gid = -p.Vdc*Yout;
        Gmf_s = p.omega_n_LPF^2 / (s^2 + 2*p.zeta_LPF*p.omega_n_LPF*s + p.omega_n_LPF^2);
        Gmf = Gmf_s*I2;
        Gdel_s = (1 - 0.5*p.Tdel*s)/(1 + 0.5*p.Tdel*s);
        Gdel = Gdel_s*I2;
        Gci_pu = p.kpi_pu + p.kii_pu/s;
        Gpi = (p.Zb/p.Vdc)*Gci_pu*I2;
        Gdec = (p.w0*p.Lf/p.Vdc^2)*J;
        Gcc = Gdec - Gpi;
        GcPLL_pu = p.kpPLL_pu + p.kiPLL_pu/s;
        Hpll = (GcPLL_pu/(s + GcPLL_pu))/p.Vb;
        Hi = [0, p.Iq0*Hpll; 0, -p.Id0*Hpll];
        Hd = [0, -p.Dq0*Hpll; 0, p.Dd0*Hpll];
        A = I2 + Gdel*Gid*Gcc*Gmf;
        B = Yout + Gid*Gdel*(Hd + Gcc*Hi)*Gmf;
        Ydq(:,:,k) = B/A;
    end
    Y = permute(Ydq,[3 1 2]);
    meta = struct('name','WT4','P_unit_W',p.PWT,'VLL_rms',p.VLL,'f0_Hz',p.f0);
end
