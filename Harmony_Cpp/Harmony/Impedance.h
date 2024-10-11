#ifndef _IMPEDANCE_H_
#define _IMPEDANCE_H_

#include "Element.h"

/*
Creates impedance with specified number of input/output pins which represent phases.
The admittance expression has to be given in Omega and can have both numerical and
symbolic value (example: `z = s-2`). Depending on the provided vector of impedance values,
we differ two cases. Namely, we create only series impedance for each phase with values:
-In the case of 1×1 vector, impedance value is given to all diagonal impedance entries.
-In the case of `pins` elements, they are representing diagonal entries of impedance.
*/


class Impedance : public Element {
public:
    /*
     * Constructor: Impedance
     *
     * Constructs the impedance model with the given symbolic name, number of pins (phases),
     * and a matrix of impedance values. It calculates the admittance matrix based on these values.
     *
     * Parameters:
     * - symbol: Symbolic identifier for the impedance element (e.g., Z1, Z2)
     * - pins: Number of input/output pins (phases)
     * - values: DenseMatrix representing the impedance values
     */
    Impedance(const std::string& symbol, int pins, DenseMatrix values);

    // Destructor to handle any clean-up tasks
    ~Impedance();

private:
    // No additional private members in this class, as the behavior is inherited from Element
};

#endif // _IMPEDANCE_H_


