function Y_DQN  = f012C2DQN(Xdcpnz_c)

    if(size(Xdcpnz_c,2) < 2)
        error('Wrong size!');
    end
    
    N = size(Xdcpnz_c,2)-1;
    Y_DQN = zeros(N+1,6);

    Y_DQN(1,4) =  2*real(Xdcpnz_c(1,1));
    Y_DQN(1,5) =  2*real(Xdcpnz_c(2,1));
    Y_DQN(1,6) =  real(Xdcpnz_c(3,1));

    Y_DQN(1,1) =  -2*imag(Xdcpnz_c(1,1));
    Y_DQN(1,2) =  2*imag(Xdcpnz_c(2,1));
    Y_DQN(1,3) =  imag(Xdcpnz_c(3,1));
    
    for i = 2 : N + 1
    
        Y_DQN(i,1) =  real(Xdcpnz_c(1,i));
        Y_DQN(i,2) = -real(Xdcpnz_c(2,i));
        Y_DQN(i,3) =  real(Xdcpnz_c(3,i));

        Y_DQN(i,4) =  imag(Xdcpnz_c(1,i));
        Y_DQN(i,5) =  imag(Xdcpnz_c(2,i));
        Y_DQN(i,6) =  imag(Xdcpnz_c(3,i));
    
    end


end