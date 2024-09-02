// Load.cpp

#include "Element.h"
#include "Load.h"
#include "Constants.h"
#include <iostream>

using namespace SymEngine;

void Load::compute_y_parameters_rlc(double R, double L, double C, double frequency)
{
    //RCP<const Basic> pi = real_double(3.141592653589793);
    //RCP<const Basic> omega = mul(real_double(2), mul(pi, real_double(frequency)));

    RCP<const Basic> omega = mul(real_double(2), mul(PI, real_double(frequency)));


    RCP<const Basic> R_val = real_double(R);
    RCP<const Basic> L_val = real_double(L);
    RCP<const Basic> C_val = real_double(C);
    RCP<const Basic> j = I;

    RCP<const Basic> Z_RLC = add(add(R_val, mul(j, mul(omega, L_val))), mul(j, div(real_double(-1), mul(omega, C_val))));

    RCP<const Basic> Y_RLC = div(real_double(1), Z_RLC);

    RCP<const Basic> Y_param1 = Y_RLC;
    RCP<const Basic> Y_param2 = real_double(0);
    RCP<const Basic> Y_param3 = real_double(0);
    RCP<const Basic> Y_param4 = real_double(0);

    RCP<const Number> Y_param1_eval = rcp_static_cast<const Number>(evalf(*Y_param1, 53));
    RCP<const Number> Y_param2_eval = rcp_static_cast<const Number>(evalf(*Y_param2, 53));
    RCP<const Number> Y_param3_eval = rcp_static_cast<const Number>(evalf(*Y_param3, 53));
    RCP<const Number> Y_param4_eval = rcp_static_cast<const Number>(evalf(*Y_param4, 53));

    RCP<const Basic> Y_param1_abs = abs(Y_param1_eval);

    double Y_param1_abs_val = eval_double(*Y_param1_abs);

    double Y_param2_abs = 0.0;
    double Y_param3_abs = 0.0;
    double Y_param4_abs = 0.0;

    std::cout << "|Load Y_param1|: " << Y_param1_abs_val << " S" << std::endl;
    std::cout << "|Load Y_param2|: " << Y_param2_abs << " S" << std::endl;
    std::cout << "|Load Y_param3|: " << Y_param3_abs << " S" << std::endl;
    std::cout << "|Load Y_param4|: " << Y_param4_abs << " S" << std::endl;
}
