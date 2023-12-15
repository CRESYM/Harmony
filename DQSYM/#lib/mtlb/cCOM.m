classdef cCOM < handle
    
    properties

    end
    
    methods
        function obj = cCOM()


        end


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

