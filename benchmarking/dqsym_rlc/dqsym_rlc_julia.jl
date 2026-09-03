using LinearAlgebra, Statistics, Printf

function transform_blocks(M,S,Sinv)
    out=zeros(ComplexF64,size(M))
    for i=0:size(M,1)÷3-1, j=0:size(M,2)÷3-1
        r=3i+1:3i+3; c=3j+1:3j+3; out[r,c]=S*M[r,c]*Sinv
    end
    out
end

function dqn2abc(X,theta,Sinv)
    abc=real.(Sinv*X[:,1]); shiftsP=[0,-2pi/3,2pi/3]; shiftsN=[0,2pi/3,-2pi/3]
    for h=1:size(X,2)-1
        z=X[:,h+1]; m=abs.(z); a=angle.(z)
        abc .+= m[1].*sin.(h*theta+a[1].+shiftsP)
        abc .+= m[2].*sin.(h*theta+a[2].+shiftsN)
        abc .+= m[3]*sin(h*theta+a[3])
    end
    abc
end

function simulate(tend=.5,dt=2e-5,f0=50.0)
    A=ComplexF64[.999055 0 0 -.00900385 0 0;0 .999055 0 0 -.00900385 0;0 0 .999055 0 0 -.00900385;.00999427 0 0 .999755 0 0;0 .00999427 0 0 .999755 0;0 0 .00999427 0 0 .999755]
    B=ComplexF64[450.238 0 0;0 450.238 0;0 0 450.238;2.25096 0 0;0 2.25096 0;0 0 2.25096]
    C=ComplexF64[9.99427e-8 0 0 1.99976e-5 0 0;0 9.99427e-8 0 0 1.99976e-5 0;0 0 9.99427e-8 0 0 1.99976e-5;-1.99905e-5 0 0 9.00385e-8 0 0;0 -1.99905e-5 0 0 9.00385e-8 0;0 0 -1.99905e-5 0 0 9.00385e-8]
    D=ComplexF64[2.25096e-5 0 0;0 2.25096e-5 0;0 0 2.25096e-5;-.00450238 0 0;0 -.00450238 0;0 0 -.00450238]
    u=ComplexF64[0 -245.5-880.2im -102.1+319.4im 373.9+733.8im 255.1-885.1im 161.1+983.9im;0 -216.5+514.1im -400.2+211.4im -82.4-141.3im -536.9+212.4im -515.6-399.9im;0 -119.4-115.7im -335.7-128.8im -889.6-92.9im 3.2-257.6im 242.7+301.7im]
    a=-.5+.866im; S=ComplexF64[1 a conj(a);1 conj(a) a;1 1 1]/3; Sinv=inv(S)
    A0=transform_blocks(A,S,Sinv); B0=transform_blocks(B,S,Sinv); C0=transform_blocks(C,S,Sinv); D0=transform_blocks(D,S,Sinv)
    rot=exp.(-im*2pi*f0*dt*(0:size(u,2)-1)); n=Int(floor(tend/dt))+1
    abc=zeros(2,n,3); brk=zeros(Int32,n,3); xold=zeros(ComplexF64,6,6)
    elapsed=@elapsed for k=1:n
        t=(k-1)*dt; brk[k,:].=(2e-4<=t<6e-4)
        xold=(A0*xold+B0*u).*transpose(rot); y=C0*xold+D0*u; theta=2pi*f0*t
        abc[1,k,:]=dqn2abc(y[1:3,:],theta,Sinv); abc[2,k,:]=dqn2abc(y[4:6,:],theta,Sinv)
    end
    elapsed,Sys.maxrss()/2.0^20,xold
end

tend=.5; repeats=5
for (i,arg) in enumerate(ARGS)
    arg=="--t-end" && (global tend=parse(Float64,ARGS[i+1])); arg=="--repeats" && (global repeats=parse(Int,ARGS[i+1]))
end
simulate(min(tend,2e-5)) 
runs=Float64[]; result=nothing
for _=1:repeats; global result=simulate(tend); push!(runs,result[1]); end
@printf("Julia DQsym RLC: steps=%d, median_time_s=%.9f, peak_memory_mb=%.3f\n",Int(floor(tend/2e-5))+1,median(runs),result[2])
@printf("final_state_checksum=%.16e\n",sum(abs.(result[3])))
