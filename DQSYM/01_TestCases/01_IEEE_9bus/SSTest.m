%#ok<*NASGU>
% Suppresing "unused variables" warnings. The variables created in initialization
% code can be used by the child blocks or in the icon drawing code. 

% Initialization code section
function [Adc,Bdc,Cdc,Ddc,InitVec_c] = SSTest(A,B,C,InitVec,dt,N)

    I = eye(size(A,1));
    
    INVR = I/(I-A*dt/2);
    
    Ad = INVR*(I+A*dt/2);
    
    Bd = INVR*B;
    
    Cd = C*INVR*dt;
    
    Dd = Cd*B/2;

    dt2 = dt/2;

    a = -0.5+0.866i;
    a2 = -0.5-0.866i;
    Sas = [1,a,a2;1,a2,a;1,1,1]/3;

    arows = size(Ad,1)/3; %signal size
    acols = size(Ad,2)/3; %signal size

    brows = size(Bd,1)/3; %signal size
    bcols = size(Bd,2)/3; %signal size

    crows = size(Cd,1)/3; %signal size
    ccols = size(Cd,2)/3; %signal size

    drows = size(Dd,1)/3; %signal size
    dcols = size(Dd,2)/3; %signal size

    %%%%%%%%%%%%%%%%%%%%%%% Check for inconsistencies  %%%%%%%%%%%%%%%%%%%%%%%

    szMatA = size(A,1);
    iL = eye(szMatA);
    szMatiL = size(iL,1);

    if(szMatiL ~= szMatA || size(A,1) ~= size(A,2) || size(iL,1) ~= size(iL,2))
        error('[Int_DQN_Mat/swFeedback] check matrix A size!');
    end

    if(arows ~= brows || acols ~= ccols || crows ~= drows || bcols ~= dcols)
        error('[Int_DQN_Mat/Initialize] the sizes of the state-space matrices!');
    end

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    %%%%%%%%%%%%%%%%%%%%%%% A --> Bpnz %%%%%%%%%%%%%%%%%%%%%%%
    Adc = complex(zeros(3*arows,3*acols));

    for i = 1 : arows
        for j = 1 : acols
            Adc((1:3) + (i-1)*3,(1:3) + (j-1)*3) = Sas*Ad((1:3) + (i-1)*3,(1:3) + (j-1)*3)/Sas;
            %Adc((1:3) + (i-1)*3,(1:3) + (j-1)*3) = diag(diag(real(Adc((1:3) + (i-1)*3,(1:3) + (j-1)*3))));
        end
    end
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    %%%%%%%%%%%%%%%%%%%%%%% B --> Bpnz %%%%%%%%%%%%%%%%%%%%%%%

    Bdc = complex(zeros(3*brows,3*bcols));

    for i = 1 : brows
        for j = 1 : bcols
            Bdc((1:3) + (i-1)*3,(1:3) + (j-1)*3) = Sas*Bd((1:3) + (i-1)*3,(1:3) + (j-1)*3)/Sas;
            Bdc((1:3) + (i-1)*3,(1:3) + (j-1)*3) = diag(diag(real(Bdc((1:3) + (i-1)*3,(1:3) + (j-1)*3))));
        end
    end
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    %%%%%%%%%%%%%%%%%%%%%%% C --> Cpnz %%%%%%%%%%%%%%%%%%%%%%%

    Cdc = complex(zeros(3*crows,3*ccols));

    for i = 1 : crows
        for j = 1 : ccols
            Cdc((1:3) + (i-1)*3,(1:3) + (j-1)*3) = Sas*Cd((1:3) + (i-1)*3,(1:3) + (j-1)*3)/Sas;
            Cdc((1:3) + (i-1)*3,(1:3) + (j-1)*3) = diag(diag(real(Cdc((1:3) + (i-1)*3,(1:3) + (j-1)*3))));
        end
    end
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    %%%%%%%%%%%%%%%%%%%%%%% C --> Cpnz %%%%%%%%%%%%%%%%%%%%%%%

    Ddc = complex(zeros(3*drows,3*dcols));

    for i = 1 : drows
        for j = 1 : dcols
            Ddc((1:3) + (i-1)*3,(1:3) + (j-1)*3) = Sas*Dd((1:3) + (i-1)*3,(1:3) + (j-1)*3)/Sas;
            Ddc((1:3) + (i-1)*3,(1:3) + (j-1)*3) = diag(diag(real(Ddc((1:3) + (i-1)*3,(1:3) + (j-1)*3))));
        end
    end
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    Adc(abs(Adc)<10*eps) = 0;
    Bdc(abs(Bdc)<10*eps) = 0;
    Cdc(abs(Cdc)<10*eps) = 0;
    Ddc(abs(Ddc)<10*eps) = 0;


    Adc = real(Adc);
    Bdc = real(Bdc);
    Cdc = real(Cdc);
    Ddc = real(Ddc);


    InitVec_c = complex(zeros(brows*3,N+1));


    if(~isempty(InitVec))
        if(mod(numel(InitVec),3)~=0) error('[SS_DQNc_Mat] InitVec with wrong size --> needs to be a multiple of 3!'); end

        for k = 1 : numel(InitVec)/3
            InitVec_c((k-1)*3+1:(k-1)*3+3,1) = Sas*InitVec((k-1)*3+1:(k-1)*3+3);
        end
    end






end

% Parameter callback section
