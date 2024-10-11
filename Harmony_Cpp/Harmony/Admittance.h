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
    /*
     * Constructor: Admittance
     *
     * Constructs the admittance model with the specified symbolic name, number of pins (phases),
     * and a matrix of admittance values. The values can represent single-phase or multi-phase
     * admittance depending on the input size.
     *
     * Parameters:
     * - symbol: Symbolic identifier for the admittance element (e.g., Y1, Y2)
     * - pins: Number of input/output pins (phases)
     * - values: DenseMatrix representing the admittance values (numerical or symbolic)
     */

    // Unified constructor for single-phase and three-phase systems
    Admittance(const std::string& symbol, int pins, DenseMatrix values);

    // Destructor to handle clean-up tasks
    ~Admittance() override;

private:
    // No additional private members; behavior is inherited from Element
};

#endif // _ADMITTANCE_H_

