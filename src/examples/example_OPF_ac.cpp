/**
 * @file example_OPF_ac.cpp
 * @brief Runnable example: AC-only OPF from bundled CSV case data.
 */
#include "Examples.h"
#include "../Solver/OPF/Powerflow.h"

void example_OPF_ac(bool plotting_enabled /*=true*/)
{
    PowerFlow pf;
    std::unordered_map<std::string, Eigen::MatrixXd>* noData = nullptr;

    pf.solve_opf("", "ac5", noData,
        /*vscControl=*/false,
        /*writeTxt=*/false,
        /*plotResult=*/plotting_enabled,
        /*print_info=*/false);

    std::cout << "Press Enter to exit...\n";
    std::cin.get();
}
