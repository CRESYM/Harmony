#include "AC_source.h"

AC_source::AC_source(const std::string& symbol, int pins, DenseMatrix Z)
	: Element(symbol, pins, pins)
{
    if (Z.ncols() != 0)  // if there are entries
    {
        if (pins > 0) { // Check for valid number of pins
            if (Z.ncols() == 1) {
                for (int i = 0; i < pins; i++)
                    Y_matrix.set(i, i, div(integer(1), Z.get(0, 0)));
            }
            else if (Z.ncols() == pins) {
                for (int i = 0; i < pins; i++)
                    Y_matrix.set(i, i, div(integer(1), Z.get(0, 0)));
            }
            else
                throw invalid_argument("Invalid number of series impedance vector entries: " + Z.ncols());
        }
    }
    else
        throw invalid_argument("Invalid number of pins, must be greater than 0!");

    // Fill in the complete Y parameters
    for (int i = 0; i < pins; i++)
        for (int j = 0; j < pins; j++) {
            Y_matrix.set(pins + i, j, sub(zero, Y_matrix.get(i, j)));
            Y_matrix.set(pins + i, pins + j, Y_matrix.get(i, j));
            Y_matrix.set(i, pins + j, sub(zero, Y_matrix.get(i, j)));
        }
}

// Destructor
AC_source::~AC_source() {
    // No need for manual memory management for DenseMatrix or other standard library components
    std::cout << "AC source object for " << getElementSymbol() << " destroyed." << std::endl;
}

// void AC_source::writeMatrixSymbolic(DenseMatrix & mat,
    //    const std::unordered_map<Bus*, int>& busIndex)
    //{
    //    std::cout << "Called for AC source '" << getElementSymbol() << "'\n";
    //
    //    if (connections.size() != 2) {
    //        std::cerr << "ERROR: AC_source should be connected to exactly two buses.\n";
    //        return;
    //    }
    //
    //    auto it = connections.begin();
    //    Bus* busA = it->first;
    //    ++it;
    //    Bus* busB = it->first;
    //
    //    std::cout << "  Connected buses: " << busA->getBusName() << ", " << busB->getBusName() << "\n";
    //
    //    auto itA = busIndex.find(busA);
    //    auto itB = busIndex.find(busB);
    //
    //    if (itA == busIndex.end()) {
    //        std::cerr << "ERROR: busA '" << busA->getBusName() << "' not found in busIndex\n";
    //        return;
    //    }
    //    if (itB == busIndex.end()) {
    //        std::cerr << " ERROR: busB '" << busB->getBusName() << "' not found in busIndex\n";
    //        return;
    //    }
    //
    //    int i = itA->second;
    //    int j = itB->second;
    //
    //    if (Z.nrows() != 1 || Z.ncols() != 1) {
    //        std::cerr << " ERROR: Z must be a 1x1 matrix.\n";
    //        return;
    //    }
    //
    //    RCP<const Basic> Zval = Z.get(0, 0);
    //    std::cout << "  Impedance Z = " << *Zval << "\n";
    //
    //    RCP<const Basic> Y_val = div(integer(1), Zval);  // Y = 1/Z
    //    std::cout << "  Calculated admittance Y = " << *Y_val << "\n";
    //
    //    std::cout << "  Setting matrix entries:\n";
    //    std::cout << "    mat(" << i << "," << i << ") += Y\n";
    //    std::cout << "    mat(" << j << "," << j << ") += Y\n";
    //    std::cout << "    mat(" << i << "," << j << ") -= Y\n";
    //    std::cout << "    mat(" << j << "," << i << ") -= Y\n";
    //
    //    mat.set(i, i, add(mat.get(i, i), Y_val));
    //    mat.set(j, j, add(mat.get(j, j), Y_val));
    //    mat.set(i, j, sub(mat.get(i, j), Y_val));
    //    mat.set(j, i, sub(mat.get(j, i), Y_val));
    //
    //    std::cout << "Finished for AC source '" << getElementSymbol() << "'\n";
    //}

void AC_source::writeMNAmatrixNumeric(Eigen::MatrixXd& A, Eigen::MatrixXd& E, Eigen::MatrixXd& B,
    int num_equations,
    int index,
    const std::unordered_map<Bus*, int>& busIndex,
    const std::unordered_map<Element*, int>& currentSourceIndex,
    const std::unordered_map<Element*, int>& stateVarIndex)
{
    if (connections.size() != 2) {
        throw std::runtime_error("AC_source should be connected to exactly two buses.");
    }

    Bus* n1 = nullptr; // For terminal 0 (positive)
    Bus* n2 = nullptr; // For terminal 1 (negative)

    for (const auto& pair : connections) {
        Bus* bus_ptr = pair.first;
        int terminal_id = pair.second;
        if (terminal_id == 0) n1 = bus_ptr;
        else if (terminal_id == 1) n2 = bus_ptr;
    }

    // Find  AC source's current source row index
    int row = -1;
    auto it_cs = currentSourceIndex.find(this);
    if (it_cs != currentSourceIndex.end())
        row = it_cs->second;
    else {
        std::cerr << "[WARNING] AC_source current source index not found\n";
        return;
    }

    // Get bus indices
    int n1_idx = (n1 && busIndex.count(n1)) ? busIndex.at(n1) : -1;
    int n2_idx = (n2 && busIndex.count(n2)) ? busIndex.at(n2) : -1;

    // Stamping into A matrix 
    if (n1_idx != -1) {
        A(row, n1_idx) += 1.0;  // V_n1 term in V_n1 - V_n2 = V_source
        A(n1_idx, row) += 1.0;  // KCL at n1: I_source flows out
    }
    if (n2_idx != -1) {
        A(row, n2_idx) += -1.0; // -V_n2 term in V_n1 - V_n2 = V_source
        A(n2_idx, row) += -1.0; // KCL at n2: I_source flows in
    }

    // Stamping into B matrix 
    if (Z.nrows() == 1 && Z.ncols() == 1) {
        SymEngine::RCP<const SymEngine::Basic> expr = Z.get(0, 0);

        if (SymEngine::is_a<SymEngine::RealDouble>(*expr)) {

            double source_value = rcp_static_cast<const RealDouble>(expr)->as_double();
            B(row, 0) += source_value;
        }
        else if (SymEngine::is_a<SymEngine::Integer>(*expr)) {
       
            double source_value = static_cast<double>(rcp_static_cast<const Integer>(expr)->as_int());
            B(row, 0) += source_value;
        }
        else {
            std::cerr << "AC_source Z element (0,0) is not a numerical type (RealDouble or Integer). Cannot stamp value to B_mat.\n";
        }
    }
    else {
        std::cerr << "AC_source Z is not a 1x1 matrix. Cannot stamp source value.\n";
    }
}
void AC_source::printElementValues() {
	printElementInfo();

	//std::cout << "AC source has " << input_pins << " phases and it is represented with AC source with " <<
	//	V << " magnitude and " << theta << " phase and series impedance " << Z << std::endl;
}

