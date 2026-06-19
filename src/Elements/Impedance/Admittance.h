#ifndef _ADMITTANCE_H_
#define _ADMITTANCE_H_

/**
 * @file Admittance.h
 * @brief Generic multi-phase admittance element with symbolic or numeric values.
 */

#include "Impedance_base.h"

/**
 * @class Admittance
 * @brief Generic admittance matrix element for single- or multi-phase networks.
 * @ingroup impedance
 *
 * Admittance values may be numerical or symbolic (e.g. s-2). For multi-pin elements,
 * the values vector may contain one diagonal entry, `pins` diagonal entries, or a
 * full `pins × pins` admittance matrix.
 */
class Admittance : public Impedance_base {
public:
    /**
     * @brief Construct an admittance element from a dense value matrix.
     * @param symbol Element identifier (e.g. Y1, Y2).
     * @param location Network area or location string.
     * @param pins Number of input/output pins (phases).
     * @param values DenseMatrix of admittance values (numerical or symbolic).
     */
    Admittance(const std::string& symbol, const std::string& location, int pins, DenseMatrix values);

    // Destructor to handle clean-up tasks
    ~Admittance() override;


private:
    // No additional private members; behavior is inherited from Element
    //DenseMatrix Y_matrix; // Admittance matrix
};

#endif // _ADMITTANCE_H_

