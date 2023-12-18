function [tud] = taut(x_vec,maxhar)
gt=length(x_vec);
tx=zeros(gt,gt);
beta=maxhar;
%vecr=transpose(x_vec);
%vecc=x_vec;
newv=x_vec;
ht=length(newv);
hti=ceil((ht-1)/2);
tud=zeros(ht,ht);%Toeplitz Matrix to be generated
for i=1:1:ht
    for j=1:1:ht
    if(i==j)
        tud(i,j)=newv((hti+1),1);
    else if(j>i)
            cji=(hti-(j-i)+1);
            if(cji>0)
            tud(i,j)=newv(cji,1);
            else 
                tud(i,j)=0;
            end
        else if (i>j)
                cij=(hti+(i-j)+1);
                if(cij<=ht)
               tud(i,j)=newv(cij,1);
                else
                    tud(i,j)=0;
                end
            end
        end
    end
    end
end
end