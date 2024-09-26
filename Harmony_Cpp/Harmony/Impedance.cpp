#include "Impedance.h"
#include "Element.h"
#include "Constants.h"

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
void Impedance::compute_y_parameters(double frequency) {
    // Compute angular frequency
    double omega = 2 * M_PI * frequency;

    // Assuming Z_matrix holds the impedance for a 3-phase system
    std::vector<std::vector<double>> Y_matrix(3, std::vector<double>(3, 0));

    // Inverse of the impedance matrix will give the admittance matrix (Y = Z^-1)
    // This requires matrix inversion (for simplicity, using a numerical library would be helpful)

    // TODO: Implement matrix inversion (you can use Eigen library for this)

    // For now, assuming diagonal matrix (impedance only between the same phases):
    for (int i = 0; i < 3; i++) {
        Y_matrix[i][i] = 1 / Z_matrix[i][i];  // Y = 1/Z
    }

    // Output the computed Y-parameters for each phase
    for (int i = 0; i < 3; i++) {
        std::cout << "|Y_matrix[" << (i + 1) << "," << (i + 1) << "]|: " << Y_matrix[i][i] << " S" << std::endl;
    }
}