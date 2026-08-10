/**
 * @file Impedance.h
 * @brief Generic multi-phase series impedance element with symbolic or numeric Z values.
 * @ingroup impedance
 */

#ifndef _IMPEDANCE_H_
#define _IMPEDANCE_H_

#include "Impedance_base.h"

/**
 * @class Impedance
 * @brief Series impedance for each phase with diagonal (or matrix) Z entries.
 *
 * Accepts SymEngine @c DenseMatrix, per-phase vectors, scalars, or complex values.
 * Used for AC branches (R+jX) and DC resistive links.
 */
class Impedance : public Impedance_base {
public:
    /**
     * @brief Construct from a symbolic dense impedance matrix.
     * @param symbol Element designator.
     * @param location Grid area tag (e.g. @c AC1, @c DC1).
     * @param pins Number of phases / conductors.
     * @param values SymEngine matrix of impedance entries.
     */
    Impedance(const std::string& symbol, const std::string& location, int pins, DenseMatrix values);

    /**
     * @brief Construct from per-phase real impedance values.
     * @param values One value (all diagonals) or @p pins diagonal entries.
     */
    Impedance(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values);

    /** @brief Single scalar impedance applied to all diagonal entries. */
    Impedance(const std::string& symbol, const std::string& location, int pins, const double values);

    /** @brief Per-phase complex impedance vector. */
    Impedance(const std::string& symbol, const std::string& location, int pins, const std::vector<complex<double>>& values);

    /** @brief Single complex impedance for all diagonal entries. */
    Impedance(const std::string& symbol, const std::string& location, int pins, const complex<double> values);

    ~Impedance();

private:
};

#endif // _IMPEDANCE_H_
