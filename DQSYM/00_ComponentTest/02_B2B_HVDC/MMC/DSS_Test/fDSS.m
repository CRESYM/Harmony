function [y] = fDSS(u, Ad, Bd, Cd, Dd, swOnRes, swOffRes, swType, ssim)

% INIT
x = zeros(size(Ad,1),1);
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

persistent Ads;
persistent Bds;
persistent Cds;
persistent Dds;

persistent x_old;
persistent swVec;
persistent swVecOld;

persistent nrSt;
persistent nrOut;

persistent nsw;
persistent ncb;
persistent npe;

persistent kb;


if(isempty(x_old))
    nrSt = size(Ad,1);
    nrOut = size(Cd,1);

    ncb = sum(swType==1) + sum(swType==2);
    npe = sum(swType==3);
    nsw = ncb + npe;
    
    Ad11 = Ad(1:nsw,1:nsw);         if(isempty(Ad11)) Ad11 = zeros(3,3); end
    Ad12 = Ad(1:nsw,nsw+1:end);     if(isempty(Ad12)) Ad12 = zeros(3,3); end
    Ad21 = Ad(nsw+1:end,1:nsw);     if(isempty(Ad21)) Ad21 = zeros(3,3); end
    Ad22 = Ad(nsw+1:end,nsw+1:end); if(isempty(Ad22)) Ad22 = zeros(3,3); end
    
    Bd11 = Bd(1:nsw,1:nsw);         if(isempty(Bd11)) Bd11 = zeros(3,3); end
    Bd12 = Bd(1:nsw,nsw+1:end);     if(isempty(Bd12)) Bd12 = zeros(3,3); end
    Bd21 = Bd(nsw+1:end,1:nsw);     if(isempty(Bd21)) Bd21 = zeros(3,3); end
    Bd22 = Bd(nsw+1:end,nsw+1:end); if(isempty(Bd22)) Bd22 = zeros(3,size(Bd,2)-nsw); end
    
    Cd11 = Cd(1:nsw,1:nsw);         if(isempty(Cd11)) Cd11 = zeros(3,3); end
    Cd12 = Cd(1:nsw,nsw+1:end);     if(isempty(Cd12)) Cd12 = zeros(3,3); end
    Cd21 = Cd(nsw+1:end,1:nsw);     if(isempty(Cd21)) Cd21 = zeros(3,3); end
    Cd22 = Cd(nsw+1:end,nsw+1:end); if(isempty(Cd22)) Cd22 = zeros(size(Cd,1)-nsw,size(Cd,2)); end
    
    Dd11 = Dd(1:nsw,1:nsw);         if(isempty(Dd11)) Dd11 = zeros(3,3); end
    Dd12 = Dd(1:nsw,nsw+1:end);     if(isempty(Dd12)) Dd12 = zeros(3,3); end
    Dd21 = Dd(nsw+1:end,1:nsw);     if(isempty(Dd21)) Dd21 = zeros(3,3); end
    Dd22 = Dd(nsw+1:end,nsw+1:end); if(isempty(Dd22)) Dd22 = zeros(3,3); end
    
    x_old = zeros(size(Ad,1),1);

    swVec = [ssim zeros(1, nsw - ncb) ]; 
    swVecOld = zeros(nsw,1); 

    kb = swOffRes.*abs(1-swVec)+swOnRes.*abs(0-swVec);

    Dd11k = zeros(size(Dd11)); for i = 1 : nsw, Dd11k(:,i) = Dd11(:,i)/kb(i); end
    D11Ik = eye(nsw)/(eye(nsw) - Dd11k);

    Dd21k = zeros(size(Dd21)); for i = 1 : nsw, Dd21k(:,i) = Dd21(:,i)/kb(i);  end
    
    E = D11Ik * Cd11;
    F = D11Ik * Cd12;
    I = D11Ik*Dd12;
    
    Bd11k = zeros(size(Bd11)); for i = 1 : nsw, Bd11k(:,i) = Bd11(:,i)/kb(i);  end
    Bd21k = zeros(size(Bd21)); for i = 1 : nsw, Bd21k(:,i) = Bd21(:,i)/kb(i);  end

    Ads = [Ad11+Bd11k*E Ad12+Bd11k*F ; Ad21+Bd21k*E Ad22+Bd21k*F ];
    Bds = [(Bd11k*I + Bd12); Bd21k*I + Bd22];
    Cds = [ D11Ik * Cd11 D11Ik * Cd12 ; Dd21k*D11Ik*Cd11 + Cd21 Dd21k*D11Ik*Cd12 + Cd22];
    Dds = [ D11Ik*Dd12; (Dd22+Dd21k*D11Ik*Dd12)];

end

% is = zeros(nsw,1);

swVec(1:ncb) = ssim(1:ncb);

if(any(swVecOld ~= swVec))

    kb = swOffRes.*abs(1-swVec)+swOnRes.*abs(0-swVec);
    Dd11k = zeros(size(Dd11)); for i = 1 : nsw, Dd11k(:,i) = Dd11(:,i)/kb(i); end
    D11Ik = eye(nsw)/(eye(nsw) - Dd11k);

    Dd21k = zeros(size(Dd21)); for i = 1 : nsw, Dd21k(:,i) = Dd21(:,i)/kb(i);  end

    E = D11Ik * Cd11;
    F = D11Ik * Cd12;
    I = D11Ik * Dd12;

    Bd11k = zeros(size(Bd11)); for i = 1 : nsw, Bd11k(:,i) = Bd11(:,i)/kb(i);  end
    Bd21k = zeros(size(Bd21)); for i = 1 : nsw, Bd21k(:,i) = Bd21(:,i)/kb(i);  end

    Ads = [Ad11+Bd11k*E Ad12+Bd11k*F ; Ad21+Bd21k*E Ad22+Bd21k*F ];
    Bds = [(Bd11k*I + Bd12); Bd21k*I + Bd22];
    Cds = [ D11Ik * Cd11 D11Ik * Cd12 ; Dd21k*D11Ik*Cd11 + Cd21 Dd21k*D11Ik*Cd12 + Cd22];
    Dds = [ D11Ik*Dd12; (Dd22+Dd21k*D11Ik*Dd12)];
end

swVecOld = swVec;


    % x           = Ads*x_old  + Bds*u(nsw+1:end);  x_old(1:nrSt) = x(1:nrSt);                                                   
    % y(1:nrOut)  = Cds*x      + Dds*u(nsw+1:end);   

    x           = Ads*x_old  + Bds*u;  x_old(1:nrSt) = x(1:nrSt);                                                   
    y(1:nrOut)  = Cds*x      + Dds*u;     

    
    is = y(1:nsw)./kb';


    for i = ncb:nsw

        if(i > ncb) %Breakers

            if(swVec(i)==1 && (is(i) <= eps))
                swVec(i) = 0; %Open
            end

            if(swVec(i)==0 && y(i) > 0)
                swVec(i) = 1; %Close
            end
        end

    end

    

    %OLD
    
    
    % if(t>1 && any(swVec(t-1,:) ~= swVec(t,:)))
    % 
    %     kb = swOffRes.*abs(1-ssim(t,:))+swOnRes.*abs(0-ssim(t,:));
    %     Dd11k = zeros(size(Dd11)); for i = 1 : nsw, Dd11k(:,i) = Dd11(:,i)/kb(i); end
    %     D11Ik = eye(nsw)/(eye(nsw) - Dd11k);
    % 
    %     Dd21k = zeros(size(Dd21)); for i = 1 : nsw, Dd21k(:,i) = Dd21(:,i)/kb(i);  end
    % 
    %     E = D11Ik * Cd11;
    %     F = D11Ik * Cd12;
    %     I = D11Ik * Dd12;
    % 
    %     Bd11k = zeros(size(Bd11)); for i = 1 : nsw, Bd11k(:,i) = Bd11(:,i)/kb(i);  end
    %     Bd21k = zeros(size(Bd21)); for i = 1 : nsw, Bd21k(:,i) = Bd21(:,i)/kb(i);  end
    % 
    %     Ads = [Ad11+Bd11k*E Ad12+Bd11k*F ; Ad21+Bd21k*E Ad22+Bd21k*F ];
    %     Bds = [(Bd11k*I + Bd12); Bd21k*I + Bd22];
    %     Cds = [ D11Ik * Cd11 D11Ik * Cd12 ; Dd21k*D11Ik*Cd11 + Cd21 Dd21k*D11Ik*Cd12 + Cd22];
    %     Dds = [ D11Ik*Dd12; (Dd22+Dd21k*D11Ik*Dd12)];
    % end
    % 
    % 
    % 
    % x           = Ads*x_old  + Bds*u(nsw+1:end);                                                     
    % y(1:nrOut)  = Cds*x      + Dds*u(nsw+1:end);   
    % 
    % 
    % 
    % for i = 1:nsw
    % 
    %     if(i<= ncb)
    % 
    %     else
    %         if(ssim(t,i)==1)
    %             id(t,i) = ysim(t,i)/kb(i);
    %         end    
    % 
    %         if(swVec(t,i)==1 && (id(t,i) <= eps))
    %             swVec(t,i) = 0; %Open
    %         end
    % 
    %         if(swVec(t,i)==0 && ysim(t,i) > 0)
    %             swVec(t,i) = 1; %Close
    %         end
    %     end
    % 
    % end



end




