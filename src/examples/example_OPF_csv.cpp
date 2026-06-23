/**
 * @file example_OPF_csv.cpp
 * @brief Runnable example: OPF case driven by CSV network data.
 */
#include "Examples.h"
#include "../Solver/OPF/Powerflow.h"

void example_OPF_csv(bool plotting_enabled /*=true*/)
{

    PowerFlow pf;
    std::unordered_map<std::string, Eigen::MatrixXd>* noData = nullptr;

    std::string dc_case = "mtdc3";   
    std::string ac_case = "ac5";       

    pf.solve_opf(dc_case, ac_case, noData,
        /*vscControl=*/true,
        /*writeTxt=*/false,
        /*plotResult=*/plotting_enabled,
        /*print_info=*/false);

    std::cout << "Press Enter to exit...\n";
    std::cin.get();
}