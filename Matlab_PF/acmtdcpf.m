function [] = acmtdcpf (acFileName, mtdcFileName) 

%% 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% acmtdcpf can be used to do AC/MTDDC PF calcuation by solving OPF %%%%%%
%%% model. Yalmip is for OPF formuation and Ipopt is the solver %%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%  References:
%  [1] J. Beerten, S. Cole and R. Belmans, "Generalized Steady-State VSC MTDC Model 
%  for Sequential AC/DC Power Flow Algorithms," in IEEE Transactions on Power Systems, 
%  vol. 27, no. 2, pp. 821-829, May 2012, doi: 10.1109/TPWRS.2011.2177867.
%  [2]  Mauro Escobar, https://github.com/me2533/acopf
%  [3]  Matacdc1.0 unser's manual, https://www.esat.kuleuven.be/electa/teaching/matacdc/MatACDCManual

%  h.li199305ee@gmail.com


%%  " Adding DC Grid and Converter Parameters "
tic;

[filepath,name,ext] = fileparts(mtdcFileName);
  
  if ~strcmp(ext,'.m')
    error('Extension of the case filename must be .m');
  end
  currFld = pwd;
  if ~isempty(filepath)
    cd(filepath);
  end
  mpc = feval(name);
  cd(currFld);
  
  if ~isfield(mpc,'conv')
    error('VSC converter parameters need to be added')
  end

  mpc.casename = name;
  mpc.bus = mpc.bus(mpc.bus(:,2)<4,:);
  mpc.branch = mpc.branch(mpc.branch(:,11)==1,:);
  
  baseMVA = mpc.baseMVA;
  basekVac = mpc.conv(:,14);

  bus_dc = mpc.bus; % dc bus parameters
  branch_dc = mpc.branch; % dc branch parameters
  conv = mpc.conv; % converter parameters
  pol = mpc.pol; % numbers of poles

  nbuses_dc = size(bus_dc,1);
  nbranches_dc = size(branch_dc,1);
  nconvs = size(conv,1);

  % dc network parameters 
  [y_dc, ~, ~] = makeYbus(baseMVA, bus_dc, branch_dc);
   y_dc = abs(full(y_dc));
 
  % converter parameters 
  rtf = conv(:,9);  % transformer resistance
  xtf = conv(:,10); % transformer reactance
  bf = conv(:,11); % filter susceptance
  rec = conv(:,12); % phase reactor resistance
  xc = conv(:,13); % phase reactor reactance
  ztfc = complex(rtf+rec, xtf+xc);
  gtfc = real(1./ztfc);
  btfc = imag(1./ztfc);

  % quaratic converter loss 
  aloss =zeros(nconvs, 1); 
  bloss =zeros(nconvs, 1);
  closs =zeros(nconvs, 1);

  % state(inverter/rectifier) of converter
  convState = zeros(nconvs, 1); 
   for i = 1:nconvs
       % p-control
       if conv(i,4) == 1  %  
           if conv(i,6) >=0 % inverter state 
               closs(i) = conv(i,22);
               convState(i) = 1;
           else
               closs(i) = conv(i,21);  % rectifier state 
               convState(i) = 0;
           end
       end
       % v-control
       if conv(i,4) == 2  %
           if sum(conv(:,6)) <=0 % inverter state 
               closs(i) = conv(i,22);
               convState(i) = 1;
           else
               closs(i) = conv(i,21); % rectifier state 
               convState(i) = 0;
           end
       end
       % droop control
       if conv(i,4) == 3 %
           if conv(i,6) >=0 % inverter state 
               closs(i) = conv(i,22);
               convState(i) = 1;
           else
               closs(i) = conv(i,21);  % rectifier state 
               convState(i) = 0;
           end
       end
   end
   closs = closs./(basekVac.^2/baseMVA);
   aloss = conv(:,19)/baseMVA;
   bloss = conv(:,20)./basekVac;

%%  " Defining DC Grid and Coverter Primal Variables and Boundaries "

% [vn_dc, pn_dc, ps, qs, pc, qc, vs, vf, vc, as, af, ac, Ic];
% 1. vn_dc: dc voltage 
% 2. pn_dc: dc power injection 
% 3. ps：active power injection at node s of converter ac side 
% 4. qs：reactive power injection at node s of converter ac side 
% 5. pc：active power injection at node c of converter ac side 
% 6. qc：reactive power injection at node c of converter ac side 
% 7. vs： nodal voltage amplitude at node s of converter ac side 
% 8. vf： nodal voltage amplitude at node f of converter ac side 
% 9. vc： nodal voltage amplitude at node c of converter ac side 
% 10. as： nodal voltage angle at node s of converter ac side
% 11. af： nodal voltage angle at node f of converter ac side 
% 12. ac： nodal voltage angle at node c of converter ac side 
% 13. Ic: converter current 
var_dc = sdpvar (2*nbuses_dc+11*nconvs,1); 
lb_dc = -inf(2*nbuses_dc+11*nconvs,1);
ub_dc = inf(2*nbuses_dc+11*nconvs,1);
% 1.
lb_dc(1:nbuses_dc) = bus_dc(:,13); 
ub_dc(1:nbuses_dc) = bus_dc(:,12);
% 2 
lb_dc(nbuses_dc+(1:nbuses_dc)) = -inf; 
ub_dc(nbuses_dc+(1:nbuses_dc)) = inf;
% 3 
lb_dc(2*nbuses_dc+(1:nconvs)) = -inf;
ub_dc(2*nbuses_dc+(1:nconvs)) = inf;
% 4
lb_dc(2*nbuses_dc+nconvs+(1:nconvs)) = -inf;
ub_dc(2*nbuses_dc+nconvs+(1:nconvs)) = inf;
% 5 
lb_dc(2*nbuses_dc+2*nconvs+(1:nconvs)) = -inf;
ub_dc(2*nbuses_dc+2*nconvs+(1:nconvs)) = inf; 
% 6 
lb_dc(2*nbuses_dc+3*nconvs+(1:nconvs)) = -inf;
ub_dc(2*nbuses_dc+3*nconvs+(1:nconvs)) = inf; 
% 7 
lb_dc(2*nbuses_dc+4*nconvs+(1:nconvs)) = conv(:,16);
ub_dc(2*nbuses_dc+4*nconvs+(1:nconvs)) = conv(:,15);
% 8 
lb_dc(2*nbuses_dc+5*nconvs+(1:nconvs)) = conv(:,16);
ub_dc(2*nbuses_dc+5*nconvs+(1:nconvs)) = conv(:,15);
% 9 
lb_dc(2*nbuses_dc+6*nconvs+(1:nconvs)) = conv(:,16);
ub_dc(2*nbuses_dc+6*nconvs+(1:nconvs)) = conv(:,15);
% 10 
lb_dc(2*nbuses_dc+7*nconvs+(1:nconvs)) = -pi;
ub_dc(2*nbuses_dc+7*nconvs+(1:nconvs)) = pi;
% 11 
lb_dc(2*nbuses_dc+8*nconvs+(1:nconvs)) = -pi;
ub_dc(2*nbuses_dc+8*nconvs+(1:nconvs)) = pi;
% 12 
lb_dc(2*nbuses_dc+9*nconvs+(1:nconvs)) = -pi;
ub_dc(2*nbuses_dc+9*nconvs+(1:nconvs)) = pi;
% 13
lb_dc(2*nbuses_dc+10*nconvs+(1:nconvs)) = 0;
ub_dc(2*nbuses_dc+10*nconvs+(1:nconvs)) = conv(:,17);

vn_dc = var_dc(1:nbuses_dc, 1); 
pn_dc = var_dc(nbuses_dc+(1:nbuses_dc), 1);
ps = var_dc(2*nbuses_dc+(1:nconvs), 1);
qs = var_dc(2*nbuses_dc+1*nconvs+(1:nconvs), 1);
pc = var_dc(2*nbuses_dc+2*nconvs+(1:nconvs), 1);
qc = var_dc(2*nbuses_dc+3*nconvs+(1:nconvs), 1);
vs = var_dc(2*nbuses_dc+4*nconvs+(1:nconvs), 1);
vf = var_dc(2*nbuses_dc+5*nconvs+(1:nconvs), 1);
vc = var_dc(2*nbuses_dc+6*nconvs+(1:nconvs), 1);
as = var_dc(2*nbuses_dc+7*nconvs+(1:nconvs), 1);
af = var_dc(2*nbuses_dc+8*nconvs+(1:nconvs), 1);
ac = var_dc(2*nbuses_dc+9*nconvs+(1:nconvs), 1);
Ic = var_dc(2*nbuses_dc+10*nconvs+(1:nconvs), 1);
convPloss = sdpvar(nconvs, 1);

con_dc = [];
con_dc = [con_dc; lb_dc <= var_dc <= ub_dc]; 

%%  " Adding DC Grid and Converter Constraints " 
for i= 1:nbuses_dc 
    con_dc = [con_dc; pn_dc(i) == pol*vn_dc(i)*sum(y_dc(i,:)'.*(vn_dc(i)*ones(nbuses_dc,1)-vn_dc))];
end  % constraint for dc power flow

for i = 1:nconvs
    con_dc=[con_dc; ps(i) == -vs(i)*vc(i)*(gtfc(i)*cos(as(i)-ac(i))+btfc(i)*sin(as(i)-ac(i))) + vs(i)^2*gtfc(i) ];
    con_dc=[con_dc; qs(i) == -vs(i)*vc(i)*(gtfc(i)*sin(as(i)-ac(i))-btfc(i)*cos(as(i)-ac(i))) - vs(i)^2*btfc(i) ];
    con_dc=[con_dc; pc(i) == -vc(i)*vs(i)*(gtfc(i)*cos(ac(i)-as(i))+btfc(i)*sin(ac(i)-as(i))) + vc(i)^2*gtfc(i) ];
    con_dc=[con_dc; qc(i) == -vc(i)*vs(i)*(gtfc(i)*sin(ac(i)-as(i))-btfc(i)*cos(ac(i)-as(i))) - vc(i)^2*btfc(i) ];
end % constraint for power flow at converter ac side

for i = 1:nconvs
    IDdc = i;
    con_dc = [con_dc; pc(i)+pn_dc(i)+convPloss(i) == 0];
    con_dc = [con_dc; 0<= convPloss(i) <=1];
    con_dc = [con_dc; convPloss(i) == aloss(i)+bloss(i)*Ic(i)+closs(i)*Ic(i)^2];
    con_dc = [con_dc; (Ic(i)*vc(i))^2 == pc(i)^2+qc(i)^2];
end % constraint for power loss inside converter

% constraint for VSC control model
for i = 1:nconvs   
    % dc side control mode
    if conv(i,4) == 1   % p control
        con_dc = [con_dc; ps(i) == -conv(i,6)/100];
    elseif conv(i,4) == 2 % dc v control
            con_dc = [con_dc; vn_dc(i) == conv(i,8)];
    else % droop control 
        con_dc = [con_dc; pn_dc(i) == (conv(i,24)-1/conv(i,23)*(vn_dc(i)-conv(i,25)))/baseMVA*(-1)];
    end
   % ac side control mode
    if conv(i,5) == 1   % q control
        con_dc = [con_dc; qs(i) == -conv(i,7)/100];
    else % ac v control
        con_dc = [con_dc; vs(i) == conv(i,8)];
    end
   % inverter or rectifier mode 
   if convState(i) == 0 % rectifier 
       con_dc = [con_dc; ps(i)>=0; pn_dc(i)>=0; pc(i)<=0];
   else % inverter 
       con_dc = [con_dc; ps(i)<=0; pn_dc(i)<=0; pc(i)>=0];
   end
end 


%%  " Adding AC Grid " 

[filepath,name,ext] = fileparts(acFileName);
  if ~strcmp(ext,'.m')
    error('Extension of the case filename must be .m');
  end
  currFld = pwd;
  if ~isempty(filepath)
    cd(filepath);
  end
  mpc = feval(name);
  cd(currFld);
  
  if max(mpc.gencost(:,1))>2 || min(mpc.gencost(:,1))<2
    error('Only generation cost functions of type 2 are supported')
  end

  if max(mpc.gencost(:,4))>min(mpc.gencost(:,4))
    error('Generation cost functions of type 2 are supportedmust be of the same degree (2 or 3) for all generators')
  end

  mpc.casename = name;
  mpc.bus = mpc.bus(mpc.bus(:,2)<4,:);
  mpc.branch = mpc.branch(mpc.branch(:,11)==1,:);
  
  baseMVA = mpc.baseMVA;
  ngrids = size(unique(mpc.bus(:,14)),1);

%%  " Defining AC Grid Primal Variables " 
 % [vm_ac, va_ac, pn_ac, qn_ac, pij_ac, qij_ac, pgen, qgen, theta_diff, vn_product_] 
 % 1 nodal voltage amplitude 
 % 2 nodal voltage phase 
 % 3 nodal active power injection 
 % 4 nodal reactive power injection 
 % 5 active power flow used for calculate active power injection
 % 6 rective power flow used for calculate active power injection
 % 7 generator active power 
 % 8 generator reactive power 
 % 9 node voltage difference 
 % 10 node voltage production


 % multiple ac grids
 var_ac = cell(ngrids,1);
 vm_ac =  cell(ngrids,1);
 va_ac = cell(ngrids,1);
 pn_ac = cell(ngrids,1);
 qn_ac = cell(ngrids,1);
 pgen = cell(ngrids,1);
 qgen = cell(ngrids,1);

%  PP = cell(ngrids,1);
%  QQ = cell(ngrids,1);
%  PPc = cell(ngrids,1);
%  QQc = cell(ngrids,1);

 con_acdc = cell(ngrids,1);
 obj_acdc = cell(ngrids,1);

 bus_ac = cell(ngrids,1);
 branch_ac = cell(ngrids,1);
 gen_ac = cell(ngrids,1);
 pd_ac = cell(ngrids,1);
 qd_dc = cell(ngrids,1);

 va_vac_mat = cell(ngrids, 1);
 va_ac_mat_t = cell(ngrids, 1);
 theta_diff = cell(ngrids, 1);
 vn_product = cell(ngrids, 1);
 pij_ac = cell(ngrids, 1);
 qij_ac = cell(ngrids, 1);

 recBuses = [];
 recRef = [];

%% "Loop Start: from AC Grid #1~#n " 
for ng = 1:ngrids

%%  " Defining AC Grid Parameters " 
bus_ac{ng} = mpc.bus(find(mpc.bus(:,14) == ng),:);
branch_ac{ng} =  mpc.branch(find(mpc.branch(:,14) == ng),:);
gen{ng} = mpc.gen(find(mpc.gen(:,22) == ng),:);
gencost = mpc.gencost(find(mpc.gencost(:,8) == ng),:);

nbuses_ac = size(bus_ac{ng},1);
nbranches_ac = size(branch_ac{ng},1);
recBuses = [recBuses; nbuses_ac];

IDtoCountmap = zeros(nbuses_ac,1);
for i=1:nbuses_ac
      IDtoCountmap(bus_ac{ng}(i,1)) = i;
    if bus_ac{ng}(i,2)==3
      refbuscount = i;
    end
end % standard order for AC bus
recRef =[recRef; refbuscount];

ngens = size(gen{ng},1);
genids = cell(nbuses_ac,1);
for i=1:ngens
    if gen{ng}(i,8)==0; continue; end
    genids{IDtoCountmap(gen{ng}(i,1))} = [genids{IDtoCountmap(gen{ng}(i,1))} i];
end % ID of AC bus connected to generators

convids = cell (nbuses_ac, 1);
for i = 1:nconvs
    if conv(i,3) == ng
    convids{conv(i,2)} = [convids{conv(i,2)}; i conv(i,3)];
    end
end % ID of ac grid bus connected to converters

% frombranchids = cell(nbuses_ac,1);
% tobranchids = cell(nbuses_ac,1);
anglelim_rad = zeros(nbranches_ac,2);
for i=1:nbranches_ac
  minangle = branch_ac{ng}(i,12);
  maxangle = branch_ac{ng}(i,13);
if (minangle==0 && maxangle==0) || (minangle==-360 && maxangle==360)
  minangle = -180; maxangle = 180;
end
  anglelim_rad(i,1) = pi*minangle/180;
  anglelim_rad(i,2) = pi*maxangle/180;
%   frombranchids{IDtoCountmap(branch_ac{ng}(i,1))} = [frombranchids{IDtoCountmap(branch_ac{ng}(i,1))} i];
%   tobranchids{IDtoCountmap(branch_ac{ng}(i,2))} = [tobranchids{IDtoCountmap(branch_ac{ng}(i,2))} i];
end  % save ID of "frombus" and "tobus" for each branch

[YY, ~, ~] = makeYbus(baseMVA, bus_ac{ng}, branch_ac{ng});
GG = real(full(YY)); 
BB = imag(full(YY));

pd{ng} = bus_ac{ng}(:,3)/baseMVA;
qd{ng} = bus_ac{ng}(:,4)/baseMVA; 

%%  " Defining AC Grid Primal Variables and Boundaries "
lb_ac = -inf(4*nbuses_ac+2*ngens,1);
ub_ac = inf(4*nbuses_ac+2*ngens,1);

% 1
lb_ac(1:nbuses_ac) = bus_ac{ng}(:,13); % lb for nodal voltage amplitude
ub_ac(1:nbuses_ac) = bus_ac{ng}(:,12); % ub for nodal voltage amplitude
% nodal voltage amplitude for refbus is determined
lb_ac(0+refbuscount) = gen{ng}(find(gen{ng}(:,1))==refbuscount, 6);
ub_ac(0+refbuscount) = gen{ng}(find(gen{ng}(:,1))==refbuscount, 6);

% 2
k = nbuses_ac; 
lb_ac(k+(1:nbuses_ac)) = -pi; % lb for nodal phase
ub_ac(k+(1:nbuses_ac)) = pi; % ub for nodal phase
% nodal voltage phase for refbus is determined
lb_ac(k+refbuscount) = pi*bus_ac{ng}(refbuscount,9)/180;  
ub_ac(k+refbuscount) = pi*bus_ac{ng}(refbuscount,9)/180; 

% 3
k = 2*nbuses_ac;
lb_ac(k+(1:nbuses_ac)) = -inf; % lb of nodal active power injection
ub_ac(k+(1:nbuses_ac)) = inf; % ub of nodal active power injection

% 4
k = 3*nbuses_ac;
lb_ac(k+(1:nbuses_ac)) = -inf; % lb of nodal reactive power injection
ub_ac(k+(1:nbuses_ac)) = inf; % ub of nodal reactive power injection

% 5
k = 4*nbuses_ac;
lb_ac(k+(1:ngens)) = gen{ng}(:,10).*gen{ng}(:,8)/baseMVA; % lb of generator active power output
ub_ac(k+(1:ngens)) = gen{ng}(:,9).*gen{ng}(:,8)/baseMVA; % ub of generator active power output

% 6
k = 4*nbuses_ac+ngens;
lb_ac(k+(1:ngens)) = gen{ng}(:,5).*gen{ng}(:,8)/baseMVA; % lb of generator reactive power output
ub_ac(k+(1:ngens)) = gen{ng}(:,4).*gen{ng}(:,8)/baseMVA; % ub of generator reactive power output

var_ac{ng} = sdpvar(4*nbuses_ac+2*ngens,1); 
vm_ac{ng} = var_ac{ng}(1:nbuses_ac, 1);
va_ac{ng} = var_ac{ng}(nbuses_ac+(1:nbuses_ac), 1);
pn_ac{ng} = var_ac{ng}(2*nbuses_ac+(1:nbuses_ac), 1);
qn_ac{ng} = var_ac{ng}(3*nbuses_ac+(1:nbuses_ac), 1);
pgen{ng} = var_ac{ng}(4*nbuses_ac+(1:ngens), 1);
qgen{ng} = var_ac{ng}(4*nbuses_ac+ngens+(1:ngens), 1);

con_acdc{ng} = [];
con_acdc{ng} = [con_acdc{ng}; lb_ac<= var_ac{ng} <=ub_ac]; 

%%  " Building AC Grid Constraints "
% PP{ng} = sdpvar(nbuses_ac, 1);
% QQ{ng} = sdpvar(nbuses_ac, 1);
% % Pij = sdpvar(nbuses_ac, nbuses_ac, 'full');
% % Qij = sdpvar(nbuses_ac, nbuses_ac, 'full');
% i=1;
% for j = 1:nbuses_ac
%     con_acdc{ng} = [con_acdc{ng}; PP{ng}(j) == vm_ac{ng}(i)*vm_ac{ng}(j)*(GG(i,j)*cos(va_ac{ng}(i)-va_ac{ng}(j))+BB(i,j)*sin(va_ac{ng}(i)-va_ac{ng}(j)))];
%     con_acdc{ng} = [con_acdc{ng}; QQ{ng}(j) == vm_ac{ng}(i)*vm_ac{ng}(j)*(GG(i,j)*sin(va_ac{ng}(i)-va_ac{ng}(j))-BB(i,j)*cos(va_ac{ng}(i)-va_ac{ng}(j)))];
% %         if i~=j 
% %            con_acdc = [con_acdc; Pij(i,j) == (vm_ac(i)^2-vm_ac(i)*vm_ac(j)*cos(va_ac(i)-va_ac(j)))*(-1)*GG(i,j)-vm_ac(i)*vm_ac(j)*(-1)*BB(i,j)*sin(va_ac(i)-va_ac(j))];
% %            con_acdc = [con_acdc; Qij(i,j) == -(vm_ac(i)^2-vm_ac(i)*vm_ac(j)*cos(va_ac(i)-va_ac(j)))*(-1)*BB(i,j)-vm_ac(i)*vm_ac(j)*(-1)*GG(i,j)*sin(va_ac(i)-va_ac(j))];
% %         end 
% end
% con_acdc{ng} = [con_acdc{ng}; pn_ac{ng}(i) == sum(PP{ng})];
% con_acdc{ng} = [con_acdc{ng}; qn_ac{ng}(i) == sum(QQ{ng})]; % constraints for ac power flow part 1
% 
% PPc{ng} = cell(nbuses_ac, 1);
% QQc{ng} = cell(nbuses_ac, 1);
% for i = 2: nbuses_ac  
%       j = []; a=[]; b=[];
%       j = [j; i];
%     if length(frombranchids{i})
%           k = frombranchids{i}; 
%           j = [j; branch_ac{ng}(k,2)];
%     end     
%     if length(tobranchids{i})
%           kk = tobranchids{i};
%           j = [j; branch_ac{ng}(kk,1)];
%     end
%        PPc{ng}{i} = sdpvar(length(j),1); 
%        QQc{ng}{i} = sdpvar(length(j),1); 
% 
%        con_acdc{ng} = [con_acdc{ng}; PPc{ng}{i} == vm_ac{ng}(i)*ones(length(j), 1).*vm_ac{ng}(j).*(GG(i,j)'.*cos(va_ac{ng}(i)-va_ac{ng}(j))+BB(i,j)'.*sin(va_ac{ng}(i)-va_ac{ng}(j)))];
%        con_acdc{ng} = [con_acdc{ng}; QQc{ng}{i} == vm_ac{ng}(i)*ones(length(j), 1).*vm_ac{ng}(j).*(GG(i,j)'.*sin(va_ac{ng}(i)-va_ac{ng}(j))-BB(i,j)'.*cos(va_ac{ng}(i)-va_ac{ng}(j)))];
%        con_acdc{ng} = [con_acdc{ng}; pn_ac{ng}(i) == sum(PPc{ng}{i})];
%        con_acdc{ng} = [con_acdc{ng}; qn_ac{ng}(i) == sum(QQc{ng}{i})]; % constraints for power flow part 2
% end

va_ac_mat{ng} = repmat(va_ac{ng}, 1, nbuses_ac);
va_ac_mat_t{ng} = repmat(va_ac{ng}.', nbuses_ac, 1);
theta_diff{ng} = va_ac_mat{ng} - va_ac_mat_t{ng}; 
vn_product{ng} = vm_ac{ng}*vm_ac{ng}.';
pij_ac{ng} = vn_product{ng} .* (GG .* cos(theta_diff{ng}) + BB .* sin(theta_diff{ng}));
qij_ac{ng} = vn_product{ng} .* (GG .* sin(theta_diff{ng}) - BB .* cos(theta_diff{ng}));
con_acdc{ng} = [con_acdc{ng}; pn_ac{ng} == sum(pij_ac{ng}, 2)];
con_acdc{ng} = [con_acdc{ng}; qn_ac{ng} == sum(qij_ac{ng}, 2)];


%%  " Building AC/DC Coupling Constraints "
for i = 1:nbuses_ac
     if length(genids{i}) && length(convids{i})
        genindex = genids{i};
        convindex = convids{i}(1); 
        if convids{i}(2) == ng
           con_acdc{ng} = [con_acdc{ng}; pn_ac{ng}(i) == sum(pgen{ng}(genindex))-pd{ng}(i)-ps(convindex)];
           con_acdc{ng} = [con_acdc{ng}; qn_ac{ng}(i) == sum(qgen{ng}(genindex))-qd{ng}(i)-qs(convindex)];
        else
           con_acdc{ng} = [con_acdc{ng}; pn_ac{ng}(i) == sum(pgen{ng}(genindex))-pd{ng}(i)];
           con_acdc{ng} = [con_acdc{ng}; qn_ac{ng}(i) == sum(qgen{ng}(genindex))-qd{ng}(i)];
        end
         % ac bus with generator and converter
     elseif length(genids{i})
            genindex = genids{i};
            con_acdc{ng} = [con_acdc{ng}; pn_ac{ng}(i) == sum(pgen{ng}(genindex))-pd{ng}(i)];
            con_acdc{ng} = [con_acdc{ng}; qn_ac{ng}(i) == sum(qgen{ng}(genindex))-qd{ng}(i)];
         % ac bus with generator
     elseif length(convids{i}) 
            convindex = convids{i}(1); 
            if convids{i}(2) == ng
               con_acdc{ng} = [con_acdc{ng}; pn_ac{ng}(i) == -pd{ng}(i)-ps(convindex)];
               con_acdc{ng} = [con_acdc{ng}; qn_ac{ng}(i) == -qd{ng}(i)-qs(convindex)];
            else
               con_acdc{ng} = [con_acdc{ng}; pn_ac{ng}(i) == -pd{ng}(i)];
               con_acdc{ng} = [con_acdc{ng}; qn_ac{ng}(i) == -qd{ng}(i)];
            end
         % ac bus with converter
     else
            con_acdc{ng} = [con_acdc{ng}; pn_ac{ng}(i) == -pd{ng}(i)];
            con_acdc{ng} = [con_acdc{ng}; qn_ac{ng}(i) == -qd{ng}(i)];
     end
end  % ac bus only with load

for i = 1:nbuses_ac
    if length(convids{i})
        if convids{i}(2) == ng
            con_acdc{ng} = [con_acdc{ng}; vm_ac{ng}(i) == vs(convids{i}(1))];
            con_acdc{ng} = [con_acdc{ng}; va_ac{ng}(i) == as(convids{i}(1))];
        end
    end
end

actgen = gen{ng}(:,8);
if gencost(1,4) == 3
    obj_acdc{ng} = sum(actgen.*( baseMVA^2*gencost(:,5).*pgen{ng}.^2 + baseMVA*gencost(:,6).*pgen{ng} + gencost(:,7) ));
elseif gencost(1,4)==2
    obj_acdc{ng} = sum(actgen.*( baseMVA*gencost(:,5).*pgen{ng} + gencost(:,6) ));
end % objective

end

cputime = toc; 

Con = []; Obj = 0;
for ng=1:ngrids
    Con = [Con; con_acdc{ng}];
    Obj = Obj+obj_acdc{ng};
end 

Con = [Con; con_dc];

Opt = sdpsettings('solver', 'ipopt', 'ipopt.mu_strategy','adaptive',  'ipopt.tol', 1e-5,  ...
'ipopt.dual_inf_tol', 1e-1, 'ipopt.compl_inf_tol', 1e-5, 'ipopt.acceptable_compl_inf_tol', 1e-3);
opfOut = optimize(Con, Obj, Opt);


%%  " Showing Power Flow Results "
fid = fopen('TestNonlin.txt', 'w');
fid = 1;
fprintf(fid,'\n================================================================================');
fprintf(fid,'\n|   AC Bus Power Flow Results                                                  |');
    fprintf(fid,'\n================================================================================');
    fprintf(fid,'\n Bus      Area      Voltage          Generation             Load        ');
    fprintf(fid,'\n #        #    Mag [pu] Ang [deg]   P [MW]   Q [MVAr]   P [MW]  Q [MVAr]');
    fprintf(fid,'\n-----   -----  --------  --------  --------  --------  -------- --------');

    for ng = 1:ngrids
        genindex = gen{ng}(:,1);

        for i = 1:recBuses(ng)
            fprintf(fid,'\n %3d %7d %8.3f %9.3f', i, ng, value(vm_ac{ng}(i)), value(va_ac{ng}(i))/pi*180);

            if i == recRef(ng)
               fprintf(fid, '*')
            end

            if ismember(i,genindex)
               m = gen{ng}(:,1);
               if i == refbuscount
                  fprintf(fid,'%9.2f %9.2f', value(pgen{ng}(find(m== i), 1))*baseMVA, value(qgen{ng}(find(m== i), 1))*baseMVA );
               else
                   fprintf(fid,'%10.2f %9.2f', value(pgen{ng}(find(m== i), 1))*baseMVA, value(qgen{ng}(find(m== i), 1))*baseMVA );
               end
                fprintf(fid,'%10.2f %8.2f', value(pd{ng}(i)*baseMVA), value(qd{ng}(i)*baseMVA) );
            else
                fprintf(fid, '       -         -');
                fprintf(fid,'%12.2f %8.2f', value(pd{ng}(i)*baseMVA), value(qd{ng}(i)*baseMVA) );
            end

        end
    end

    GenCostRes = 0;
    for ng=1:ngrids
        GenCostRes = GenCostRes+value(obj_acdc{ng});
    end 
 fprintf(fid,'\n-----   ----- --------  --------  --------  --------  -------- --------');
 fprintf(fid,'\n The total generation costs is ＄%.2f(€%.2f)',GenCostRes, GenCostRes/1.08);
 fprintf(fid,'\n');
 fprintf(fid,'\n');



fprintf(fid, '\n================================================================================');
fprintf(fid, '\n|     DC Bus Power Flow Results                                                |');
fprintf(fid, '\n================================================================================');
fprintf(fid, '\n Bus   Bus    AC   DC Voltage   DC Power   PCC Bus Injection   Converter loss');
fprintf(fid, '\n DC #  AC #  Area   Vdc [pu]    Pdc [MW]   Ps [MW]  Qs [MVAr]  Conv_Ploss [MW]');
fprintf(fid, '\n-----  ----  ----  ---------    --------   -------  --------    --------');

for i = 1:nbuses_dc
        fprintf(fid,'\n %3d %5d %5d %9.3f %12.3f %9.3f %9.3f %11.3f', i, conv(i,2), conv(i,3), value(vn_dc(i)), value(pn_dc(i))*baseMVA ...
            , value(ps(i))*baseMVA, value(qs(i))*baseMVA, value(convPloss(i)*baseMVA));
end
fprintf(fid, '\n-----  ----  ----  ---------    --------   -------  --------    --------');
 fprintf(fid,'\n The total converter losses is %.3f MW',sum(value(convPloss))*baseMVA);
 fprintf(fid,'\n');
 fprintf(fid,'\n Power flow computation time is %.3fs',cputime);
 fprintf(fid,'\n');
 fprintf(fid,'\n');
