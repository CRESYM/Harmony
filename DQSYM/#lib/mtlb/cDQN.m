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

        function Rpnz  = fMatAbc2Pnz(obj, Rabc)

            mrows = size(Rabc,1)/3; %signal size
            mcols = size(Rabc,2)/3; %signal size

            Rpnz = complex(zeros(3*mrows,3*mcols));

    
            for i = 1 : mrows
                for j = 1 : mcols
                    Rpnz((1:3) + (i-1)*3,(1:3) + (j-1)*3) = obj.Sas*Rabc((1:3) + (i-1)*3,(1:3) + (j-1)*3)/obj.Sas;
                end
            end

            Rpnz = real(Rpnz);


        end

        function [Adc,Bdc,Cdc,Ddc1,Ddc2,InitVec_c] = fDiscSSTest(obj,A,B,C,D,InitVec,dt,N)
        
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


        
        end
        
        % Parameter callback section


    end
end

