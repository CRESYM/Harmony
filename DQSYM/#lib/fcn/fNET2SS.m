function [SYS1, rlc] = fNET2SS(file)

    SYS1 = struct();
    SYS = ss();
    unit='OMU'; % Units = Ohms, mH and uF
    
    rlc = xlsread(file,'RLC'); rlc = rlc(:,1:6);
    src = xlsread(file,'SRC'); src = src(:,1:6);
    sw  = xlsread(file,'SW');
    out = readtable(file,'Sheet','OUT');   

    % out = out(:,1:9);

    yout = [out.TXT; out.TXT_1];
    yout = yout(~cellfun('isempty',yout));
    
    maxlen = max(cellfun(@length, yout));
    yout = cellfun(@(x)([x zeros(1, maxlen - length(x))]), yout, 'UniformOutput', false);

    y_type = zeros(1,size(yout,1));

    for i = 1 : size(yout,1)
       if(yout{i}(1) == 'U')
          y_type(i) = 0;
       else
           y_type(i) = 1;
       end
    end
    
    yout = cell2mat(yout);
    
    
    [A,B,C,D,stateNames,x0]=power_statespace(rlc,sw,src,[],yout,y_type,unit);


    SYS = ss(A,B,C,D);
    SYS.StateName = stateNames(~contains(stateNames,'*'));


    SYS1.A = SYS.A;
    SYS1.B = SYS.B;
    SYS1.C = SYS.C;
    SYS1.D = SYS.D;
    SYS1.x0 = x0;
    SYS1.StateName = stateNames(~contains(stateNames,'*'));
end


