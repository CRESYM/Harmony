function [f,P1] = plot_fft(Fs,X)
                             
%T = 1/Fs;             % Sampling period       
L = length(X);             % Length of signal
%t = (0:L-1)*T;
Y = fft(X);
P2 = abs(Y/L);
P1 = P2(1:L/2+1);
P1(2:end-1) = 2*P1(2:end-1);
f = Fs/L*(0:(L/2));


end

