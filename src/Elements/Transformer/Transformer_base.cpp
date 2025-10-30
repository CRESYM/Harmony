#include "Transformer_base.h"

Transformer_base::Transformer_base(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values)
	: Element(symbol, location, pins, pins) {
    m_pins = pins;
}

// Destructor
Transformer_base::~Transformer_base() {
    std::cout << "Transformer object for " << getElementSymbol() << " destroyed." << std::endl;
}

void Transformer_base::computePowerFlow(std::map<std::string, double>& branchData,
    const std::map<std::string, double>& globalParams) const
{
    using cd = std::complex<double>;

	string area = element_location.substr(0, 2); // Extract area code from element_location

    if ((area[0] == 'D' || area[0] == 'd') && (area[1] == 'C' || area[1] == 'c')) { // DC network
        cd Y12 = substitute_symbol(Y_matrix.get(0, m_pins), omega, 0.0);

        cd zs = -cd(1.0) / Y12 / globalParams.at("Z_base");

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

        cd Zs = cd(1.0) / Ys / globalParams.at("Z_base");

        branchData["transformer"] = 1;
        branchData["tap"] = tap;
        branchData["shift"] = 0;

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

