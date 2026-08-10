#ifndef AC_SOURCE_H
#define AC_SOURCE_H

/**
 * @file AC_source.h
 * @brief Ideal AC voltage source with series internal impedance.
 */

#include "Source_base.h"

/**
 * @class AC_source
 * @brief Ideal AC voltage source with added series impedance.
 * @ingroup source
 *
 * Supports single- or multi-phase configuration. Power-flow data (voltage magnitude,
 * phase shift, P/Q limits) is supplied after construction.
 */
class AC_source : public Source_base {
public:
    /**
     * @brief Construct an AC source with a dense impedance matrix.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases).
     * @param V Voltage magnitude.
     * @param Z Series impedance matrix.
     */
    AC_source(const std::string& symbol, const std::string& location, int pins, double V, DenseMatrix Z);
    /**
     * @brief Construct an AC source with a vector of per-phase impedances.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases).
     * @param V Voltage magnitude.
     * @param Z Series impedance values in ohms.
     */
	AC_source(const std::string& symbol, const std::string& location, int pins, double V, const std::vector<double>& Z);
    /**
     * @brief Construct an AC source with a scalar series impedance.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases).
     * @param V Voltage magnitude.
     * @param Z Series impedance in ohms.
     */
	AC_source(const std::string& symbol, const std::string& location, int pins, double V, const double Z);

    // Destructor
    ~AC_source();

    void writeMNAmatrix(SymEngine::DenseMatrix&, std::unordered_map<Bus*, int>&, int, std::map<Element*, std::vector<RCP<const Basic>>>&) override;

    std::vector<MatrixXcd> simulateInputStep(
        const std::vector<MatrixXcd>& states, int nKeep) const override;

    // Function to print AC source values
    void printElementValues() override;

private:
    // Properties

};

#endif

