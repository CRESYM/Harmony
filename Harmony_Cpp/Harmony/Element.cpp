// element.cpp

#include "element.h"
//#include "Load.h"
//#include "Generator.h"
#include "Impedance.h"

// Destructor
Element::~Element() {}

// Implementation of compute_y_parameters
void Element::compute_y_parameters(double frequency) {
    std::cout << "This should be overridden in derived classes." << std::endl;
}


// Implementation of compute_y_parameters for Impedance
void Impedance::compute_y_parameters(double frequency) {

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
}