function export_pv_y(freq_csv, out_csv, modelSet)
%EXPORT_PV_Y  Write MATLAB compute_PV_v23 Ydq(f) on Harmony frequencies.
    if nargin < 3 || strlength(string(modelSet)) == 0
        modelSet = 'zhao_rtds';
    end
    freq = readmatrix(freq_csv);
    freq = freq(:);
    freq = freq(isfinite(freq) & freq > 0);
    if isempty(freq)
        error('No usable frequencies in %s', freq_csv);
    end

    [Y, meta] = compute_PV_v23(freq, modelSet);

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

    fprintf('MATLAB PV %s  P=%.6f W  VLL=%.3f V  f0=%.1f Hz  Cdc=%.4g F  n=%d\n', ...
        string(meta.modelSet), meta.P_unit_W, meta.VLL_rms, meta.f0_Hz, meta.Cdc_F, numel(freq));
end

function [Y, meta] = compute_PV_v23(freq, modelSet)
% Standalone copy of Interactions_AUTH_v2 compute_PV_v23 (paper_TableV / legacy).
    key = lower(modelSet);
    switch key
        case {'zhao_rtds','zhao','historical_32mf_leftsolve','historical','legacy'}
            Cdc = 32e-3;
            matrixOrder = 'legacy';
        case {'paper_tablev','paper'}
            Cdc = 64e-3;
            matrixOrder = 'paper';
        otherwise
            error('Unknown PV_MODEL_SET "%s".', modelSet);
    end

    Nc = 36; Ncp = 4; Nmod_s = 29; Nmod_p = 164;
    Voc_ref = 21.7; Isc_ref = 3.35; Vmp_ref = 17.4; Imp_ref = 3.05;
    Ns = Nc*Nmod_s; Np = Ncp*Nmod_p;
    Voc_array = Nmod_s*Voc_ref;
    Vmp_array = Nmod_s*Vmp_ref;
    Imp_array = Np*Imp_ref;
    Pmp_array = Vmp_array*Imp_array;

    Tref = 298.15; kB = 1.380649e-23; qe = 1.602176634e-19;
    fitfun = @(n) pv_mpp_fit_error_v23(n,Ns,Voc_array,Vmp_array,Isc_ref,Imp_ref,qe,kB,Tref);
    n_eff = fzero(fitfun,[0.5 5]);
    a_oc = qe*Voc_array/(Ns*n_eff*kB*Tref);
    I0_eff = Isc_ref/(exp(a_oc)-1);

    Lboost = 250e-6; Vdc0 = 800; Cpv = 7.2e-3;
    VLL = 315; Pbase = 1e6; f1 = 50; w1 = 2*pi*f1;
    R1 = 0; L1 = 63e-6; Cf = 1500e-6; Rc = 0.051; L2 = 0;
    Vpccd = VLL*sqrt(2/3); Vpccq = 0;
    Vb = Vpccd; Ib = 2*Pbase/(3*Vb); Zb = Vb/Ib;
    Vdc_base = Vdc0; Vpv_base = 500;

    kpB_Z = 1/Vpv_base; kiB_Z = 20/Vpv_base;
    kpi_Z = 2*Zb*1; kii_Z = 2*Zb*50;
    kpdc_Z = (Ib/Vdc_base)*5; kidc_Z = (Ib/Vdc_base)*100;
    kppll_Z = (2*pi/Vb)*30; kipll_Z = (2*pi/Vb)*200;

    Ppv = Pmp_array; Vpv0 = Vmp_array; Ipv0 = Imp_array;
    D = 1-Vpv0/Vdc0; IL0 = Ipv0;
    I2d0 = 2*Ppv/(3*Vpccd); I2q0 = 0;
    den_cf = 1+(w1*Rc*Cf)^2;
    Vcfd = (Vpccd-w1*L2*I2q0+w1^2*Rc*Cf*L2*I2d0+Rc*w1*Cf*Vpccq)/den_cf;
    Vcfq = (Vpccq+w1*L2*I2d0+w1^2*Rc*Cf*L2*I2q0-Rc*w1*Cf*Vpccd)/den_cf;
    I1d0 = I2d0-w1*Cf*Vcfq; I1q0 = I2q0+w1*Cf*Vcfd;
    Vod = Vpccd+R1*I1d0-w1*L1*I1q0-w1*L2*I2q0;
    Voq = Vpccq+R1*I1q0+w1*L1*I1d0+w1*L2*I2d0;
    Md0 = 2*Vod/Vdc0; Mq0 = 2*Voq/Vdc0;

    kpv = -qe*(Np*I0_eff+Np*Isc_ref-Ipv0)/(Ns*n_eff*kB*Tref);
    vth = Ns*n_eff*kB*Tref/qe;
    kmp = vth^2/(I0_eff*Np*Vpv0*exp(qe*Vpv0/(Ns*n_eff*kB*Tref))+(Ipv0/Vpv0)*vth^2);
    lambda = kpv*kmp;

    I2 = eye(2); Nm = [Md0 Mq0;0 0]; Ni = [I1d0 I1q0;0 0];
    Hm = [Md0/2 0;Mq0/2 0]; Hv = Vdc0/2*I2;
    Y = complex(zeros(numel(freq),2,2));

    for kk = 1:numel(freq)
        s = 1j*2*pi*freq(kk);
        gB = kpB_Z+kiB_Z/s;
        num_zdc = s*Lboost*(s*Cpv-kpv)+1+gB*Vdc0*(1-lambda);
        den_zdc = (1-D)^2*(kpv-s*Cpv)+(D-1)*gB*IL0*(1-lambda) ...
            -s*Cdc-s*Cdc*Vdc0*gB*(1-lambda)+s^2*Lboost*Cdc*(kpv-s*Cpv);
        zdc = num_zdc/den_zdc;
        Zdc = -[zdc 0;0 0];

        Yc = [s*Cf -w1*Cf;w1*Cf s*Cf];
        Zrl1 = [R1+s*L1 -w1*L1;w1*L1 R1+s*L1];
        Zrc = Rc*I2; Zl2 = [s*L2 -w1*L2;w1*L2 s*L2];
        Arc = Zrc*Yc+I2;
        Gii = Nm*Yc*(Arc\Zl2)+Nm;
        Gvi = Nm*Yc/Arc; Gmi = Ni;
        H1 = Zrl1*Yc/Arc+I2;
        Gvv = H1\Hm; Gmv = H1\Hv; Giv = -(H1\(H1*Zl2+Zrl1));

        Gdc = [kpdc_Z+kidc_Z/s 0;0 0];
        Gi = (kpi_Z+kii_Z/s)*I2; Gv = I2/Vdc0;
        Hpll = kppll_Z+kipll_Z/s; Gpll = Hpll/(s+Vpccd*Hpll);
        Gpllm = [0 Gpll*Mq0;0 -Gpll*Md0];
        Gplli = [0 Gpll*I2q0;0 -Gpll*I2d0];

        JA = Gvv+Gmv*Gv*Gi*Gdc;
        JB = Giv-Gmv*Gv*Gi;
        JC = Gmv*(Gv*Gi*Gplli+Gpllm)+I2;
        FA = I2-Zdc*Gmi*Gv*Gi*Gdc;
        FB = Zdc*Gii-Zdc*Gmi*Gv*Gi;
        FC = Zdc*Gvi-Zdc*Gmi*(Gv*Gi*Gplli+Gpllm);
        A = JC-JA*(FA\FC);
        B = JA*(FA\FB)+JB;

        if strcmp(matrixOrder,'paper')
            Zpv = -B/A;
        else
            Zpv = -(A\B);
        end
        Y(kk,:,:) = reshape(Zpv\I2,1,2,2);
    end

    meta = struct('name','PV','P_unit_W',Pmp_array,'VLL_rms',VLL,'f0_Hz',f1, ...
        'fmax_Hz',1000,'modelSet',modelSet,'Cdc_F',Cdc, ...
        'n_eff',n_eff,'I0_eff',I0_eff,'kpB_Z',kpB_Z,'kiB_Z',kiB_Z, ...
        'kpdc_Z',kpdc_Z,'kidc_Z',kidc_Z,'kpi_Z',kpi_Z,'kii_Z',kii_Z, ...
        'kppll_Z',kppll_Z,'kipll_Z',kipll_Z);
end

function err = pv_mpp_fit_error_v23(n,Ns,Voc,Vmp,Isc,Imp,q,k,T)
    a = q/(Ns*n*k*T);
    I0 = Isc/(exp(a*Voc)-1);
    err = Isc-I0*(exp(a*Vmp)-1)-Imp;
end
