//#include "Element.h"
//#include "Generator.h"
//#include "Constants.h"
//#include <iostream>

//using namespace SymEngine; 

/*void Generator::compute_y_parameters_generator(double R_f, double L_f, double X_d, double T_f, double frequency)
{
    //RCP<const Basic> pi = real_double(3.141592653589793);
    //RCP<const Basic> omega = mul(real_double(2), mul(pi, real_double(frequency)));

    // Use the constant PI from Constants.h
    RCP<const Basic> omega = mul(real_double(2), mul(PI, real_double(frequency)));

    RCP<const Basic> s = mul(I, omega);

    RCP<const Basic> R_f_val = real_double(R_f);
    RCP<const Basic> L_f_val = real_double(L_f);
    RCP<const Basic> Z_f = add(R_f_val, mul(s, L_f_val));

    RCP<const Basic> Y_f = div(real_double(1), Z_f);

    RCP<const Basic> X_d_val = real_double(X_d);
    RCP<const Basic> Z_d = mul(I, X_d_val);

    RCP<const Basic> T_f_val = real_double(T_f);
    RCP<const Basic> H_f = div(real_double(1), add(mul(T_f_val, s), real_double(1)));

    RCP<const Basic> Y11 = Y_f;
    RCP<const Basic> Y12 = neg(div(H_f, Z_d));
    RCP<const Basic> Y21 = real_double(0);
    RCP<const Basic> Y22 = neg(div(real_double(1), Z_d));

    RCP<const Number> Y11_eval = rcp_static_cast<const Number>(evalf(*Y11, 53));
    RCP<const Number> Y12_eval = rcp_static_cast<const Number>(evalf(*Y12, 53));
    RCP<const Number> Y21_eval = rcp_static_cast<const Number>(evalf(*Y21, 53));
    RCP<const Number> Y22_eval = rcp_static_cast<const Number>(evalf(*Y22, 53));

    RCP<const Basic> Y11_abs = abs(Y11_eval);
    RCP<const Basic> Y12_abs = abs(Y12_eval);
    RCP<const Basic> Y21_abs = abs(Y21_eval);
    RCP<const Basic> Y22_abs = abs(Y22_eval);

    double Y11_abs_val = eval_double(*Y11_abs);
    double Y12_abs_val = eval_double(*Y12_abs);
    double Y21_abs_val = eval_double(*Y21_abs);
    double Y22_abs_val = eval_double(*Y22_abs);
 
    std::cout << "|Generator Y11|: " << Y11_abs_val << " S" << std::endl;
    std::cout << "|Generator Y12|: " << Y12_abs_val << " S" << std::endl;
    std::cout << "|Generator Y21|: " << Y21_abs_val << " S" << std::endl;
    std::cout << "|Generator Y22|: " << Y22_abs_val << " S" << std::endl;
}*/