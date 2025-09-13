#include "Resistor.h"


Resistor::Resistor(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& R)
    : Element(symbol, location, pins, pins)
{
    if (pins <= 0) {
        throw std::invalid_argument("Invalid number of pins. Must be greater than 0.");
    }

    if (R.empty()) {
        throw std::invalid_argument("Resistance vector is empty.");
    }
    
    if (R.size() != 1 && R.size() != static_cast<size_t>(pins)) {
        throw std::invalid_argument("Mismatch between number of pins and resistance values.");
    }

    if (R.size() == 1) {
        if (R[0] <= 0.0)
            throw std::invalid_argument("Resistance must be positive.");
        R_values = std::vector<double>(pins, R[0]);
    }
    else {
        // Validate all resistances
        for (double r : R) {
            if (r <= 0.0)
                throw std::invalid_argument("All resistance values must be positive.");
        }
        R_values = R;
    }
    // Diagonal entries for positive nodes
    for (int i = 0; i < pins; ++i) {
        Y_matrix.set(i, i, real_double(1.0 / R_values[i]));
    }

    // Fill in the complete Y parameters
    for (int i = 0; i < pins; i++)
        for (int j = 0; j < pins; j++) {
            Y_matrix.set(pins + i, j, sub(zero, Y_matrix.get(i, j)));
            Y_matrix.set(pins + i, pins + j, Y_matrix.get(i, j));
            Y_matrix.set(i, pins + j, sub(zero, Y_matrix.get(i, j)));
        }
}



void Resistor::writeMNAmatrix(SymEngine::DenseMatrix& matrix, std::unordered_map<Bus*, int>& bus_indices, int location,
    std::map<Element*, std::vector<RCP<const Basic>>>& symbols_map)
{
    std::vector<Bus*> buses = getBuses();
    Bus* node1 = buses.size() > 0 ? buses[0] : nullptr;
    Bus* node2 = buses.size() > 1 ? buses[1] : nullptr;

    if (node1 && (bus_indices.count(node1) != 0)) {
        int n1 = bus_indices[node1];
        for (int i = 0; i < input_pins; ++i) {
            matrix.set(n1+i, n1+i, addSym(matrix.get(n1+i, n1+i), inv(real_double(R_values[i]))));
		}

        if (node2 && (bus_indices.count(node2) != 0)) {
			int n2 = bus_indices[node2];
            for (int i = 0; i < output_pins; ++i) {
                matrix.set(n1+i, n2+i, subSym(matrix.get(n1+i, n2+i), inv(real_double(R_values[i]))));
                matrix.set(n2+i, n1+i, subSym(matrix.get(n2+i, n1+i), inv(real_double(R_values[i]))));
				matrix.set(n2+i, n2+i, addSym(matrix.get(n2+i, n2+i), inv(real_double(R_values[i]))));
			}
        }
    }
    else if (node2 && (bus_indices.count(node2) != 0)) {
        int n2 = bus_indices[node2];
        for (int i = 0; i < output_pins; ++i) {
            matrix.set(n2+i, n2+i, addSym(matrix.get(n2+i, n2+i), inv(real_double(R_values[i]))));
        }
    }
}

void Resistor::printElementValues() {
    std::cout << "Resistor values (Ohms): ";
    for (double r : R_values) {
        std::cout << r << " ";
    }
    std::cout << "\nAdmittance matrix (Y):\n";
    // Fill in the complete Y parameters
    for (int i = 0; i < Y_matrix.nrows(); i++)
        for (int j = 0; j < Y_matrix.ncols(); j++) {
            std::cout << Y_matrix.get(i, j)->__str__();
        }
}

Resistor::~Resistor() {}
