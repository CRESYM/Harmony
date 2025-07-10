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

    void AC_source::writeMNAmatrixNumeric(Eigen::MatrixXd & A,
        int num_equations,
        int index,
        const std::unordered_map<Bus*, int>&busIndex)
{
    if (connections.size() != 2) {
        throw std::runtime_error("AC_source should be connected to exactly two buses.");
    }

    auto it = connections.begin();
    Bus* n1 = it->first;
    ++it;
    Bus* n2 = it->first;

    int row = index; // Branch current equation row index

    if (n1) {
        int r = busIndex.at(n1);
        A(row, r) += 1.0;    // KCL at positive terminal
        A(r, row) += 1.0;    
    }
    if (n2) {
        int r = busIndex.at(n2);
        A(row, r) += -1.0;   // KCL at negative terminal
        A(r, row) += -1.0;
    }
}

void AC_source::printElementValues() {
	printElementInfo();

	//std::cout << "AC source has " << input_pins << " phases and it is represented with AC source with " <<
	//	V << " magnitude and " << theta << " phase and series impedance " << Z << std::endl;
}

