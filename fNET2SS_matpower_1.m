function [SYS, rlc] = fNET2SS_matpower(mpc)

SYS = ss();
unit='OMU'; % Units = Ohms, mH and uF

bus = mpc.bus;
branch = mpc.branch;
baseMVA = mpc.baseMVA;

[m1,n1] = size(branch);
[m2,n2] = size(bus);

m1_3ph = m1*3;
m2_3ph = m2*3;

rlc = zeros(m1_3ph + m2_3ph, 6);

for i = 1:m1
    for j = 1:3

        rlc((i-1)*3 + j,1) = branch(i,1) + 0.1*j;
        rlc((i-1)*3 + j,2) = branch(i,2) + 0.1*j;
        rlc((i-1)*3 + j,3) = 0;

        Z_base = (bus(branch(i,1),10)^2)/baseMVA;

        rlc((i-1)*3 + j,4) = Z_base*branch(i,3);
        rlc((i-1)*3 + j,5) = Z_base*branch(i,4)/(2*pi*50)*10^3;
        rlc((i-1)*3 + j,6) = 0;

    end
end

for i = 1:m2
    for j = 1:3

        rlc(m1_3ph + (branch(i,1) - 1)*3 + j,1) = branch(i,1) + 0.1*j;
        rlc(m1_3ph + (branch(i,1) - 1)*3 + j,2) = 0;
        rlc(m1_3ph + (branch(i,1) - 1)*3 + j,3) = 1;
        rlc(m1_3ph + (branch(i,1) - 1)*3 + j,4) = 0;
        rlc(m1_3ph + (branch(i,1) - 1)*3 + j,5) = 0;
        
        Y_base = baseMVA/(bus(branch(i,1),10)^2);

        rlc(m1_3ph + (branch(i,1) - 1)*3 + j,6) = Y_base*branch(i,5)/(2*pi*50)*(10^6)*0.5;

    end
end

for i = 1:m2
    for j = 1:3

        rlc(m1_3ph + (branch(i,2) - 1)*3 + j,1) = branch(i,2) + 0.1*j;
        rlc(m1_3ph + (branch(i,2) - 1)*3 + j,2) = 0;
        rlc(m1_3ph + (branch(i,2) - 1)*3 + j,3) = 1;
        rlc(m1_3ph + (branch(i,2) - 1)*3 + j,4) = 0;
        rlc(m1_3ph + (branch(i,2) - 1)*3 + j,5) = 0;
        
        Y_base = baseMVA/(bus(branch(i,2),10)^2);

        rlc(m1_3ph + (branch(i,2) - 1)*3 + j,6) = rlc(m1_3ph + (branch(i,2) - 1)*3 + j,6) + Y_base*branch(i,5)/(2*pi*50)*(10^6)*0.5;

    end
end

src = zeros(m2_3ph, 6);

for i = 1:m2
    for j = 1:3
        src((i-1)*3 + j,1) = 0;
        src((i-1)*3 + j,2) = bus(i) + 0.1*j;
        src((i-1)*3 + j,3) = 1;
        src((i-1)*3 + j,4) = 1;
        src((i-1)*3 + j,5) = 0;
        src((i-1)*3 + j,6) = 50;
    end
end

    sw = [];

    yout = char(m1_3ph + m2_3ph, 1);

    for i = 1:(m1_3ph + m2_3ph)
        yout(i) = 'i';
    end

    y_type = zeros(1, m1_3ph + m2_3ph);

    [A,B,C,D,stateNames] = power_statespace(rlc,sw,src,[],yout,y_type,unit);

    SYS = ss(A,-B,-C,D);
    SYS.StateName = stateNames;
    
end

