function y = DSS(u, Ad, Bd, Cd, Dd, dt, nsw, InitVec, bvec)

% INIT

y = zeros(size(Cd,1),1);

persistent Ad11;
persistent Ad12;
persistent Ad21;
persistent Ad22;

persistent Bd11;
persistent Bd12;
persistent Bd21;
persistent Bd22;
persistent Cd11;
persistent Cd12;
persistent Cd21;
persistent Cd22;
persistent Dd11;
persistent Dd12;
persistent Dd21;
persistent Dd22;

persistent Xs_old;
persistent Xns_old;
persistent x_old;
persistent ys_old;
persistent bvec_old;
persistent us_old;
persistent tmr;

if(isempty(x_old))

    arows = size(Ad,1)/3; %signal size
    acols = size(Ad,2)/3; %signal size

    brows = size(Bd,1)/3; %signal size
    bcols = size(Bd,2)/3; %signal size

    crows = size(Cd,1)/3; %signal size
    ccols = size(Cd,2)/3; %signal size

    drows = size(Dd,1)/3; %signal size
    dcols = size(Dd,2)/3; %signal size

    Ad11 = Ad(1:nsw,1:nsw);        if(isempty(Ad11)) Ad11 = zeros(3,3); end
    Ad12 = Ad(1:nsw,nsw+1:end);    if(isempty(Ad12)) Ad12 = zeros(3,3); end
    Ad21 = Ad(nsw+1:end,1:nsw);    if(isempty(Ad21)) Ad21 = zeros(3,3); end
    Ad22 = Ad(nsw+1:end,nsw+1:end); if(isempty(Ad22)) Ad22 = zeros(3,3); end

    Bd11 = Bd(1:nsw,1:nsw);        if(isempty(Bd11)) Bd11 = zeros(3,3); end
    Bd12 = Bd(1:nsw,nsw+1:end);    if(isempty(Bd12)) Bd12 = zeros(3,3); end
    Bd21 = Bd(nsw+1:end,1:nsw);    if(isempty(Bd21)) Bd21 = zeros(3,3); end
    Bd22 = Bd(nsw+1:end,nsw+1:end); if(isempty(Bd22)) Bd22 = zeros(3,3*bcols-nsw); end

    Cd11 = Cd(1:nsw,1:nsw);        if(isempty(Cd11)) Cd11 = zeros(3,3); end
    Cd12 = Cd(1:nsw,nsw+1:end);    if(isempty(Cd12)) Cd12 = zeros(3,3); end
    Cd21 = Cd(nsw+1:end,1:nsw);    if(isempty(Cd21)) Cd21 = zeros(3,3); end
    Cd22 = Cd(nsw+1:end,nsw+1:end); if(isempty(Cd22)) Cd22 = zeros(3*crows-nsw,3*ccols); end

    Dd11 = Dd(1:nsw,1:nsw);        if(isempty(Dd11)) Dd11 = zeros(3,3); end
    Dd12 = Dd(1:nsw,nsw+1:end);    if(isempty(Dd12)) Dd12 = zeros(3,3); end
    Dd21 = Dd(nsw+1:end,1:nsw);    if(isempty(Dd21)) Dd21 = zeros(3,3); end
    Dd22 = Dd(nsw+1:end,nsw+1:end); if(isempty(Dd22)) Dd22 = zeros(3,3); end
   
    InitVecS = InitVec(1:nsw);
    
    InitVecNS = InitVec(nsw+1:end); if(isempty(InitVecNS)) InitVecNS = 0; end

    Xns_old = InitVecNS;
    Xs_old = InitVecS;

    x_old = [InitVecS; InitVecNS];
    ys_old = zeros(nsw,1);
    us_old = zeros(nsw,1);

    bvec_old = bvec;



    tmr = 0;
end
us = zeros(nsw,1);
%us = u(1:nsw);
uns = u(nsw+1:end);

if(any((bvec_old) ~= bvec))
    cngsw = 1;
    ys_old = us_old * 0.009;
else
    cngsw = 0;
end


if(bvec(1) == 0)
    us(1:3) = ys_old(1:3) / 1e6;
else
    
    us(1:3) = ys_old(1:3) / 0.01;
end

    x = Ad*x_old + [Bd11 Bd12; Bd21 Bd22]*[us; uns];
    y = Cd*x + [Dd11 Dd12; Dd21 Dd22]*[us; uns];




%OLD

x_old = x;
ys_old = y(1:nsw);
bvec_old = bvec;
tmr = tmr + 1;
us_old = us;


end



% Us = u(1:nsw);
% Uns = u(nsw+1:end);
% 
% 
% Xs = Ad11*Xs_old + Bd11*Us + Bd12*Uns + Ad12*Xns;
% Ys = Cd11*Xs_old + Cd12*Xns_old + Dd12*Uns + Dd11*Us;
% 
% Xns = Ad22*Xns_old + Bd21*Us + Bd22*Uns + Ad21*Xs;
% Yns = Cd22*Xns_old + Cd21*Xs_old + Dd21*Us + Dd22*Uns;

% Xs_old = Xs;
% Xns_old = Xns;

% Us = u(1:nsw);
% Uns = u(nsw+1:end);
