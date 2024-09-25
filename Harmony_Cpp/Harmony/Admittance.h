#ifndef _ADMITTANCE_H_
#define _ADMITTANCE_H_

#include "Element.h"
#include <vector> // Include vector header


using namespace SymEngine; // Use the SymEngine namespace

/*
Creates admittance with specified number of input/output pins `pins`. The admittance expression
 `exp` has to be given in Ω and can have both numerical and symbolic value (example: `z = s-2`).
Depending on the provided vector of admittance values, we differ three cases. If the number of 
element pins is greater than 1, admittance can be represented with vector with one, `pins` or 
`pins × pins` number of elements. Namely
-In the case of 1×1 vector, admittance has only one value. Then this value is given to all diagonal
admittance entries.
-In the case of `pins` elements, they are representing diagonal entries of admittance.
-In case of `pins x pins` elements, they are representing all matrix of admittance.
*/

class Admittance : public Element {
public:
    // Constructor for Admittance class
    //Admittance(int pins, const std::vector<RCP<Symbol>>& admittance); // Pass by reference for efficiency
    Admittance(const std::string& symbol, int pins, std::vector<RCP<Symbol>> admittanceValues);

    ~Admittance();

private:
    RCP<Symbol>** admittance; // 2D array for admittance matrix (if needed)
};

#endif // _ADMITTANCE_H_

