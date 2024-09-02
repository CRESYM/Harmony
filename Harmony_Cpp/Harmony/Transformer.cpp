#include "Transformer.h"
#include <iostream>

using namespace SymEngine;


void Transformer::compute_y_parameters_transformer(double R_p, double X_p, double R_s, double X_s, double a)
{
    RCP<const Basic> R_p_val = real_double(R_p);
    RCP<const Basic> X_p_val = real_double(X_p);

    RCP<const Basic> j = I; // Built-in constant for imaginary unit

    RCP<const Basic> Y_p = div(real_double(1), add(R_p_val, mul(j, X_p_val)));

    RCP<const Basic> R_s_val = real_double(R_s);
    RCP<const Basic> X_s_val = real_double(X_s);
    RCP<const Basic> Y_s = div(real_double(1), add(R_s_val, mul(j, X_s_val)));

    RCP<const Basic> Y_param1 = Y_p;
    RCP<const Basic> Y_param2 = mul(real_double(-a), Y_p);
    RCP<const Basic> Y_param3 = div(real_double(-1), mul(real_double(a), add(R_s_val, mul(j, X_s_val))));
    RCP<const Basic> Y_param4 = Y_s;

    RCP<const Basic> Y_param1_eval = evalf(*Y_param1, 53);
    RCP<const Basic> Y_param2_eval = evalf(*Y_param2, 53);
    RCP<const Basic> Y_param3_eval = evalf(*Y_param3, 53);
    RCP<const Basic> Y_param4_eval = evalf(*Y_param4, 53);

    double Y_param1_abs = rcp_static_cast<const RealDouble>(Y_param1_eval)->as_double();
    double Y_param2_abs = rcp_static_cast<const RealDouble>(Y_param2_eval)->as_double();
    double Y_param3_abs = rcp_static_cast<const RealDouble>(Y_param3_eval)->as_double();
    double Y_param4_abs = rcp_static_cast<const RealDouble>(Y_param4_eval)->as_double();

    std::cout << "|Transformer Y_param1|: " << Y_param1_abs << " S" << std::endl;
    std::cout << "|Transformer Y_param2|: " << Y_param2_abs << " S" << std::endl;
    std::cout << "|Transformer Y_param3|: " << Y_param3_abs << " S" << std::endl;
    std::cout << "|Transformer Y_param4|: " << Y_param4_abs << " S" << std::endl;
}
