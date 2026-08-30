#ifndef TRANSFORMER_REAL_H
#define TRANSFORMER_REAL_H

/**
 * @file Transformer_real.h
 * @brief Real transformer model with turns ratio and phase shift.
 */

#include "Transformer_base.h"

/**
 * @class Transformer_real
 * @brief Real transformer with turns ratio and phase shift (paper eq. (2)).
 * @ingroup transformer
 */
class Transformer_real : public Transformer_base {
public:
    /**
     * @brief Construct a real transformer model.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases) per winding.
     * @param values Six values `{R_p, L_p, R_s, L_s, a, φ}` or eight
     *        `{R_p, L_p, R_s, L_s, R_m, L_m, a, φ}`. φ is the paper eq. (1)
     *        phase shift in radians: Vp/Vs = a exp(jφ).
     */
    Transformer_real(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values);

    ~Transformer_real() override; 

    double getTurnsRatio() const { return a; }

    /** @brief Paper eq. (1) phase shift φ [rad]. Alias of getPhaseShift(). */
    double getPhaseLag() const { return getPhaseShift(); }

private:
    double a = 0.0;  // Turns ratio NP/NS
};

#endif // TRANSFORMER_REAL_H
