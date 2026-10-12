function export_wt3_y(freq_csv, out_csv)
%EXPORT_WT3_Y  MATLAB WT3_DFIG_SequenceImpedance_2p5MW_v6_directEval Ydq(f).
    freq = readmatrix(freq_csv);
    freq = freq(:);
    freq = freq(isfinite(freq) & freq > 0);
    if isempty(freq)
        error('No usable frequencies in %s', freq_csv);
    end

    [Y, meta] = compute_WT3_direct(freq);

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

    fprintf('MATLAB WT3 paper_2p5MW  P=%.6f W  VLL=%.3f V  slip=%+.3f  n=%d\n', ...
        meta.P_unit_W, meta.VLL_rms, meta.slip, numel(freq));
end

function [Y, meta] = compute_WT3_direct(freq)
    par.f1 = 50; par.w1 = 2*pi*par.f1;
    par.Sbase = 2.5e6; par.VLLbase = 690; par.Vdc = 1200;
    par.Vpk_base = par.VLLbase*sqrt(2/3);
    par.Zb_s = par.VLLbase^2/par.Sbase; par.Lb_s = par.Zb_s/par.w1;
    par.Rs_pu = 0.01; par.Rr_pu = 0.006; par.Xls_pu = 0.102;
    par.Xlr_pu = 0.08596; par.Xm_pu = 4.348;
    par.Ns = 1; par.Nr = 2.6377; par.a = par.Ns/par.Nr; par.a2 = par.a^2;
    par.Rs = par.Rs_pu*par.Zb_s; par.Lls = par.Xls_pu*par.Lb_s;
    par.Lm = par.Xm_pu*par.Lb_s;
    par.VLLbase_r = par.VLLbase/par.a; par.Zb_r = par.VLLbase_r^2/par.Sbase;
    par.Lb_r = par.Zb_r/par.w1;
    par.Rr_rot = par.Rr_pu*par.Zb_r; par.Llr_rot = par.Xlr_pu*par.Lb_r;
    par.Rr_ref = par.a2*par.Rr_rot; par.Llr_ref = par.a2*par.Llr_rot;
    par.Ls = par.Lls+par.Lm; par.Lr_ref = par.Llr_ref+par.Lm;
    par.Lsr = par.Lls+par.Llr_ref;
    par.Kp_pll = 30; par.Ki_pll = 200;
    par.Krp_pu = 0.5; par.Kri_pu = 25;
    par.Krp_rot = par.Krp_pu*par.Zb_r; par.Kri_rot = par.Kri_pu*par.Zb_r;
    par.Ksp_pu = 1; par.Ksi_pu = 50;
    par.Ksp = par.Ksp_pu*par.Zb_s; par.Ksi = par.Ksi_pu*par.Zb_s;
    par.Krd_rot = 0; par.Ksd = 0; par.Rf = 0; par.Lf = 0.3e-3;
    par.V1 = par.Vpk_base;

    slip = -0.35; Ptarget = 2.5e6; Qs = 0; Qgsc = 0;
    fun = @(Ps) wt3_mismatch(Ps,Qs,slip,Ptarget,par);
    Plo = 0.2*Ptarget; Phi = 1.8*Ptarget;
    if sign(fun(Plo))==sign(fun(Phi))
        Ps = fzero(fun,Ptarget/(1-slip));
    else
        Ps = fzero(fun,[Plo Phi]);
    end
    op = wt3_op(Ps,Qs,slip,par);
    op.Pgsc = op.Protor_out; op.Qgsc = Qgsc;
    op.Sgsc_out = op.Pgsc+1j*op.Qgsc;
    op.Ic1 = conj(op.Sgsc_out/((3/2)*par.V1));
    op.Zf_w1 = par.Rf+1j*par.w1*par.Lf;
    op.Vgs = par.V1+op.Ic1*op.Zf_w1;
    sigma_p_w1 = (1j*par.w1-1j*op.wr)/(1j*par.w1);
    op.Zeq_RSC_w1 = 1j*par.w1*par.Lsr+par.Rs+par.Rr_ref/sigma_p_w1;
    op.Vrs_seq = par.V1+op.Ir1_ref*op.Zeq_RSC_w1;

    Ydq = wt3_eval(freq,op,par);
    Y = permute(Ydq,[3 1 2]);
    meta = struct('name','WT3','P_unit_W',Ptarget,'VLL_rms',par.VLLbase, ...
        'f0_Hz',par.f1,'slip',slip);
end

function mismatch = wt3_mismatch(Ps,Qs,slip,Ptarget,par)
    op = wt3_op(Ps,Qs,slip,par);
    mismatch = Ps+op.Protor_out-Ptarget;
end

function op = wt3_op(Ps,Qs,slip,par)
    op.slip = slip; op.wr = (1-slip)*par.w1; op.ws = par.w1-op.wr;
    Ss_abs = -(Ps+1j*Qs);
    op.Is1 = conj(Ss_abs/((3/2)*par.V1));
    op.Ir1_ref = (par.V1-(par.Rs+1j*par.w1*par.Ls)*op.Is1)/(1j*par.w1*par.Lm);
    op.Vr1_ref = 1j*op.ws*par.Lm*op.Is1+(par.Rr_ref+1j*op.ws*par.Lr_ref)*op.Ir1_ref;
    op.Srotor_abs = (3/2)*op.Vr1_ref*conj(op.Ir1_ref);
    op.Protor_out = -real(op.Srotor_abs); op.Qrotor_out = -imag(op.Srotor_abs);
end

function Ydq = wt3_eval(freq,op,par)
    freq = freq(:).'; N = numel(freq); sjw = 1j*2*pi*freq;
    sp = sjw-1j*par.w1; sn = sjw+1j*par.w1;
    sigma_p = (sjw-1j*op.wr)./sjw; sigma_n = (sjw+1j*op.wr)./sjw;
    V1pll = abs(par.V1)/par.Vpk_base;
    Apll_p = par.Kp_pll*sp+par.Ki_pll; Apll_n = par.Kp_pll*sn+par.Ki_pll;
    Tpll_p = V1pll*Apll_p./(sp.^2+V1pll*Apll_p);
    Tpll_n = V1pll*Apll_n./(sn.^2+V1pll*Apll_n);
    Hri_p = par.a2*(par.Krp_rot+par.Kri_rot./sp);
    Hri_n = par.a2*(par.Krp_rot+par.Kri_rot./sn);
    Krd = par.a2*par.Krd_rot;
    Hsi_p = par.Ksp+par.Ksi./sp; Hsi_n = par.Ksp+par.Ksi./sn;

    ZRSp = (sjw*par.Lsr+par.Rs+par.Rr_ref./sigma_p+(Hri_p-1j*Krd)./sigma_p) ./ ...
        (1-(Tpll_p/2).*((op.Ir1_ref/par.V1).*(Hri_p-1j*Krd)./sigma_p+op.Vrs_seq/par.V1));
    ZRSn = (sjw*par.Lsr+par.Rs+par.Rr_ref./sigma_n+(Hri_n+1j*Krd)./sigma_n) ./ ...
        (1-(Tpll_n/2).*((conj(op.Ir1_ref)/conj(par.V1)).*(Hri_n+1j*Krd)./sigma_n+conj(op.Vrs_seq)/conj(par.V1)));
    Zf = par.Rf+sjw*par.Lf;
    ZGSp = (Zf+Hsi_p-1j*par.Ksd) ./ ...
        (1-(Tpll_p/2).*((op.Ic1/par.V1).*(Hsi_p-1j*Krd)+op.Vgs/par.V1));
    ZGSn = (Zf+Hsi_n+1j*par.Ksd) ./ ...
        (1-(Tpll_n/2).*((conj(op.Ic1)/conj(par.V1)).*(Hsi_n+1j*Krd)+conj(op.Vgs)/conj(par.V1)));
    Yp = 1./ZRSp+1./ZGSp; Yn = 1./ZRSn+1./ZGSn;
    Ydd = 0.5*(Yp+Yn); Ycross = 0.5j*(Yp-Yn);
    Ydq = complex(zeros(2,2,N));
    Ydq(1,1,:) = reshape(Ydd,1,1,N); Ydq(1,2,:) = reshape(Ycross,1,1,N);
    Ydq(2,1,:) = reshape(-Ycross,1,1,N); Ydq(2,2,:) = reshape(Ydd,1,1,N);
end
