#include "Capacitor.h"


// Frequency-domain constructor Supports both single-phase and multi-phase
Capacitor::Capacitor(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& capacitance)
    : Element(symbol, location, pins, pins), C(capacitance)
{
    // Single-phase case: 2x2 admittance matrix
    if (pins == 1) {
        RCP<const Basic> sC = mul(real_double(C[0]), s);  // s * C
        Y_matrix.set(0, 0, sC);
        Y_matrix.set(0, 1, mul(integer(-1), sC));
        Y_matrix.set(1, 0, mul(integer(-1), sC));
        Y_matrix.set(1, 1, sC);
    }
    else {
        if (C.size() == 1) {
            for (int i = 1; i < pins; ++i) {
                C.push_back(C[0]);  // Fill with the same capacitance value
			}
        }
        // Multi-phase case: block diagonal admittance matrix
        for (int i = 0; i < pins; ++i) {
            if (C[i] <= 0.0) {
                throw std::invalid_argument("Capacitance must be positive.");
			}
            RCP<const Basic> sC = mul(real_double(C[i]), s);  // s * C
            int a = 2 * i;
            int b = 2 * i + 1;
            Y_matrix.set(a, a, sC);                       // Y(i,i)
            Y_matrix.set(a, b, mul(integer(-1), sC));     // Y(i,j)
            Y_matrix.set(b, a, mul(integer(-1), sC));     // Y(j,i)
            Y_matrix.set(b, b, sC);                       // Y(j,j)
        }
    }
}


Capacitor::~Capacitor()
{
}

void Capacitor::writeMNAmatrix(SymEngine::DenseMatrix& matrix, std::unordered_map<Bus*, int>& bus_indices, int location, 
    std::map<Element*, std::vector<RCP<const Basic>>>& symbol_map)
{
    Bus* node1 = nullptr;
    Bus* node2 = nullptr;
    for (auto& [bus, index] : connections) {
        if (index == 1) {
            node1 = bus;  // First bus connected to the element
        }
        else if (index == 2) {
            node2 = bus;  // Second bus connected to the element
		}
	}
    
	std::vector<RCP<const Basic>> symbols;

    for (int p = 0; p < input_pins; ++p) {
        int row = location + p;   // branch current row
     
        RCP<const Basic> vc_sym = symbol("Vc_" + getElementSymbol() + std::to_string(p));
		symbols.push_back(vc_sym);

        matrix.set(row, matrix.ncols()-1, vc_sym);
        if (node1 && (bus_indices.count(node1) != 0)) { 
            int r = bus_indices[node1]+p;  
            RCP<const Basic> Csym = real_double(C[p]);
            matrix.set(row, r, one);  
            matrix.set(r, row, Csym);  
        }
        if (node2 && (bus_indices.count(node2) != 0)) { 
            int r = bus_indices[node2]+p;  
            RCP<const Basic> Csym = real_double(C[p]);
            matrix.set(row, r, minus_one);
            matrix.set(r, row, mul(minus_one, Csym));
        }
    }
	symbol_map[this] = symbols;  // Store the symbols for this element
}

// Print basic capacitor info
void Capacitor::printElementValues() {
    std::cout << "Capacitor symbol: " << getElementSymbol() << std::endl;
    std::cout << "Initial voltage: " << initial_value << " V" << std::endl;
    std::cout << "Y matrix (symbolic):\n" << Y_matrix.__str__() << std::endl;
}

