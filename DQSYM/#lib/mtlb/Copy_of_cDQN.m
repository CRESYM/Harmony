classdef cDQN < handle
    
    properties

        Sas;
        Ssa;
        f;
        Tdqn_a;
        Ta_dqn;
    end
    
    methods
        function obj = cDQN(freq)
            %CDQN Construct an instance of this class
            %   Detailed explanation goes here
            obj.f = freq;
            a = -0.5+0.866i;
            a2 = -0.5-0.866i;
            obj.Sas = [1,a,a2;1,a2,a;1,1,1]/3;

            obj.Ssa = inv(obj.Sas);

            Tad_p2 = (2/3)*[0 sin(0-2*pi/3) sin(0+2*pi/3);  
                            1 cos(0-2*pi/3) cos(0+2*pi/3);];

            Tad_z2 = (1/3)*[0 0 0;  
                            1 1 1;];

            Tda_z2 = [0     1;
                      0     1;
                      0     1];
            
            Tda_p2 = [0                 1;
                      sin(0-2*pi/3)     cos(0-2*pi/3);
                      sin(0+2*pi/3)     cos(0+2*pi/3)];

            obj.Tdqn_a = [Tda_p2(:,1)  Tda_p2(:,1)  Tda_z2(:,1)  Tda_p2(:,2)  Tda_p2(:,2)  2*Tda_z2(:,2)]/2;
            obj.Ta_dqn = [Tad_p2(1,:); Tad_p2(1,:); Tad_z2(1,:); Tad_p2(2,:); Tad_p2(2,:); Tad_z2(2,:)];

        end

        function Zdcpnz_c    = fMultDQNc(Xdcpnz_c, Ydcpnz_c, N, HrmDbl)
            Zdcpnz_c = complex(zeros(3,2*N+1));
        
            persistent T132;
            persistent T213;
            persistent T231;
            persistent T321;
            persistent T312;
            persistent Sas;
            persistent Ssa;
        
            if(isempty(T132)) 
                T132 = [1 0 0; 0 0 1; 0 1 0];
                T213 = [0 1 0; 1 0 0; 0 0 1];
                T231 = [0 1 0; 0 0 1; 1 0 0];
                T321 = [0 0 1; 0 1 0; 1 0 0];
                T312 = [0 0 1; 1 0 0; 0 1 0];
        
                a = -0.5+0.866i;
                a2 = -0.5-0.866i;
                Sas = [1,a,a2;1,a2,a;1,1,1]/3;
                Ssa = eye(3)/(Sas);
            end
        
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        
            Zdcpnz_c(:,1) = Sas*(Ssa*Xdcpnz_c(:,1).*Ssa*Ydcpnz_c(:,1));
            
               
            for h = 1 : 2*N
                if(h<=N)
        
                    Zdcpnz_c(3,1) = Zdcpnz_c(3,1) + 0.5*real(Xdcpnz_c(:,h+1)'*Ydcpnz_c(:,h+1));
        
        
                    Zdcpnz_c(3,2*h+1) = Zdcpnz_c(3,2*h+1) + 0.5*Xdcpnz_c(1,h+1)*Ydcpnz_c(2,h+1)*exp(-1i*pi/2);
        
                    Zdcpnz_c(1:2,1) = Zdcpnz_c(1:2,1) + 0.25*abs(Xdcpnz_c(1,h+1))*abs(Ydcpnz_c(2,h+1))*[1;1].*exp(1i*((-angle(Xdcpnz_c(1,h+1))+angle(Ydcpnz_c(2,h+1)))*[1;-1]));
                    Zdcpnz_c(1:2,1) = Zdcpnz_c(1:2,1) + 0.25*abs(Xdcpnz_c(1,h+1))*abs(Ydcpnz_c(3,h+1))*[1;1].*exp(1i*((-angle(Xdcpnz_c(1,h+1))+angle(Ydcpnz_c(3,h+1)))*[-1;1]));
        
        
                    Zdcpnz_c(3,2*h+1) = Zdcpnz_c(3,2*h+1) + 0.5*Xdcpnz_c(2,h+1)*Ydcpnz_c(1,h+1)*exp(-1i*pi/2);
        
        
                    Zdcpnz_c(1:2,1) = Zdcpnz_c(1:2,1) + 0.25*abs(Xdcpnz_c(2,h+1))*abs(Ydcpnz_c(1,h+1))*[1;1].*exp(1i*((-angle(Xdcpnz_c(2,h+1))+angle(Ydcpnz_c(1,h+1)))*[-1;1]));
                    Zdcpnz_c(1:2,1) = Zdcpnz_c(1:2,1) + 0.25*abs(Xdcpnz_c(2,h+1))*abs(Ydcpnz_c(3,h+1))*[1;1].*exp(1i*((-angle(Xdcpnz_c(2,h+1))+angle(Ydcpnz_c(3,h+1)))*[1;-1]));
                    
                    Zdcpnz_c(1:2,1) = Zdcpnz_c(1:2,1) + 0.25*abs(Xdcpnz_c(3,h+1))*abs(Ydcpnz_c(1,h+1))*[1;1].*exp(1i*((-angle(Xdcpnz_c(3,h+1))+angle(Ydcpnz_c(1,h+1)))*[1;-1]));
                    Zdcpnz_c(1:2,1) = Zdcpnz_c(1:2,1) + 0.25*abs(Xdcpnz_c(3,h+1))*abs(Ydcpnz_c(2,h+1))*[1;1].*exp(1i*((-angle(Xdcpnz_c(3,h+1))+angle(Ydcpnz_c(2,h+1)))*[-1;+1]));
        
                    %%%%%%%%%%%%%%%%% POSITIVE (Ydc*Xp) %%%%%%%%%%%%%%%%%%%
                    Zdcpnz_c(:,h+1) = Zdcpnz_c(:,h+1) + T312*Ydcpnz_c(:,1)*Xdcpnz_c(1,h+1); %[0 0 1; 1 0 0; 0 1 0]
                    %%%%%%%%%%%%%%%%% POSITIVE (Xdc*Yp) %%%%%%%%%%%%%%%%%%%
                    Zdcpnz_c(:,h+1) = Zdcpnz_c(:,h+1) + T312*Xdcpnz_c(:,1)*Ydcpnz_c(1,h+1);
                    %%%%%%%%%%%%%%%%% NEGATIVE (Ydc*Xn) %%%%%%%%%%%%%%%%%%%
                    Zdcpnz_c(:,h+1) = Zdcpnz_c(:,h+1) + T231*Ydcpnz_c(:,1)*Xdcpnz_c(2,h+1);
                    %%%%%%%%%%%%%%%%% NEGATIVE (Xdc*Yn) %%%%%%%%%%%%%%%%%%%
                    Zdcpnz_c(:,h+1) = Zdcpnz_c(:,h+1) + T231*Xdcpnz_c(:,1)*Ydcpnz_c(2,h+1);
                    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    Zdcpnz_c(:,h+1) = Zdcpnz_c(:,h+1) + Ydcpnz_c(:,1)*Xdcpnz_c(3,h+1);
                    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    Zdcpnz_c(:,h+1) = Zdcpnz_c(:,h+1) + Xdcpnz_c(:,1)*Ydcpnz_c(3,h+1);
                    
        %%%%%%%%%%%%%%%%%%%%%% h<=N ODD %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    if(mod(h,2)==1) 
                        %%%%%%%%%%%%%%%%% D3p (3ph+) %%%%%%%%%%%%%%%%%%%%%%%
        
                        %&%%%%%%%%%%%%%%%%%%%%%%%%%% PP + NN %%%%%%%%%%%%%%%%%%%%%%%%%%%
                        for i = 1 : h - 1
                           Zdcpnz_c(1,h+1) = Zdcpnz_c(1,h+1) +    0.5*( real(Xdcpnz_c(2,i+1))*imag(Ydcpnz_c(2,h-i+1)) + imag(Xdcpnz_c(2,i+1))*real(Ydcpnz_c(2,h-i+1)) );
                           Zdcpnz_c(1,h+1) = Zdcpnz_c(1,h+1) + 1i*0.5*( -real(Xdcpnz_c(2,i+1))*real(Ydcpnz_c(2,h-i+1)) + imag(Xdcpnz_c(2,i+1))*imag(Ydcpnz_c(2,h-i+1)) );
        
                           Zdcpnz_c(2,h+1) = Zdcpnz_c(2,h+1) +    0.5*( real(Xdcpnz_c(1,i+1))*imag(Ydcpnz_c(1,h-i+1)) + imag(Xdcpnz_c(1,i+1))*real(Ydcpnz_c(1,h-i+1)) );
                           Zdcpnz_c(2,h+1) = Zdcpnz_c(2,h+1) + 1i*0.5*( -real(Xdcpnz_c(1,i+1))*real(Ydcpnz_c(1,h-i+1)) + imag(Xdcpnz_c(1,i+1))*imag(Ydcpnz_c(1,h-i+1)) );
                        end
        
                        for i = 1 : N - h
        
                            Zdcpnz_c(3,h+1) = Zdcpnz_c(3,h+1) -    imag(0.5*Xdcpnz_c(1:2,i+1)'*Ydcpnz_c(1:2,i+h+1)) -    imag(0.5*Ydcpnz_c(1:2,i+1)'*Xdcpnz_c(1:2,i+h+1));
                            Zdcpnz_c(3,h+1) = Zdcpnz_c(3,h+1) + 1i*real(0.5*Xdcpnz_c(1:2,i+1)'*Ydcpnz_c(1:2,i+h+1)) + 1i*real(0.5*Ydcpnz_c(1:2,i+1)'*Xdcpnz_c(1:2,i+h+1));
        
                        end
                        %&%%%%%%%%%%%%%%%%%%%%%%%%%% PP + NN %%%%%%%%%%%%%%%%%%%%%%%%%%%
        
                        for k = 1 : (N + mod(N+1,2) - h)/2
                            i = 2*k-1;
                            j = i+h;
        
                            Zdcpnz_c(1,h+1) = Zdcpnz_c(1,h+1) + (0.5*(Xdcpnz_c(:,i+1)'*T132*T213*Ydcpnz_c(:,j+1))*exp(+1i*pi/2)) ...
                                                              + (0.5*(Xdcpnz_c(:,j+1)'*T213*T132*Ydcpnz_c(:,i+1))*exp(-1i*pi/2))';
        
                            Zdcpnz_c(2,h+1) = Zdcpnz_c(2,h+1) + (0.5*(Xdcpnz_c(:,i+1)'*T132*T321*Ydcpnz_c(:,j+1))*exp(+1i*pi/2)) ...
                                                              + (0.5*(Xdcpnz_c(:,j+1)'*T321*T132*Ydcpnz_c(:,i+1))*exp(-1i*pi/2))';
        
                            Zdcpnz_c(3,h+1) = Zdcpnz_c(3,h+1) + (0.5*Xdcpnz_c(3,i+1)'*Ydcpnz_c(3,j+1)*exp(1i*pi/2)) ...
                                                              - (0.5*Xdcpnz_c(3,j+1)'*Ydcpnz_c(3,i+1)*exp(1i*pi/2))';
                        end
        
                        for k = 1 : (N + mod(N,2) - h - 1)/2
                            i = 2*k;
                            j = i+h;
                            
                            Zdcpnz_c(1,h+1) = Zdcpnz_c(1,h+1) + (0.5*(Xdcpnz_c(:,i+1)'*T132*T213*Ydcpnz_c(:,j+1))*exp(+1i*pi/2)) ...
                                                              + (0.5*(Xdcpnz_c(:,j+1)'*T213*T132*Ydcpnz_c(:,i+1))*exp(-1i*pi/2))';
        
                            Zdcpnz_c(2,h+1) = Zdcpnz_c(2,h+1) + (0.5*(Xdcpnz_c(:,i+1)'*T132*T321*Ydcpnz_c(:,j+1))*exp(+1i*pi/2)) ...
                                                              + (0.5*(Xdcpnz_c(:,j+1)'*T321*T132*Ydcpnz_c(:,i+1))*exp(-1i*pi/2))';
        
                            Zdcpnz_c(3,h+1) = Zdcpnz_c(3,h+1) + (0.5*Xdcpnz_c(3,i+1)'*Ydcpnz_c(3,j+1)*exp(1i*pi/2)) ...
                                                              - (0.5*Xdcpnz_c(3,j+1)'*Ydcpnz_c(3,i+1)*exp(1i*pi/2))';
                        end
        
                        for k = (h-1)/2:-1:1
                            i = k;
                            j = h-i;  
        
                            Zdcpnz_c(3,h+1) = Zdcpnz_c(3,h+1) + 0.5*(real(Xdcpnz_c(:,i+1))'*T213*imag(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,i+1))'*T213*real(Ydcpnz_c(:,j+1)) ...
                                                                   + real(Xdcpnz_c(:,j+1))'*T213*imag(Ydcpnz_c(:,i+1)) + imag(Xdcpnz_c(:,j+1))'*T213*real(Ydcpnz_c(:,i+1)));
                                                                                               
                            Zdcpnz_c(3,h+1) = Zdcpnz_c(3,h+1) + 1i*0.5*(-real(Xdcpnz_c(:,i+1))'*T213*real(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,i+1))'*T213*imag(Ydcpnz_c(:,j+1)) ...
                                                                   - real(Xdcpnz_c(:,j+1))'*T213*real(Ydcpnz_c(:,i+1)) + imag(Xdcpnz_c(:,j+1))'*T213*imag(Ydcpnz_c(:,i+1)));
        
                            Zdcpnz_c(1,h+1) = Zdcpnz_c(1,h+1) + 0.5*(real(Xdcpnz_c([1,3],i+1))'*[0 1; 1 0]*imag(Ydcpnz_c([1,3],j+1)) + imag(Xdcpnz_c([1,3],i+1))'*[0 1; 1 0]*real(Ydcpnz_c([1,3],j+1)) ...
                                                                   + real(Xdcpnz_c([1,3],j+1))'*[0 1; 1 0]*imag(Ydcpnz_c([1,3],i+1)) + imag(Xdcpnz_c([1,3],j+1))'*[0 1; 1 0]*real(Ydcpnz_c([1,3],i+1)) );
        
                            Zdcpnz_c(1,h+1) = Zdcpnz_c(1,h+1) + 1i*0.5*(-real(Xdcpnz_c([1,3],i+1))'*[0 1; 1 0]*real(Ydcpnz_c([1,3],j+1)) + imag(Xdcpnz_c([1,3],i+1))'*[0 1; 1 0]*imag(Ydcpnz_c([1,3],j+1)) ...
                                                                      - real(Xdcpnz_c([1,3],j+1))'*[0 1; 1 0]*real(Ydcpnz_c([1,3],i+1)) + imag(Xdcpnz_c([1,3],j+1))'*[0 1; 1 0]*imag(Ydcpnz_c([1,3],i+1)) );
        
                            Zdcpnz_c(2,h+1) = Zdcpnz_c(2,h+1) + 0.5*(real(Xdcpnz_c([2,3],i+1))'*[0 1; 1 0]*imag(Ydcpnz_c([2,3],j+1)) + imag(Xdcpnz_c([2,3],i+1))'*[0 1; 1 0]*real(Ydcpnz_c([2,3],j+1)) ...
                                                                   + real(Xdcpnz_c([2,3],j+1))'*[0 1; 1 0]*imag(Ydcpnz_c([2,3],i+1)) + imag(Xdcpnz_c([2,3],j+1))'*[0 1; 1 0]*real(Ydcpnz_c([2,3],i+1)) );
        
                            Zdcpnz_c(2,h+1) = Zdcpnz_c(2,h+1) + 1i*0.5*(-real(Xdcpnz_c([2,3],i+1))'*[0 1; 1 0]*real(Ydcpnz_c([2,3],j+1)) + imag(Xdcpnz_c([2,3],i+1))'*[0 1; 1 0]*imag(Ydcpnz_c([2,3],j+1)) ...
                                                                      - real(Xdcpnz_c([2,3],j+1))'*[0 1; 1 0]*real(Ydcpnz_c([2,3],i+1)) + imag(Xdcpnz_c([2,3],j+1))'*[0 1; 1 0]*imag(Ydcpnz_c([2,3],i+1)) );
        
                        end
        %%%%%%%%%%%%%%%%%%%%%% h<=N EVEN %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%           
                    else  
        
                        Zdcpnz_c(3,h+1) = Zdcpnz_c(3,h+1) + 0.5*imag(Xdcpnz_c(3,h/2+1)*Ydcpnz_c(3,h/2+1))           -0.5*1i*real(Xdcpnz_c(3,h/2+1)*Ydcpnz_c(3,h/2+1));
                        Zdcpnz_c(1,h+1) = Zdcpnz_c(1,h+1) + 0.5*real(Xdcpnz_c(:,h/2+1))'*T321*imag(Ydcpnz_c(:,h/2+1)) + 0.5*imag(Xdcpnz_c(:,h/2+1))'*T321*real(Ydcpnz_c(:,h/2+1));
                        Zdcpnz_c(1,h+1) = Zdcpnz_c(1,h+1) - 0.5*1i*real(Xdcpnz_c(:,h/2+1))'*T321*real(Ydcpnz_c(:,h/2+1)) + 0.5*1i*imag(Xdcpnz_c(:,h/2+1))'*T321*imag(Ydcpnz_c(:,h/2+1));
                        Zdcpnz_c(2,h+1) = Zdcpnz_c(2,h+1) + 0.5*real(Xdcpnz_c(:,h/2+1))'*T132*imag(Ydcpnz_c(:,h/2+1)) + 0.5*imag(Xdcpnz_c(:,h/2+1))'*T132*real(Ydcpnz_c(:,h/2+1));
                        Zdcpnz_c(2,h+1) = Zdcpnz_c(2,h+1) - 0.5*1i*real(Xdcpnz_c(:,h/2+1))'*T132*real(Ydcpnz_c(:,h/2+1)) + 0.5*1i*imag(Xdcpnz_c(:,h/2+1))'*T132*imag(Ydcpnz_c(:,h/2+1));
                        
                        for k = 1:N-h
                            i = k;
                            j = h+i;
        
                            Zdcpnz_c(1,h+1) = Zdcpnz_c(1,h+1) + 0.5*( - real(Xdcpnz_c(:,i+1))'*T231*imag(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,i+1))'*T231*real(Ydcpnz_c(:,j+1)) ...
                                                                      + real(Xdcpnz_c(:,j+1))'*T312*imag(Ydcpnz_c(:,i+1)) - imag(Xdcpnz_c(:,j+1))'*T312*real(Ydcpnz_c(:,i+1)));
        
                            Zdcpnz_c(1,h+1) = Zdcpnz_c(1,h+1) + 0.5*1i*( + real(Xdcpnz_c(:,i+1))'*T231*real(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,i+1))'*T231*imag(Ydcpnz_c(:,j+1)) ...
                                                                         + real(Xdcpnz_c(:,j+1))'*T312*real(Ydcpnz_c(:,i+1)) + imag(Xdcpnz_c(:,j+1))'*T312*imag(Ydcpnz_c(:,i+1)));
        
                            Zdcpnz_c(2,h+1) = Zdcpnz_c(2,h+1) + 0.5*( - real(Xdcpnz_c(:,i+1))'*T312*imag(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,i+1))'*T312*real(Ydcpnz_c(:,j+1)) ...
                                                                      + real(Xdcpnz_c(:,j+1))'*T231*imag(Ydcpnz_c(:,i+1)) - imag(Xdcpnz_c(:,j+1))'*T231*real(Ydcpnz_c(:,i+1)));
        
                            Zdcpnz_c(2,h+1) = Zdcpnz_c(2,h+1) + 1i*0.5*( + real(Xdcpnz_c(:,i+1))'*T312*real(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,i+1))'*T312*imag(Ydcpnz_c(:,j+1)) ...
                                                                      + real(Xdcpnz_c(:,j+1))'*T231*real(Ydcpnz_c(:,i+1)) + imag(Xdcpnz_c(:,j+1))'*T231*imag(Ydcpnz_c(:,i+1)));
        
                            Zdcpnz_c(3,h+1) = Zdcpnz_c(3,h+1) + 0.5*(-real(Xdcpnz_c(:,i+1))'*imag(Ydcpnz_c(:,j+1))+imag(Xdcpnz_c(:,i+1))'*real(Ydcpnz_c(:,j+1)) ...
                                                                     +real(Xdcpnz_c(:,j+1))'*imag(Ydcpnz_c(:,i+1))-imag(Xdcpnz_c(:,j+1))'*real(Ydcpnz_c(:,i+1)));
        
                            Zdcpnz_c(3,h+1) = Zdcpnz_c(3,h+1) + 0.5*1i*(+real(Xdcpnz_c(:,i+1))'*real(Ydcpnz_c(:,j+1))+imag(Xdcpnz_c(:,i+1))'*imag(Ydcpnz_c(:,j+1)) ...
                                                                     +real(Xdcpnz_c(:,j+1))'*real(Ydcpnz_c(:,i+1))+imag(Xdcpnz_c(:,j+1))'*imag(Ydcpnz_c(:,i+1)));
        
                        end
        
                        for k = h/2-1:-1:1
                            i = k;
                            j = h-i;   
        
                            Zdcpnz_c(3,h+1) = Zdcpnz_c(3,h+1) +    0.5*(+real(Xdcpnz_c(:,i+1))'*T213*imag(Ydcpnz_c(:,j+1)) + real(Xdcpnz_c(:,j+1))'*T213*imag(Ydcpnz_c(:,i+1)) + imag(Xdcpnz_c(:,i+1))'*T213*real(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,j+1))'*T213*real(Ydcpnz_c(:,i+1)));
        
                            Zdcpnz_c(3,h+1) = Zdcpnz_c(3,h+1) + 1i*0.5*(-real(Xdcpnz_c(:,i+1))'*T213*real(Ydcpnz_c(:,j+1)) - real(Xdcpnz_c(:,j+1))'*T213*real(Ydcpnz_c(:,i+1)) + imag(Xdcpnz_c(:,i+1))'*T213*imag(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,j+1))'*T213*imag(Ydcpnz_c(:,i+1)));
        
                            Zdcpnz_c(1,h+1) = Zdcpnz_c(1,h+1) + 0.5*( + real(Xdcpnz_c(:,i+1))'*T321*imag(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,i+1))'*T321*real(Ydcpnz_c(:,j+1)) ...
                                                                      + real(Xdcpnz_c(:,j+1))'*T321*imag(Ydcpnz_c(:,i+1)) + imag(Xdcpnz_c(:,j+1))'*T321*real(Ydcpnz_c(:,i+1)));
        
        
                            Zdcpnz_c(1,h+1) = Zdcpnz_c(1,h+1) + 0.5*1i*( - real(Xdcpnz_c(:,i+1))'*T321*real(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,i+1))'*T321*imag(Ydcpnz_c(:,j+1)) ...
                                                                         - real(Xdcpnz_c(:,j+1))'*T321*real(Ydcpnz_c(:,i+1)) + imag(Xdcpnz_c(:,j+1))'*T321*imag(Ydcpnz_c(:,i+1)));
                            
                            Zdcpnz_c(2,h+1) = Zdcpnz_c(2,h+1) + 0.5*( + real(Xdcpnz_c(:,i+1))'*T132*imag(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,i+1))'*T132*real(Ydcpnz_c(:,j+1)) ...
                                                                      + real(Xdcpnz_c(:,j+1))'*T132*imag(Ydcpnz_c(:,i+1)) + imag(Xdcpnz_c(:,j+1))'*T132*real(Ydcpnz_c(:,i+1)));
        
                            Zdcpnz_c(2,h+1) = Zdcpnz_c(2,h+1) + 1i*0.5*( - real(Xdcpnz_c(:,i+1))'*T132*real(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,i+1))'*T132*imag(Ydcpnz_c(:,j+1)) ...
                                                                         - real(Xdcpnz_c(:,j+1))'*T132*real(Ydcpnz_c(:,i+1)) + imag(Xdcpnz_c(:,j+1))'*T132*imag(Ydcpnz_c(:,i+1)));
        
                        end
                    end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%% h>N  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
                else %h>N
                    if(HrmDbl)
                    
                        %%%%%%%%%%%%%%%%%%%%%%%%%%% PP + NN %%%%%%%%%%%%%%%%%%%%%%%%%%%
                        for i = h-N : N
                            Zdcpnz_c(1,h+1) = Zdcpnz_c(1,h+1) +    0.5*( real(Xdcpnz_c(2,i+1))*imag(Ydcpnz_c(2,h-i+1)) + imag(Xdcpnz_c(2,i+1))*real(Ydcpnz_c(2,h-i+1)) );
                            Zdcpnz_c(1,h+1) = Zdcpnz_c(1,h+1) + 1i*0.5*( -real(Xdcpnz_c(2,i+1))*real(Ydcpnz_c(2,h-i+1)) + imag(Xdcpnz_c(2,i+1))*imag(Ydcpnz_c(2,h-i+1)) );
        
                            Zdcpnz_c(2,h+1) = Zdcpnz_c(2,h+1) +    0.5*( real(Xdcpnz_c(1,i+1))*imag(Ydcpnz_c(1,h-i+1)) + imag(Xdcpnz_c(1,i+1))*real(Ydcpnz_c(1,h-i+1)) );
                            Zdcpnz_c(2,h+1) = Zdcpnz_c(2,h+1) + 1i*0.5*( -real(Xdcpnz_c(1,i+1))*real(Ydcpnz_c(1,h-i+1)) + imag(Xdcpnz_c(1,i+1))*imag(Ydcpnz_c(1,h-i+1)) );
                        end
                        %%%%%%%%%%%%%%%%%%%%%%%%%%% PP + NN %%%%%%%%%%%%%%%%%%%%%%%%%%%
        
                        if(mod(h,2)==1) %%%%%%%%%%% h>N ODD %%%%%%%%%%%%%%%%%%%
        
                            for k = (h-1)/2:-1:h-N
                                i = k;  %%%
                                j = h-i;
        
                                Zdcpnz_c(1,h+1) = Zdcpnz_c(1,h+1) + 0.5*( + real(Xdcpnz_c([1,3],i+1))'*imag(Ydcpnz_c([3,1],j+1)) + imag(Xdcpnz_c([1,3],i+1))'*real(Ydcpnz_c([3,1],j+1)) ...
                                                                          + real(Xdcpnz_c([1,3],j+1))'*imag(Ydcpnz_c([3,1],i+1)) + imag(Xdcpnz_c([1,3],j+1))'*real(Ydcpnz_c([3,1],i+1)));
        
                                Zdcpnz_c(1,h+1) = Zdcpnz_c(1,h+1) + 1i*0.5*( - real(Xdcpnz_c([1,3],i+1))'*real(Ydcpnz_c([3,1],j+1)) + imag(Xdcpnz_c([1,3],i+1))'*imag(Ydcpnz_c([3,1],j+1)) ...
                                                                             - real(Xdcpnz_c([1,3],j+1))'*real(Ydcpnz_c([3,1],i+1)) + imag(Xdcpnz_c([1,3],j+1))'*imag(Ydcpnz_c([3,1],i+1)));
        
                                Zdcpnz_c(2,h+1) = Zdcpnz_c(2,h+1) + 0.5*( + real(Xdcpnz_c([2,3],i+1))'*imag(Ydcpnz_c([3,2],j+1)) + imag(Xdcpnz_c([2,3],i+1))'*real(Ydcpnz_c([3,2],j+1)) ...
                                                                          + real(Xdcpnz_c([2,3],j+1))'*imag(Ydcpnz_c([3,2],i+1)) + imag(Xdcpnz_c([2,3],j+1))'*real(Ydcpnz_c([3,2],i+1)));
        
                                Zdcpnz_c(2,h+1) = Zdcpnz_c(2,h+1) + 1i*0.5*( - real(Xdcpnz_c([2,3],i+1))'*real(Ydcpnz_c([3,2],j+1)) + imag(Xdcpnz_c([2,3],i+1))'*imag(Ydcpnz_c([3,2],j+1)) ...
                                                                             - real(Xdcpnz_c([2,3],j+1))'*real(Ydcpnz_c([3,2],i+1)) + imag(Xdcpnz_c([2,3],j+1))'*imag(Ydcpnz_c([3,2],i+1)));
        
                                Zdcpnz_c(3,h+1) = Zdcpnz_c(3,h+1) +    0.5*(+real(Xdcpnz_c(:,i+1))'*T213*imag(Ydcpnz_c(:,j+1)) + real(Xdcpnz_c(:,j+1))'*T213*imag(Ydcpnz_c(:,i+1)) + imag(Xdcpnz_c(:,i+1))'*T213*real(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,j+1))'*T213*real(Ydcpnz_c(:,i+1)));
                                Zdcpnz_c(3,h+1) = Zdcpnz_c(3,h+1) + 1i*0.5*(-real(Xdcpnz_c(:,i+1))'*T213*real(Ydcpnz_c(:,j+1)) - real(Xdcpnz_c(:,j+1))'*T213*real(Ydcpnz_c(:,i+1)) + imag(Xdcpnz_c(:,i+1))'*T213*imag(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,j+1))'*T213*imag(Ydcpnz_c(:,i+1)));
        
                            end
        
                        else %%%%%%%%%%% h>N EVEN %%%%%%%%%%%%%%%%%%%
        
                            Zdcpnz_c(1,h+1) = 0.5*real(Xdcpnz_c(:,h/2+1))'*T321*imag(Ydcpnz_c(:,h/2+1)) + 0.5*imag(Xdcpnz_c(:,h/2+1))'*T321*real(Ydcpnz_c(:,h/2+1)) - 0.5*1i*real(Xdcpnz_c(:,h/2+1))'*T321*real(Ydcpnz_c(:,h/2+1)) + 0.5*1i*imag(Xdcpnz_c(:,h/2+1))'*T321*imag(Ydcpnz_c(:,h/2+1));
                            Zdcpnz_c(2,h+1) = 0.5*real(Xdcpnz_c(:,h/2+1))'*T132*imag(Ydcpnz_c(:,h/2+1)) + 0.5*imag(Xdcpnz_c(:,h/2+1))'*T132*real(Ydcpnz_c(:,h/2+1))- 0.5*1i*real(Xdcpnz_c(:,h/2+1))'*T132*real(Ydcpnz_c(:,h/2+1)) + 0.5*1i*imag(Xdcpnz_c(:,h/2+1))'*T132*imag(Ydcpnz_c(:,h/2+1));
                            Zdcpnz_c(3,h+1) = Zdcpnz_c(3,h+1) + 0.5*imag(Xdcpnz_c(3,h/2+1)*Ydcpnz_c(3,h/2+1)) - 1i*0.5*real(Xdcpnz_c(3,h/2+1)*Ydcpnz_c(3,h/2+1));
        
        
                            for i = (h+2)/2:N
                                j = h-i;
        
                                Zdcpnz_c(1,h+1) = Zdcpnz_c(1,h+1) + 0.5*( + real(Xdcpnz_c(:,i+1))'*T321*imag(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,i+1))'*T321*real(Ydcpnz_c(:,j+1)) ...
                                                                          + real(Xdcpnz_c(:,j+1))'*T321*imag(Ydcpnz_c(:,i+1)) + imag(Xdcpnz_c(:,j+1))'*T321*real(Ydcpnz_c(:,i+1)));
            
                                Zdcpnz_c(1,h+1) = Zdcpnz_c(1,h+1) + 0.5*1i*( - real(Xdcpnz_c(:,i+1))'*T321*real(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,i+1))'*T321*imag(Ydcpnz_c(:,j+1)) ...
                                                                             - real(Xdcpnz_c(:,j+1))'*T321*real(Ydcpnz_c(:,i+1)) + imag(Xdcpnz_c(:,j+1))'*T321*imag(Ydcpnz_c(:,i+1)));
        
                                Zdcpnz_c(2,h+1) = Zdcpnz_c(2,h+1) + 0.5*( + real(Xdcpnz_c(:,i+1))'*T132*imag(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,i+1))'*T132*real(Ydcpnz_c(:,j+1)) ...
                                                                          + real(Xdcpnz_c(:,j+1))'*T132*imag(Ydcpnz_c(:,i+1)) + imag(Xdcpnz_c(:,j+1))'*T132*real(Ydcpnz_c(:,i+1)));
            
                                Zdcpnz_c(2,h+1) = Zdcpnz_c(2,h+1) + 0.5*1i*( - real(Xdcpnz_c(:,i+1))'*T132*real(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,i+1))'*T132*imag(Ydcpnz_c(:,j+1)) ...
                                                                             - real(Xdcpnz_c(:,j+1))'*T132*real(Ydcpnz_c(:,i+1)) + imag(Xdcpnz_c(:,j+1))'*T132*imag(Ydcpnz_c(:,i+1)));
        
                                Zdcpnz_c(3,h+1) = Zdcpnz_c(3,h+1) +    0.5*(+real(Xdcpnz_c(:,i+1))'*T213*imag(Ydcpnz_c(:,j+1)) + real(Xdcpnz_c(:,j+1))'*T213*imag(Ydcpnz_c(:,i+1)) + imag(Xdcpnz_c(:,i+1))'*T213*real(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,j+1))'*T213*real(Ydcpnz_c(:,i+1)));
                                Zdcpnz_c(3,h+1) = Zdcpnz_c(3,h+1) + 1i*0.5*(-real(Xdcpnz_c(:,i+1))'*T213*real(Ydcpnz_c(:,j+1)) - real(Xdcpnz_c(:,j+1))'*T213*real(Ydcpnz_c(:,i+1)) + imag(Xdcpnz_c(:,i+1))'*T213*imag(Ydcpnz_c(:,j+1)) + imag(Xdcpnz_c(:,j+1))'*T213*imag(Ydcpnz_c(:,i+1)));
        
                            end
                        end
                    end
                end  
            end
            
        
        end
  
        function Ydcpnz_c  = fDQN012C(obj, X_DQN)
        
            if(size(X_DQN,2) < 1)
                error('Wrong size!');
            end
            
            N = size(X_DQN,1);
        
            if(size(X_DQN,1) == 6 && size(X_DQN,2) == 1)
                N = 1;
                X_DQN = X_DQN';
            end
        
            Ydcpnz_c = complex(zeros(3,N));
        
            for i = 1 : N
             
                if(i>1)
                    Ydcpnz_c(1,i) = (X_DQN(i,1)) + 1i*X_DQN(i,4);
                    Ydcpnz_c(2,i) = (-X_DQN(i,2)) + 1i*X_DQN(i,5);
                    Ydcpnz_c(3,i) = (X_DQN(i,3)) + 1i*X_DQN(i,6);
                else
        
                    Ydcpnz_c(1,i) = 0.5*(X_DQN(i,4) - 1i*X_DQN(i,1));
                    Ydcpnz_c(2,i) = 0.5*(X_DQN(i,5) + 1i*X_DQN(i,2));
                    Ydcpnz_c(3,i) =     (X_DQN(i,6) + 1i*X_DQN(i,3));
                end
            
            end
        
        
        end
    
        function Y_DQN  = f012C2DQN(obj, Xdcpnz_c)
        
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

        function Y_PNZ  = fdq2PNZc(obj, Xd, Xq)


            Y_PNZ = complex(zeros(3,2));
            Y_PNZ(1,2) = Xd + 1i*Xq;
        
        
        end

        function Y_PNZ  = fdqdq2PNZc(obj, dqdq)
            sdq = numel(dqdq)/2;

            Y_PNZ = complex(zeros(3*sdq,2));
            for i = 1 : sdq
                Y_PNZ(1+(i-1)*3,2) = dqdq(2*i-1) + 1i*dqdq(2*i);
            end
        
        
        end

        function Y_PNZ  = fCalcMat(obj, A, B)
            Y_PNZ = complex(zeros(3,3));

            C = ( [real(A), -imag(A); imag(A) real(A)] ) \ [real(B);imag(B)];
        
            Y_PNZ(1,1) = C(1) + 1i*C(2);
        end


        function Ac  = fdq2pnzMAT(obj, A, X)
            sA = size(A,1);
            if(size(A,1) ~= size(A,2) || mod(sA,2) ~= 0)
            error('[fdq2pnzMAT] Wrong matrix A size');
            end
            if(size(X,1)~=size(A,2))
                error('[fdq2pnzMAT] Wrong vector X size');
            end
 
            Ac = complex(zeros(sA/2*3));

            B = A*X;

            C =  obj.fCalcMat(X(1)+1i*X(2) , B(1)+1i*B(2));


        end

    end
end

