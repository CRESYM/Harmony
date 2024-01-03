function [ts,ps] = time_domain(xvec,tvec,maxhar,wo)
%xvec=the vector of the Fourier Coefficients
%tvec=the time series vector
%maxhar=the number of harmonics taken into consideration
%wo=fundamental angular frequency
alpha=length(xvec);
beta=length(tvec);
ts=zeros(beta,1);
ps=zeros(beta,1);
gvec=zeros(alpha,1);
for k=1:1:beta
    for j=1:1:alpha
    gvec(j,1)=exp(-(maxhar-j+1)*wo*tvec(k,1)*1i);
    end
    ts(k,1)=abs(transpose(gvec)*xvec);
    ps(k,1)=angle(transpose(gvec)*xvec);
end

end

