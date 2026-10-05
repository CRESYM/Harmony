/**
 * @file Transformer_base.cpp
 * @brief Implementation of Base class for transformer models with winding R-L parameters.
 */
#include "Transformer_base.h"

Transformer_base::Transformer_base(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values)
	: Element(symbol, location, pins, pins) {
    m_pins = pins;
}

// Destructor
Transformer_base::~Transformer_base() = default;

void Transformer_base::applyWindingConnection(bool deltaPrimary, bool deltaSecondary) {
    if (m_pins != 3) {
        throw std::invalid_argument("Invalid number of pins. It must be 3!");
    }

    // Paper eqs. (4)–(5): Ti and Tv, each scaled by 1/√3.
    auto Ti = DenseMatrix(3, 3, {
        integer(1), zero, integer(-1),
        integer(-1), integer(1), zero,
        zero, integer(-1), integer(1)
    });
    mul_dense_scalar(Ti, real_double(1.0 / sqrt(3.0)), Ti);
    auto Tv = DenseMatrix(3, 3, {
        one, minus_one, zero,
        zero, one, minus_one,
        minus_one, zero, one
    });
    mul_dense_scalar(Tv, real_double(1.0 / sqrt(3.0)), Tv);

    auto N1 = createZeroMatrix(6, 6);
    auto N2 = createZeroMatrix(6, 6);
    for (int i = 0; i < 3; i++) {
        if (!deltaPrimary) {
            N1.set(i, i, integer(1));
            N2.set(i, i, integer(1));
        }
        if (!deltaSecondary) {
            N1.set(3 + i, 3 + i, integer(1));
            N2.set(3 + i, 3 + i, integer(1));
        }
        for (int j = 0; j < 3; j++) {
            if (deltaPrimary) {
                N1.set(i, j, Ti.get(i, j));
                N2.set(i, j, Tv.get(i, j));
            }
            if (deltaSecondary) {
                N1.set(3 + i, 3 + j, Ti.get(i, j));
                N2.set(3 + i, 3 + j, Tv.get(i, j));
            }
        }
    }

    DenseMatrix tmp = createZeroMatrix(6, 6);
    mul_dense_dense(N1, Y_matrix, tmp);
    mul_dense_dense(tmp, N2, Y_matrix);
}

void Transformer_base::computePowerFlow(std::map<std::string, double>& branchData,
    std::map<std::string, double>& globalParams) const
{
    using cd = std::complex<double>;

	string area = element_location.substr(0, 2); // Extract area code from element_location

    if ((area[0] == 'D' || area[0] == 'd') && (area[1] == 'C' || area[1] == 'c')) { // DC network
        cd Y12 = substitute_symbol(Y_matrix.get(0, m_pins), omega, 0.0);

        cd zs = -cd(1.0) / Y12 / globalParams.at("ACZbase");

        branchData["r"] = std::real(zs);
        branchData["x"] = 0.0;
        branchData["b"] = 0.0;
        branchData["grid"] = (int)element_location[2] - '0'; // Example of setting grid based on element_location

        for (auto& [key, value] : element_OPF_info)
            branchData[key] = value;
    }
    else if ((area[0] == 'A' || area[0] == 'a') && (area[1] == 'c' || area[1] == 'C')) { // AC network
        cd Y11 = substitute_symbol(Y_matrix.get(0, 0), omega, globalParams.at("omega"));
        cd Y12 = substitute_symbol(Y_matrix.get(0, m_pins), omega, globalParams.at("omega"));
        cd Y22 = substitute_symbol(Y_matrix.get(m_pins, m_pins), omega, globalParams.at("omega"));

        double tap = std::sqrt(std::real(Y22 / Y11));
        cd Ys = -Y12 * tap;
        cd Yc = Y22 - Ys;

        cd Zs = cd(1.0) / Ys / globalParams.at("ACZbase");

        branchData["transformer"] = 1;
        branchData["tap"] = tap;
        branchData["shift"] = phase_shift;

        branchData["r"] = std::real(Zs);
        branchData["x"] = std::imag(Zs);

        branchData["g_fr"] = branchData["g_to"] = std::real(Yc);
        branchData["b_fr"] = branchData["b_to"] = std::imag(Yc);

        branchData["c_rating_a"] = 1.0;
        branchData["grid"] = (int)element_location[2] - '0'; // Example of setting grid based on element_location

        for (auto& [key, value] : element_OPF_info)
            branchData[key] = value;
    }
    else {
        throw std::runtime_error("Invalid network type specified in element_location.");
	}  
}

