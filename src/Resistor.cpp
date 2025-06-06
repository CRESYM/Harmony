#include "Resistor.h"

Resistor::Resistor(const std::string& symbol, int pins, double R)
    : Element(symbol, pins, pins), R_values(pins, R)
{
    if (pins <= 0) {
        throw std::invalid_argument("Invalid number of pins. Must be greater than 0.");
    }

    if (R <= 0.0) {
        throw std::invalid_argument("Resistance must be positive.");
    }

    Y_matrix.assign(2 * pins, std::vector<double>(2 * pins, 0.0));

    double y = 1.0 / R;
    for (int i = 0; i < pins; ++i) {
        Y_matrix[i][i] = y;
    }

    // Populate full Y matrix
    for (int i = 0; i < pins; ++i) {
        for (int j = 0; j < pins; ++j) {
            double val = Y_matrix[i][j];
            Y_matrix[pins + i][j] = -val;
            Y_matrix[i][pins + j] = -val;
            Y_matrix[pins + i][pins + j] = val;
        }
    }
}

Resistor::Resistor(const std::string& symbol, int pins, const std::vector<double>& resistances)
    : Element(symbol, pins, pins), R_values(resistances)
{
    {
        if (pins <= 0) {
            throw std::invalid_argument("Invalid number of pins. Must be greater than 0.");
        }

        if (resistances.empty()) {
            throw std::invalid_argument("Resistance vector is empty.");
        }

        Y_matrix.assign(2 * pins, std::vector<double>(2 * pins, 0.0));

        if (resistances.size() == 1) {
            double r = resistances[0];
            if (r <= 0.0) throw std::invalid_argument("Resistance must be positive.");
            for (int i = 0; i < pins; ++i) {
                Y_matrix[i][i] = 1.0 / r;
            }
        }
        else if (resistances.size() == static_cast<size_t>(pins)) {
            for (int i = 0; i < pins; ++i) {
                double r = resistances[i];
                if (r <= 0.0) throw std::invalid_argument("Resistance must be positive.");
                Y_matrix[i][i] = 1.0 / r;
            }
        }
        else {
            throw std::invalid_argument("Mismatch between number of pins and resistance values.");
        }

        // Populate full Y matrix
        for (int i = 0; i < pins; ++i) {
            for (int j = 0; j < pins; ++j) {
                double y = Y_matrix[i][j];
                Y_matrix[pins + i][j] = -y;
                Y_matrix[i][pins + j] = -y;
                Y_matrix[pins + i][pins + j] = y;
            }
        }
    }
}
void Resistor::printElementValues() {
    std::cout << "Resistor values (Ohms): ";
    for (double r : R_values) {
        std::cout << r << " ";
    }
    std::cout << "\nAdmittance matrix (Y):\n";
    for (const auto& row : Y_matrix) {
        for (double val : row) {
            std::cout << val << "\t";
        }
        std::cout << "\n";
    }
}

Resistor::~Resistor()
{
}
