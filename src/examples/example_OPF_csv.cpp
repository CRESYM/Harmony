#include "Examples.h"
#include "../Solver/OPF/Powerflow.h"

void example_OPF_csv()
{

    PowerFlow pf;
    std::unordered_map<std::string, Eigen::MatrixXd>* noData = nullptr;

    std::string dc_case = "mtdc3";   
    std::string ac_case = "ac5";       

    pf.solve_opf(dc_case, ac_case, noData,
        /*vscControl=*/true,
        /*writeTxt=*/false,
        /*plotResult=*/false,
        /*print_info=*/false);
}