/**
 * @file example_OPF_ieee.cpp
 * @brief Runnable examples: IEEE benchmark OPF cases from bundled CSV data.
 *
 * Case data lives in `src/data` and was produced with `tools/matpower_to_harmony.py`:
 *
 *  - `ieee9`       MATPOWER `case9` (WSCC 3-machine 9-bus), 345 kV, 100 MVA
 *  - `ieee39`      MATPOWER `case39` (New England 39-bus)
 *  - `ieee39hvdc`  pglib-opf-hvdc `case39_10_he` — case39 plus a 10-terminal DC grid
 *  - `rts24hvdc`   pglib-opf-hvdc `case24_7_jb` — three RTS zones joined only by DC
 *  - `hvdc2ptp`    two point-to-point HVDC links, for use with the `ieee9` AC grid
 */
#include "Examples.h"
#include "../Solver/OPF/Powerflow.h"

void example_OPF_ieee9(bool plotting_enabled /*=true*/)
{
    PowerFlow pf;
    std::unordered_map<std::string, Eigen::MatrixXd>* noData = nullptr;

    pf.solve_opf("", "ieee9", noData,
        /*vscControl=*/false,
        /*writeTxt=*/false,
        /*plotResult=*/plotting_enabled,
        /*print_info=*/false);
}

void example_OPF_ieee9_hvdc(bool plotting_enabled /*=true*/)
{
    PowerFlow pf;
    std::unordered_map<std::string, Eigen::MatrixXd>* noData = nullptr;

    // Each link has one converter on constant power and one on DC voltage,
    // so the converter setpoints in the CSV are enforced.
    pf.solve_opf("hvdc2ptp", "ieee9", noData,
        /*vscControl=*/true,
        /*writeTxt=*/false,
        /*plotResult=*/plotting_enabled,
        /*print_info=*/false);
}

void example_OPF_ieee39(bool plotting_enabled /*=true*/)
{
    PowerFlow pf;
    std::unordered_map<std::string, Eigen::MatrixXd>* noData = nullptr;

    pf.solve_opf("", "ieee39", noData,
        /*vscControl=*/false,
        /*writeTxt=*/false,
        /*plotResult=*/plotting_enabled,
        /*print_info=*/false);
}

void example_OPF_ieee39_hvdc(bool plotting_enabled /*=true*/)
{
    PowerFlow pf;
    std::unordered_map<std::string, Eigen::MatrixXd>* noData = nullptr;

    // No converter in this benchmark controls DC voltage, so the setpoints are
    // left to the optimiser instead of being fixed by the control model.
    pf.solve_opf("ieee39hvdc", "ieee39hvdc", noData,
        /*vscControl=*/false,
        /*writeTxt=*/false,
        /*plotResult=*/plotting_enabled,
        /*print_info=*/false);
}

void example_OPF_rts24_hvdc(bool plotting_enabled /*=true*/)
{
    PowerFlow pf;
    std::unordered_map<std::string, Eigen::MatrixXd>* noData = nullptr;

    pf.solve_opf("rts24hvdc", "rts24hvdc", noData,
        /*vscControl=*/false,
        /*writeTxt=*/false,
        /*plotResult=*/plotting_enabled,
        /*print_info=*/false);
}
