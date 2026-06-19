/**
 * @file DC_source.h
 * @brief Ideal DC voltage source with optional series impedance.
 * @ingroup source
 */

#ifndef _DC_SOURCE_H_
#define _DC_SOURCE_H_

#include "Source_base.h"

/**
 * @class DC_source
 * @brief DC-side voltage source for hybrid AC-DC networks.
 *
 * Supports scalar or per-phase voltage with scalar or vector series impedance,
 * analogous to @ref AC_source on the AC side.
 */
class DC_source : public Source_base {
public:
    /** @brief Scalar voltage and series resistance. */
    DC_source(const std::string& symbol, const std::string& location, int pins, double V, double R);

    /** @brief Per-conductor voltages with common series resistance. */
    DC_source(const std::string& symbol, const std::string& location, int pins, const vector<double>& V, double R);

    /** @brief Scalar voltage with per-conductor impedance vector. */
    DC_source(const std::string& symbol, const std::string& location, int pins, double V, const vector<double>& Z);

    /** @brief Per-conductor voltage and impedance vectors. */
    DC_source(const std::string& symbol, const std::string& location, int pins, const vector<double>& V, const vector<double>& Z);

    ~DC_source() {}

    /** @brief Stamp the source into the symbolic MNA matrix. */
    void writeMNAmatrix(SymEngine::DenseMatrix&, std::unordered_map<Bus*, int>&, int, std::map<Element*, std::vector<RCP<const Basic>>>&) override;

private:
};

#endif
