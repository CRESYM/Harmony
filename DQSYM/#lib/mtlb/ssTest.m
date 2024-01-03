
I = eye(size(A,1));
INVR = I/(I-A*dt/2);
Ad1 = INVR*(I+A*dt/2);
Bd1 = INVR*B*dt/2;
Cd1 = C*Ad1;
Dd11 = C*INVR*B*dt/2+D;
Dd12 = C*INVR*B*dt/2;

%Calculating the PNZ matrices

    a = -0.5+0.866i;
    a2 = -0.5-0.866i;
    Sas = [1,a,a2;1,a2,a;1,1,1]/3;

    arows = size(Ad1,1)/3; %signal size
    acols = size(Ad1,2)/3; %signal size

    brows = size(Bd1,1)/3; %signal size
    bcols = size(Bd1,2)/3; %signal size

    crows = size(Cd1,1)/3; %signal size
    ccols = size(Cd1,2)/3; %signal size

    d1rows = size(Dd11,1)/3; %signal size
    d1cols = size(Dd11,2)/3; %signal size

    d2rows = size(Dd12,1)/3; %signal size
    d2cols = size(Dd12,2)/3; %signal size


    %%%%%%%%%%%%%%%%%%%%%%% Check for inconsistencies  %%%%%%%%%%%%%%%%%%%%%%%

    szMatA = size(A,1);
    iL = eye(szMatA);
    szMatiL = size(iL,1);

    if(szMatiL ~= szMatA || size(A,1) ~= size(A,2) || size(iL,1) ~= size(iL,2))
        error('[Int_DQN_Mat/swFeedback] check matrix A size!');
    end

    if(arows ~= brows || acols ~= ccols || crows ~= d1rows || bcols ~= d1cols)
        error('[Int_DQN_Mat/Initialize] the sizes of the state-space matrices!');
    end

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


    %%%%%%%%%%%%%%%%%%%%%%% A --> Bpnz %%%%%%%%%%%%%%%%%%%%%%%
    Adc = complex(zeros(3*arows,3*acols));

    for i = 1 : arows
        for j = 1 : acols
            Adc((1:3) + (i-1)*3,(1:3) + (j-1)*3) = Sas*Ad1((1:3) + (i-1)*3,(1:3) + (j-1)*3)/Sas;
            %Adc((1:3) + (i-1)*3,(1:3) + (j-1)*3) = diag(diag(real(Adc((1:3) + (i-1)*3,(1:3) + (j-1)*3))));
        end
    end
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    %%%%%%%%%%%%%%%%%%%%%%% B --> Bpnz %%%%%%%%%%%%%%%%%%%%%%%

    Bdc = complex(zeros(3*brows,3*bcols));

    for i = 1 : brows
        for j = 1 : bcols
            Bdc((1:3) + (i-1)*3,(1:3) + (j-1)*3) = Sas*Bd1((1:3) + (i-1)*3,(1:3) + (j-1)*3)/Sas;
            %Bdc((1:3) + (i-1)*3,(1:3) + (j-1)*3) = diag(diag(real(Bdc((1:3) + (i-1)*3,(1:3) + (j-1)*3))));
        end
    end
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    %%%%%%%%%%%%%%%%%%%%%%% C --> Cpnz %%%%%%%%%%%%%%%%%%%%%%%

    Cdc = complex(zeros(3*crows,3*ccols));

    for i = 1 : crows
        for j = 1 : ccols
            Cdc((1:3) + (i-1)*3,(1:3) + (j-1)*3) = Sas*Cd1((1:3) + (i-1)*3,(1:3) + (j-1)*3)/Sas;
            %Cdc((1:3) + (i-1)*3,(1:3) + (j-1)*3) = diag(diag(real(Cdc((1:3) + (i-1)*3,(1:3) + (j-1)*3))));
        end
    end
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    %%%%%%%%%%%%%%%%%%%%%%% D1 --> Cpnz %%%%%%%%%%%%%%%%%%%%%%%

    Ddc1 = complex(zeros(3*d1rows,3*d1cols));

    for i = 1 : d1rows
        for j = 1 : d1cols
            Ddc1((1:3) + (i-1)*3,(1:3) + (j-1)*3) = Sas*Dd11((1:3) + (i-1)*3,(1:3) + (j-1)*3)/Sas;
            %Ddc1((1:3) + (i-1)*3,(1:3) + (j-1)*3) = diag(diag(real(Ddc1((1:3) + (i-1)*3,(1:3) + (j-1)*3))));
        end
    end
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    %%%%%%%%%%%%%%%%%%%%%%% D2 --> Cpnz %%%%%%%%%%%%%%%%%%%%%%%

    Ddc2 = complex(zeros(3*d2rows,3*d2cols));

    for i = 1 : d2rows
        for j = 1 : d2cols
            Ddc2((1:3) + (i-1)*3,(1:3) + (j-1)*3) = Sas*Dd12((1:3) + (i-1)*3,(1:3) + (j-1)*3)/Sas;
            %Ddc2((1:3) + (i-1)*3,(1:3) + (j-1)*3) = diag(diag(real(Ddc2((1:3) + (i-1)*3,(1:3) + (j-1)*3))));
        end
    end
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    % Adc(abs(Adc)<10*eps) = 0;
    % Bdc(abs(Bdc)<10*eps) = 0;
    % Cdc(abs(Cdc)<10*eps) = 0;
    % Ddc(abs(Ddc)<10*eps) = 0;

    InitVec_c = complex(zeros(brows*3,N+1));


    if(~isempty(InitVec))
        if(mod(numel(InitVec),3)~=0) error('[SS_DQNc_Mat] InitVec with wrong size --> needs to be a multiple of 3!'); end

        for k = 1 : numel(InitVec)/3
            InitVec_c((k-1)*3+1:(k-1)*3+3,1) = Sas*InitVec((k-1)*3+1:(k-1)*3+3);
        end
    end




% Parameter callback section
