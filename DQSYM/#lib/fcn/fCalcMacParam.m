function SMRD = fCalcMacParam(Rs, NomParam, reactVec, timeConstVec, lfVec, Mech, Ts)


%#ok<*NASGU>
% Suppresing "unused variables" warnings. The variables created in initialization
% code can be used by the child blocks or in the icon drawing code. 

% Initialization code section
SMRD = struct();

    SMRD.dwo = 0;

    SMRD.nState = numel(reactVec)-1;
    
    if(SMRD.nState>5)
        SMRD.RLinv   = zeros(6,6);
        SMRD.Linv    = zeros(6,6);
        SMRD.L       = zeros(6,6);
        SMRD.R       = zeros(6,6);
        W = zeros(6,6);
    else
        SMRD.RLinv   = zeros(5,5);
        SMRD.Linv    = zeros(5,5);
        SMRD.L       = zeros(5,5);
        SMRD.R       = zeros(5,5);
        W            = zeros(5,5);
    end

    SMRD.Rs      = Rs;

    SMRD.N       = 1;
    
    SMRD.H       = Mech(1);
    SMRD.F       = Mech(2);
    SMRD.p       = Mech(3);

    SMRD.Pn = NomParam(1);
    SMRD.Vn = NomParam(2);
    SMRD.fn = NomParam(3);
    
    
    Vb = SMRD.Vn*sqrt(2/3);
    Ib = sqrt(2/3)*SMRD.Pn/SMRD.Vn;
    
    
    Td_p    = timeConstVec(1);
    Td_pp   = timeConstVec(2);
    if(SMRD.nState>5)
        Tq_p    = timeConstVec(3);
        Tq_pp   = timeConstVec(4);
    else
        Tq_pp   = timeConstVec(3);
    end
    
    
    %Set the time constants in Lq(s)
    %See (eq. 7.2.14)
    
    wn=2*pi*SMRD.fn;
    SMRD.wn = wn;%Rated frequency in rad/sec
    %Implementation of the formulas in 
    
    % Xaq=1.165; %Mutual inductance
    % Ll=.188; %Stator leakage inductance
    
    %T5=0.0234;
    
    
    Xd      = reactVec(1);
    Xd_p    = reactVec(2);
    Xd_pp   = reactVec(3);
    Xq      = reactVec(4);


    if(SMRD.nState>5)
        Xq_p    = reactVec(5);
        Xq_pp   = reactVec(6);
        Xl      = reactVec(7);

        Tqo_p   = Tq_p * Xq/Xq_p;
        Tqo_pp  = Tq_pp * Xq_p/Xq_pp;
    else
        Xq_pp   = reactVec(5);
        Xl      = reactVec(6);
        Tqo_pp  = Tq_pp * Xq/Xq_pp;
    end


    Xad     = Xd-Xl; %Mutual inductance
    Xaq     = Xq-Xl; %Mutual inductance
    
    
    SMRD.Xd = Xd;
    SMRD.Xq = Xq;
    SMRD.Xl = Xl;
    SMRD.Xad = Xad;
    SMRD.Xaq = Xaq;
    
        
   
    LAM1 = sqrt(Xd^2*Xd_pp^2*(Td_p-Td_pp)^2+2*Td_p*Td_pp*Xd*Xd_p*Xd_pp*(Xd-2*Xd_p+Xd_pp)-2*Td_pp*Xd*Xd_p*Xd_pp*(Xd-Xd_p+Xd_pp)+Td_pp^2*Xd_pp^2*(Xd^2+Xd_pp^2));
    Tdo_p  = (Td_p*Xd*Xd_pp+LAM1+Td_pp*(Xd*Xd_p+Xd_p*Xd_pp-Xd*Xd_pp))/(2*Xd_p*Xd_pp);
    Tdo_pp = Xd*Td_p*Td_pp/(Xd_pp*Tdo_p);
    % 
    % Tqo_p   = Tq_p * Xq/Xq_p;
    % Tqo_pp  = Tq_pp * Xq_p/Xq_pp;


    % D-Axis
    a=SMRD.Xad*(Xd_p-SMRD.Xl)/(Xd-Xd_p);
    b=a*SMRD.Xad*(Xd_pp-SMRD.Xl)/(a*Xd+SMRD.Xl*SMRD.Xad-(a+SMRD.Xad)*Xd_pp);
    c=(b+a*SMRD.Xl*SMRD.Xad/(SMRD.Xl*SMRD.Xad+a*Xd))/Td_pp;
    
    T5 = b/c;

    if(1)
        a = Xad*(Xd_p-Xl)/(Xd-Xd_p);
        b = Xad*(Xd_pp-Xl)/(a*Xd+Xl*Xad-(a+Xad)*Xd_pp);
        c = (1/Td_pp)*(b-(a*Xl*Xad)/(Xl*Xad+a*Xd) );
        
        T11d = b/c;
    end

    
    if(1)
        %Set the time constants in Ld(s),sG(s)
        %See (eq. 7.2.4-7.2.5)
        T1 = Td_p; %T1=1.419; %Td'
        T2 = Td_pp; %T2=0.0669; %Td''
        T3 = Tdo_p; %T3=5.62; %Tdo'
        T4 = Tdo_pp; %T4=0.09; %Tdo''
        %T5 = T5; %T5=0.0234;
        Ld = Xd; %Ld= 1.104; %Synchronous inductance
        Ll = Xl; %Ll=0.198; %Stator leakage inductance
        Lad = Xad; %Lad=Ld-Ll; %Mutual inductance
        %Rated frequency in rad/sec
        %Implementation of the formulas in

        T5 = T11d;

        Ro=wn*Ld*(T3+T4-T1-T2);
        Ro=Lad*Lad/Ro;
        a=Ld*(T1+T2)-Ll*(T3+T4);a=a/Lad;
        b=Ld*T1*T2-Ll*T3*T4; b=b/Lad;
        c=(T3*T4-T1*T2)/(T3+T4-T1-T2);

        Lf1d=b-a*T5+T5*T5; %in p.u.
        Lf1d=Ro*Lf1d/(c-T5); %in p.u.
        Rfd=Ro*(Lf1d+Ro*(2*T5-a));
        Rfd=Rfd/(Lf1d+Ro*(T5+c-a));%in p.u.

        R1d=Ro*Rfd/(Rfd-Ro); %in p.u.
        %Inductances in p.u.
        L1d=R1d*T5; 
        L1d=L1d*wn;

        Lfd=Rfd*(a-T5-Lf1d/Ro); %in p.u.
        Lfd=Lfd*wn; 
        Lf1d=Lf1d*wn;

        SMRD.Lf1d = Lf1d;
        SMRD.Rfd = Rfd;
        SMRD.R1d = R1d;
        SMRD.L1d = L1d;
        SMRD.Lfd = Lfd;
    end


    %%%%%%%%%%%%%%%%%%%%%%%%% Q AXIS %%%%%%%%%%%%%%%%%%%%%%%%% Tqo′ Xq″ Xq SMRD.Xl 
    
    if(SMRD.nState>5)
        %conv(x,y)= polynomial multiplication
        N=conv([Tq_p 1],[Tq_pp 1]); 
        D=conv([Tqo_p 1],[Tqo_pp 1]);
        Nz=conv(N-D*SMRD.Xl/Xq,[Xaq/SMRD.wn 0]);
        Dz=D-N; Nz=Nz(1:3); Dz=Dz(1:2);
        Req=Nz(3)/Dz(2);
        Nz=Nz/Nz(3); Dz=Dz/Dz(2);
        %compute polynomial roots and sort them
        Tab=sort(abs(roots(Nz))); Tab=1./Tab;
        Tm=sort(abs(roots(Dz))); Tm=1/Tm(1);
        %Solve a system of two linear equa�tions
        x=[1 1;Tab(1) Tab(2)]\[1 ;Tm]/Req;
        %Results
        SMRD.R1q=1/x(1); 
        SMRD.L1q=SMRD.wn*SMRD.R1q*Tab(1); %in p.u.
        SMRD.R2q=1/x(2); 
        SMRD.L2q=SMRD.wn*SMRD.R2q*Tab(2); %in p.u.
    else
        N=conv([1],[Tq_pp 1]); 
        D=conv([1],[Tqo_pp 1]);
        Nz=conv(N-D*SMRD.Xl/Xq,[Xaq/SMRD.wn 0]);
        Dz=D-N; Nz=Nz(1:2); Dz=Dz(1:1);
        Req=Nz(2)/Dz(1);
        Nz=Nz/Nz(2); Dz=Dz/Dz(1);
        %compute polynomial roots and sort them
        Tab=sort(abs(roots(Nz))); Tab=1./Tab;
        Tm=1;
        %Solve a system of two linear equa�tions
        x=[1;Tab(1)]/Req;
        %Results
        SMRD.R1q=1/x(1); 
        SMRD.L1q=SMRD.wn*SMRD.R1q*Tab(1); %in p.u.
    end
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    %%%%%%%%%%%%%%%%%%%%%%%%% R/L MAT %%%%%%%%%%%%%%%%%%%%%%%%%

    q_axis = 1;


    if(SMRD.nState>5) % D = 2,3,4 ; Q = 1,5,6

        if(q_axis)
            SMRD.L(2,2) = SMRD.Xd;  SMRD.L(2,3) = SMRD.Xad;                         SMRD.L(2,4) = SMRD.Xad; %D
            SMRD.L(3,2) = SMRD.Xad; SMRD.L(3,3) = SMRD.Xad + SMRD.Lf1d + SMRD.Lfd;  SMRD.L(3,4) = SMRD.Lf1d + SMRD.Xad; %D
            SMRD.L(4,2) = SMRD.Xad; SMRD.L(4,3) = SMRD.Xad + SMRD.Lf1d;             SMRD.L(4,4) = SMRD.Lf1d + SMRD.Xad + SMRD.L1d; %D

            SMRD.L(1,1) = SMRD.Xq;  SMRD.L(1,5) = SMRD.Xaq;                         SMRD.L(1,6) = SMRD.Xaq; %Q
            SMRD.L(5,1) = SMRD.Xaq; SMRD.L(5,5) = SMRD.Xaq + SMRD.L1q;              SMRD.L(5,6) = SMRD.Xaq; %Q
            SMRD.L(6,1) = SMRD.Xaq; SMRD.L(6,5) = SMRD.Xaq;                         SMRD.L(6,6) = SMRD.Xaq + SMRD.L2q; %Q


            SMRD.R(1,1) = Rs;
            SMRD.R(2,2) = Rs;
            SMRD.R(3,3) = SMRD.Rfd;
            SMRD.R(4,4) = SMRD.R1d;
            SMRD.R(5,5) = SMRD.R1q;
            SMRD.R(6,6) = SMRD.R2q;

        else
            SMRD.L(1,1) = SMRD.Xd;  SMRD.L(1,3) = SMRD.Xad;                         SMRD.L(1,5) = SMRD.Xad;
            SMRD.L(2,2) = SMRD.Xq;  SMRD.L(2,4) = SMRD.Xaq;                         SMRD.L(2,6) = SMRD.Xaq;
            SMRD.L(3,1) = SMRD.Xad; SMRD.L(3,3) = SMRD.Xad + SMRD.Lf1d + SMRD.Lfd;  SMRD.L(3,5) = SMRD.Lf1d + SMRD.Xad;
            SMRD.L(4,2) = SMRD.Xaq; SMRD.L(4,4) = SMRD.Xaq + SMRD.L1q;              SMRD.L(4,6) = SMRD.Xaq;
            SMRD.L(5,1) = SMRD.Xad; SMRD.L(5,3) = SMRD.Xad + SMRD.Lf1d;             SMRD.L(5,5) = SMRD.Lf1d + SMRD.Xad + SMRD.L1d;
            SMRD.L(6,2) = SMRD.Xaq; SMRD.L(6,4) = SMRD.Xaq;                         SMRD.L(6,6) = SMRD.Xaq  + SMRD.L2q;

            SMRD.R(1,1) = Rs;
            SMRD.R(2,2) = Rs;
            SMRD.R(3,3) = SMRD.Rfd;
            SMRD.R(4,4) = SMRD.R1q;
            SMRD.R(5,5) = SMRD.R1d;
            SMRD.R(6,6) = SMRD.R2q;
        end
    else

        if(q_axis)
            SMRD.L(2,2) = SMRD.Xd;  SMRD.L(2,3) = SMRD.Xad;                         SMRD.L(2,4) = SMRD.Xad;           %D
            SMRD.L(3,2) = SMRD.Xad; SMRD.L(3,3) = SMRD.Xad + SMRD.Lf1d + SMRD.Lfd;  SMRD.L(3,4) = SMRD.Lf1d + SMRD.Xad; %D
            SMRD.L(4,2) = SMRD.Xad; SMRD.L(4,3) = SMRD.Xad + SMRD.Lf1d;             SMRD.L(4,4) = SMRD.Lf1d + SMRD.Xad + SMRD.L1d;  %D
            
            SMRD.L(1,1) = SMRD.Xq;  SMRD.L(1,5) = SMRD.Xaq;              %Q
            SMRD.L(5,1) = SMRD.Xaq; SMRD.L(5,5) = SMRD.Xaq + SMRD.L1q;   %Q

            SMRD.R(1,1) = Rs;
            SMRD.R(2,2) = Rs;
            SMRD.R(3,3) = SMRD.Rfd;
            SMRD.R(4,4) = SMRD.R1d;
            SMRD.R(5,5) = SMRD.R1q;

        else
            SMRD.L(1,1) = SMRD.Xd; SMRD.L(1,3) = SMRD.Xad; SMRD.L(1,5) = SMRD.Xad;
            SMRD.L(2,2) = SMRD.Xq; SMRD.L(2,4) = SMRD.Xaq;
            SMRD.L(3,1) = SMRD.Xad; SMRD.L(3,3) = SMRD.Xad + SMRD.Lf1d + SMRD.L1d; SMRD.L(3,5) = SMRD.Lf1d + SMRD.Xad;
            SMRD.L(4,2) = SMRD.Xaq; SMRD.L(4,4) = SMRD.Xaq + SMRD.L1q; 
            SMRD.L(5,1) = SMRD.Xad; SMRD.L(5,3) = SMRD.Xad + SMRD.Lf1d; SMRD.L(5,5) = SMRD.Lf1d + SMRD.Xad + SMRD.Lfd;

            SMRD.R(1,1) = Rs;
            SMRD.R(2,2) = Rs;
            SMRD.R(3,3) = SMRD.Rfd;
            SMRD.R(4,4) = SMRD.R1q;
            SMRD.R(5,5) = SMRD.R1d;

        end
    end


   if(q_axis)
        W(2,1) = -1;
        W(1,2) = +1;
   else
        W(2,1) = +1;
        W(1,2) = -1;
   end
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%% LF  %%%%%%%%%%%%%%%%%%%%%%%%%
    
    Pt = lfVec(1);
    Qt = lfVec(2);
    Vt = lfVec(3);
    Et = lfVec(4);
    At = lfVec(5);
    % if(numel(lfVec)>5) fac = lfVec(6); end
    
    phi = At;
    SMRD.phi = phi;

    SMRD.Vt = Vt*exp(1i*(phi));
    SMRD.It = (Pt-1i*Qt)/(Vt*exp(-1i*(phi)));
       
    %fi = sign(Qt)*acos(Pt/(Vt*It)); rad2deg(fi);
    fi = angle(SMRD.Vt)-angle(SMRD.It);
    SMRD.fi = fi;
    
    % del = (atan((Xq*It*cos(fi)-Rs*It*sin(fi))/(Vt+Rs*It*cos(fi)+Xq*It*sin(fi))));
    del = angle(SMRD.Vt+(SMRD.Rs+1i*SMRD.Xq)*SMRD.It)-(SMRD.phi);

    SMRD.del = del;

    if(0)
    SMRD.Vfnp = ((SMRD.Rfd*Vb/(Xad*wn))^2);
    else
    SMRD.Vfnp = SMRD.Rfd/SMRD.Xad;
    end
    
    v_dq = Vt*exp(1i*(pi/2-del)); %correct
    ed = real(v_dq);
    eq = imag(v_dq);
    efd = Et*SMRD.Vfnp;
    
   if(q_axis)
        v_vec = [eq; ed; efd; zeros(SMRD.nState-3,1)];
        i_vec = (v_vec'/(SMRD.R-SMRD.L*W))';
   else
        v_vec = [ed; eq; efd; zeros(SMRD.nState-3,1)];
        i_vec = (v_vec'/(SMRD.R-SMRD.L*W))';
   end

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    SMRD.idq0 = i_vec;
    SMRD.phidq0 = SMRD.L*i_vec;
    
    SMRD.dwo = 0;

    SMRD.Linv = eye(SMRD.nState)/SMRD.L;
    
    if(1)
        SMRD.Linv2 = SMRD.Linv;
        SMRD.Linv2(3,4) = 0; SMRD.Linv2(4,3) = 0;
    end


    SMRD.RLinv = SMRD.R*SMRD.Linv;


    SMRD.Pb = 1;
    SMRD.Ib = Ib;
    SMRD.Vb = Vb;

    SMRD.IqdSign = [-1    -1     ones(1,SMRD.nState-2)];


    SMRD.Ts = Ts;
    SMRD.web = SMRD.wn;
    SMRD.v_dq = v_dq;

    tho = del+phi-pi/2;
    SMRD.tho = tho;


% Parameter callback section


end



%iDQ    = (0.9453+1i*0.2655); iDQ_mag = 0.9819
%idq    = (0.8677+1i*0.4595); idq_mag = 0.9819
% [ 0.4596    0.8676    1.2097         0    0.0000         0]
%phidq0 = [0.4376   -0.8089    0.7672   -0.7400    0.4013   -0.7400]


% psi_d = +eq+Rs*iq; %correct
% psi_q = -ed-Rs*id; %correct
% psi_fd = (Xad+SMR.Lf1d + SMR.L1d)*ifd - Xad*id; %(1.6600+0.1649)*1.2097 - 1.6600*0.8677
% psi_1q = -Xaq*iq;
% psi_1d = Xad*(ifd-id);
% psi_2q = -Xaq*iq;



% 
% fac = 22/24;
% 
% I_DQ = (Pt-1i*Qt)/(Vt*exp(-1i*phi))/fac; % Correct!
% I_dq = I_DQ*exp(-1i*tho);
% 
% 
% 
% 
% SMR.N = ((Rfd*Vb/(Lmd*wn))^2);
% 
% 
% del = deg2rad(61.59);
% 
% 
% id = 0.8677;
% iq = 0.4595;
% 
% v_dq = Vt*fac*exp(1i*(pi/2-del)); %correct
% ed = real(v_dq);
% eq = imag(v_dq);
% 
% 
% 
% ifd = (eq+Rs*iq+Xd*id)/Xad; %correct
% 
% efd = Rfd*if