#ifndef _ADMITTANCE_H_
#define _ADMITTANCE_H_

#include "Element.h"

/*
Creates admittance with specified number of input/output pins which represent phases. 
The admittance expression has to be given in Omega and can have both numerical and 
symbolic value (example: `z = s-2`). Depending on the provided vector of admittance values, 
we differ three cases. If the number of element pins is greater than 1, admittance can be 
represented with vector with one, `pins` or `pins × pins` number of elements. Namely:
-In the case of 1×1 vector, admittance has only one value. Then this value is given to all 
diagonal admittance entries.
-In the case of `pins` elements, they are representing diagonal entiries of admittance.
-In case of `pins x pins` elements, they are representing all matrix of admittance.
*/

class Admittance : public Element {
public:
    // Unified constructor for single-phase and three-phase systems
    Admittance(const std::string& symbol, int pins, DenseMatrix values);

    ~Admittance() override;

    void compute_y_parameters(double frequency) override;
    //void compute_y_parameters() override;

    // Print Admittance values
    void printElementValues() override {
        std::cout << "Admittance Matrix for element: " << getElementSymbol() << std::endl;
        for (int i = 0; i < Y_matrix.nrows(); i++) {
            for (int j = 0; j < Y_matrix.ncols(); j++) {
                std::cout << Y_matrix.get(i, j)->__str__() << " "; // Use __str__() for output
            }
            std::cout << std::endl;
        }
    }

private:
       
};

#endif // _ADMITTANCE_H_

