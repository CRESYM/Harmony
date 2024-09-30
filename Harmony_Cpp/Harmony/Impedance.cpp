#include "Impedance.h"
#include "Element.h"
#include "Constants.h"

// Constructor for single-phase using a symbolic impedance value
/*Impedance::Impedance(const std::string& symbol, int inputPins, int outputPins, const RCP<const Basic>& impedanceValue)
    : Element(symbol, inputPins, outputPins), Z_matrix(1, 1) { // Initializing a 1x1 matrix for a single-phase
    Z_matrix.set(0, 0, impedanceValue); // Set the impedance value
}

// Constructor for three-phase using a 3x3 matrix for multi-phase systems
Impedance::Impedance(const std::string& symbol, int inputPins, int outputPins, const DenseMatrix& impedanceMatrix)
    : Element(symbol, inputPins, outputPins), Z_matrix(impedanceMatrix) { // Use the provided matrix directly
    if (impedanceMatrix.nrows() != 3 || impedanceMatrix.ncols() != 3) {
        throw std::invalid_argument("Impedance matrix must be 3x3 for three-phase systems.");
    }
}

// Constructor for vector-based values (single-phase or three-phase)
Impedance::Impedance(const std::string& symbol, int inputPins, int outputPins, const std::vector<std::vector<RCP<Symbol>>>& impedanceValues)
    : Element(symbol, inputPins, outputPins), Z_matrix(inputPins, outputPins) {
    for (int i = 0; i < inputPins; ++i) {
        for (int j = 0; j < outputPins; ++j) {
            Z_matrix.set(i, j, impedanceValues[i][j]); // Set values from the vector to the Z_matrix
        }
    }
}*/

// Destructor
Impedance::~Impedance() {
    // Clean-up if needed (Smart pointers handle most memory management)
}

// Implementation of compute_y_parameters for Single-phaseImpedance
//void Impedance::compute_y_parameters(double frequency) {

    /*// Compute angular frequency
    RCP<const Basic> omega = mul(real_double(2), mul(PI, real_double(frequency)));
    RCP<const Basic> j = I;

    // Calculate the impedance
    RCP<const Basic> Z = add(real_double(R), mul(j, real_double(X)));

    // Calculate the admittance
    RCP<const Basic> Y = div(real_double(1), Z);

    // Extract real and imaginary parts by evaluating the expression
    RCP<const Number> Y_eval = rcp_static_cast<const Number>(evalf(*Y, 53));
    double Y_val = eval_double(*Y_eval);

    // If the imaginary part is needed, you can compute it separately
    RCP<const Basic> Y_real = add(real_double(0), mul(j, real_double(0)));  // Replace with appropriate logic if needed

    // Print the results
    std::cout << "|Impedance Y|: " << Y_val << " S" << std::endl;
    std::cout << "|Impedance Y_real|: " << eval_double(*Y_real) << " S" << std::endl; // Adjust this line if necessary
    std::cout << "|Impedance Y_param3|: " << 0.0 << " S" << std::endl;
    std::cout << "|Impedance Y_param4|: " << 0.0 << " S" << std::endl;*/
    //}
    // 
//Three-phase
//void Impedance::compute_y_parameters(double frequency) {
//    // Compute angular frequency
//    //double omega = 2 * M_PI * frequency;
//
//    
//}
