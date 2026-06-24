#ifndef POWERFLOW_H
#define POWERFLOW_H

/**
 * @file Powerflow.h
 * @brief AC/DC optimal power flow (OPF) solver and network data assembly.
 *
 * Loads MATPOWER-style case data, builds AC and DC network matrices from a
 * Harmony Network object, solves the combined OPF problem, and exposes
 * per-bus results for downstream use or visualization.
 */

#include "../../Constants.h"
#include "../../Bus.h"

#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <utility>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <cctype>

class Element;
class Network;
class Bus;

/**
 * @brief Per-bus DC-side OPF solution quantities.
 */
struct DCBusResult {
    std::string busName;
    int busIndex;
    double vn;
    double pn;
    double ps, qs, vs, thetas;
    double pc, qc, vc, thetac;
};

/**
 * @class PowerFlow
 * @brief Combined AC/DC optimal power flow solver.
 *
 * Manages case-data loading, network parameter extraction from Harmony
 * elements, OPF optimization, result storage, and optional visualization.
 */
class PowerFlow {
public:

    /** @brief Default constructor. */
    PowerFlow() {};

    /**
     * @brief Loads AC grid case data from bundled CSV files.
     * @param case_name Case identifier (e.g. "case57").
     * @return Map of matrix name to data (bus, branch, gen, gencost, res).
     */
    std::unordered_map<std::string, Eigen::MatrixXd> create_ac(const std::string& case_name);

    /**
     * @brief Loads DC grid case data from bundled CSV files.
     * @param case_name Case identifier.
     * @return Map of matrix name to data (bus, branch, conv).
     */
    std::unordered_map<std::string, Eigen::MatrixXd> create_dc(const std::string& case_name);

    /**
     * @brief Appends an AC bus row to the OPF bus dictionary.
     * @param dict_ac AC bus table being assembled.
     * @param bus Harmony bus object.
     * @param global_params Shared network parameters (base MVA, etc.).
     * @param print_info If true, prints diagnostic information.
     */
    void addBusAC(std::vector<std::vector<std::string>>& dict_ac,
        Bus* bus, std::map<std::string, double>& global_params, bool print_info = false);

    /**
     * @brief Appends a DC bus row to the OPF bus dictionary.
     * @param dict_dc DC bus table being assembled.
     * @param bus Harmony bus object.
     * @param global_params Shared network parameters.
     * @param print_info If true, prints diagnostic information.
     */
    void addBusDC(std::vector<std::vector<std::string>>& dict_dc,
        Bus* bus, std::map<std::string, double>& global_params, bool print_info = false);

    /**
     * @brief Adds an AC branch (line/transformer) entry to the OPF data.
     * @param element Branch element (Line, Transformer, etc.).
     * @param global_params Shared network parameters.
     * @param print_info If true, prints diagnostic information.
     */
    void make_BranchAC(Element* element, std::map<std::string, double>& global_params,
        bool print_info = false);

    /**
     * @brief Adds a DC branch entry to the OPF data.
     * @param element DC branch element.
     * @param global_params Shared network parameters.
     * @param print_info If true, prints diagnostic information.
     */
    void make_BranchDC(Element* element, std::map<std::string, double>& global_params,
        bool print_info = false);

    /**
     * @brief Adds a synchronous generator entry to the OPF data.
     * @param element Generator element.
     * @param global_params Shared network parameters.
     * @param print_info If true, prints diagnostic information.
     */
    void make_Generator(Element* element, std::map<std::string, double>& global_params, 
        bool print_info = false);

    /**
     * @brief Adds a renewable energy source (RES) entry to the OPF data.
     * @param element RES element.
     * @param global_params Shared network parameters.
     * @param print_info If true, prints diagnostic information.
     */
    void make_RES(Element* element, std::map<std::string, double>& global_params,
        bool print_info = false);

    /**
     * @brief Adds a load entry to the OPF data.
     * @param element Load element.
     * @param global_params Shared network parameters.
     * @param print_info If true, prints diagnostic information.
     */
    void make_Load(Element* element, std::map<std::string, double>& global_params,
        bool print_info = false);

    /**
     * @brief Adds a VSC/MMC converter entry to the OPF data.
     * @param element Converter element.
     * @param global_params Shared network parameters.
     * @param print_info If true, prints diagnostic information.
     */
    void make_Converter(Element* element, std::map<std::string, double>& global_params,
        bool print_info = false);

    /**
     * @brief Builds OPF data structures from a Harmony network and runs the solver.
     * @param net Network to optimize.
     * @param global_params Shared network parameters.
     * @param vscControl Enable VSC droop/control constraints (default true).
     * @param writeTxt Write text output files (default false).
     * @param plotResult Open OPF visualization (default false).
     * @param print_info Print diagnostic information (default false).
     */
    void make_OPF(Network* net, std::map<std::string, double>& global_params, bool vscControl = true, bool writeTxt = false,
        bool plotResult = false, bool print_info = false);

    /**
     * @brief Applies solved AC OPF parameters back to a named AC grid.
     * @param acgrid_name Name of the AC sub-grid.
     * @param dataOPF Solved OPF data matrices.
     */
    void load_params_ac(const std::string& acgrid_name, const std::unordered_map<std::string, Eigen::MatrixXd>& dataOPF);

    /**
     * @brief Applies solved DC OPF parameters back to a named DC grid.
     * @param dcgrid_name Name of the DC sub-grid.
     * @param dataOPF Solved OPF data matrices.
     */
    void load_params_dc(const std::string& dcgrid_name, const std::unordered_map<std::string, Eigen::MatrixXd>& dataOPF);

    /**
     * @brief Runs the combined AC/DC OPF optimization.
     * @param dc_name DC grid case name.
     * @param ac_name AC grid case name.
     * @param dataOPF Optional pointer to store/load case data; may be nullptr.
     * @param vscControl Enable VSC control constraints.
     * @param writeTxt Write text output files.
     * @param plotResult Open OPF visualization.
     * @param print_info Print diagnostic information.
     */
    void solve_opf(const std::string& dc_name,
        const std::string& ac_name, std::unordered_map<std::string, Eigen::MatrixXd>* dataOPF,
        bool vscControl, bool writeTxt, bool plotResult, bool print_info);

    /**
     * @brief Returns the nested OPF result dictionary (read-only).
     * @return Map of grid → element → parameter → value.
     */
    const auto& getNetData() const { return data; }

    /**
     * @brief Returns the nested OPF result dictionary (mutable).
     * @return Reference to the result data map.
     */
    auto& getNetData() { return data; }

    /**
     * @brief Retrieves solved DC bus quantities by name.
     * @param dcBusName Name of the DC bus.
     * @param global_params Shared network parameters (base values, polarity).
     * @return Populated DCBusResult structure.
     */
    DCBusResult getDCBusResult(const std::string& dcBusName,
        const std::map<std::string, double>& global_params) const;

private:
    std::unordered_map<std::string, Eigen::MatrixXd> network_ac;
    double baseMVA_ac;
    Eigen::MatrixXd bus_entire_ac, branch_entire_ac, gen_entire_ac, gencost_entire_ac, res_entire_ac;
    int ngrids;
    std::vector<Eigen::MatrixXd> bus_ac, branch_ac, generator_ac, gencost_ac, res_ac;
    std::vector<std::vector<int>> recRef;
    std::vector<Eigen::VectorXd> pd_ac, qd_ac, sres_ac;
    std::vector<int> nbuses_ac, nbranches_ac, ngens_ac, nress_ac;
    std::vector<Eigen::SparseMatrix<double>> GG_ac, BB_ac, GG_ft_ac, BB_ft_ac, GG_tf_ac, BB_tf_ac;
    std::vector<Eigen::VectorXi> fbus_ac, tbus_ac;
    std::vector<Eigen::MatrixXd> anglelim_rad;
    std::vector<Eigen::VectorXi> IDtoCountmap;
    std::vector<int> refbuscount_ac;

    std::unordered_map<std::string, Eigen::MatrixXd> network_dc;
    double baseMW_dc, pol_dc;
    int nbuses_dc, nbranches_dc, nconvs_dc;
    Eigen::MatrixXd bus_dc, branch_dc, conv_dc;
    Eigen::SparseMatrix<std::complex<double>> Y_dc;
    Eigen::SparseMatrix<double> y_dc;
    Eigen::VectorXcd ztfc_dc;
    Eigen::VectorXd rtf_dc, xtf_dc, bf_dc, rc_dc, xc_dc;
    Eigen::VectorXd gtfc_dc, btfc_dc, aloss_dc, bloss_dc, closs_dc;
    Eigen::VectorXd basekV_dc;
    Eigen::VectorXi convState_dc;
    Eigen::VectorXi fbus_dc, tbus_dc;

    Eigen::VectorXd pn_dc, Ic_dc, lc_dc;
    Eigen::VectorXd Ctt_dc, Ccc_dc, Ctc_dc, Stc_dc, Cct_dc, Sct_dc, convPloss_dc;

    std::vector<int> nbuses_ac_viz, ngens_ac_viz;
    int nconvs_dc_viz, nbuses_dc_viz, ngrids_viz;

    std::map<std::string, std::map<std::string, std::map<std::string, double>>> data;

    std::unordered_map<std::string, int> busName2Id_;

    Eigen::MatrixXd readCSVtoCpp(const std::string& filename);
    Eigen::SparseMatrix<std::complex<double>> makeYbus(double baseMVA, const Eigen::MatrixXd& bus, const Eigen::MatrixXd& branch);

    Eigen::VectorXd vn2_dc_k;
    Eigen::VectorXd pn_dc_k;
    Eigen::MatrixXd pij_dc_k;
    Eigen::MatrixXd lij_dc_k;
    Eigen::VectorXd ps_dc_k, qs_dc_k;
    Eigen::VectorXd pc_dc_k, qc_dc_k;
    Eigen::VectorXd theta_s_k, theta_c_k;
    Eigen::VectorXd v2s_dc_k, v2c_dc_k;
    Eigen::VectorXd convPloss_dc_k;

    std::vector<Eigen::VectorXd> vn2_ac_k;
    std::vector<Eigen::VectorXd> theta_ac_k;
    std::vector<Eigen::VectorXd> pn_ac_k;
    std::vector<Eigen::VectorXd> qn_ac_k;
    std::vector<Eigen::VectorXd> pgen_ac_k;
    std::vector<Eigen::VectorXd> qgen_ac_k;
    std::vector<Eigen::MatrixXd> pij_ac_k;
    std::vector<Eigen::MatrixXd> qij_ac_k;
    std::vector<Eigen::MatrixXd> ss_ac_k;
    std::vector<Eigen::MatrixXd> cc_ac_k;

    std::vector<Element*> conv_point;

    double opf_user_base_mva_ = 100.0;

};

#endif // POWERFLOW_H
