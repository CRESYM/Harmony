#ifndef POWERFLOW_H
#define POWERFLOW_H

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

struct DCBusResult {
    std::string busName;   
    int busIndex;         
    double vn;         
    double pn;         
    double ps, qs, vs, thetas;        
    double pc, qc, vc, thetac;         
};

class PowerFlow {
public:

    
    PowerFlow() {};

    // Data loading
    std::unordered_map<std::string, Eigen::MatrixXd> create_ac(const std::string& case_name);
    std::unordered_map<std::string, Eigen::MatrixXd> create_dc(const std::string& case_name);

    //Power flow computation //network_powerflow.cpp

    void addBusAC(std::vector<std::vector<std::string>>& dict_ac,
        Bus* bus, std::map<std::string, double>& global_params, bool print_info = false);

    void addBusDC(std::vector<std::vector<std::string>>& dict_dc,
        Bus* bus, std::map<std::string, double>& global_params, bool print_info = false);

    void make_BranchAC(Element* element, std::map<std::string, double>& global_params,
        bool print_info = false);

    void make_BranchDC(Element* element, std::map<std::string, double>& global_params,
        bool print_info = false);

    void make_Generator(Element* element, std::map<std::string, double>& global_params, 
        bool print_info = false);

    void make_Load(Element* element, std::map<std::string, double>& global_params,
        bool print_info = false);

    void make_Converter(Element* element, std::map<std::string, double>& global_params,
        bool print_info = false);

    void make_OPF(Network* net, std::map<std::string, double>& global_params, bool vscControl = true, bool writeTxt = false,
        bool plotResult = false, bool print_info = false);

    void load_params_ac(const std::string& acgrid_name, const std::unordered_map<std::string, Eigen::MatrixXd>& dataOPF);
    void load_params_dc(const std::string& dcgrid_name, const std::unordered_map<std::string, Eigen::MatrixXd>& dataOPF);

    void solve_opf(const std::string& dc_name,
        const std::string& ac_name, std::unordered_map<std::string, Eigen::MatrixXd>* dataOPF,
        bool vscControl, bool writeTxt, bool plotResult, bool print_info);

    const auto& getNetData() const { return data; }
    auto& getNetData() { return data; }

    // get Results
    DCBusResult getDCBusResult(const std::string& dcBusName,
        const std::map<std::string, double>& global_params) const;
    // get OPF results
    // Visualization
    //void viz_opf();

private:
    // Internal data - AC
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

    // Internal data - DC
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

    // DC optimization variables
    Eigen::VectorXd pn_dc, Ic_dc, lc_dc;
    Eigen::VectorXd Ctt_dc, Ccc_dc, Ctc_dc, Stc_dc, Cct_dc, Sct_dc, convPloss_dc;
    // Eigen::MatrixXd lij_dc_k;

    // Visualization buffers
    //Eigen::VectorXd vn2_dc_k, ps_dc_k, qs_dc_k;
    //Eigen::MatrixXd pij_dc_k;
    std::vector<int> nbuses_ac_viz, ngens_ac_viz;
    //std::vector<Eigen::VectorXd> vn2_ac_k, pgen_ac_k, qgen_ac_k;
    //std::vector<Eigen::MatrixXd> pij_ac_k, qij_ac_k;
    int nconvs_dc_viz, nbuses_dc_viz, ngrids_viz;

    //store Dictionary OPF
    std::map<std::string, std::map<std::string, std::map<std::string, double>>> data;

    std::unordered_map<std::string, int> busName2Id_;

    // Internal helper
    Eigen::MatrixXd readCSVtoCpp(const std::string& filename);
    Eigen::SparseMatrix<std::complex<double>> makeYbus(double baseMVA, const Eigen::MatrixXd& bus, const Eigen::MatrixXd& branch);

    // Read results
    // --- DC-side results ---
    Eigen::VectorXd vn2_dc_k;       // Solved DC bus voltage squared
    Eigen::VectorXd pn_dc_k;        // Solved DC bus power injection
    Eigen::MatrixXd pij_dc_k;       // Solved DC branch active power
    Eigen::MatrixXd lij_dc_k;       // Solved DC branch current squared
    Eigen::VectorXd ps_dc_k, qs_dc_k;   // Solved converter AC-side power
    Eigen::VectorXd pc_dc_k, qc_dc_k;   // Solved converter DC-side power
    Eigen::VectorXd theta_s_k, theta_c_k; // Solved converter theta
    Eigen::VectorXd v2s_dc_k, v2c_dc_k; // Solved converter voltage
    Eigen::VectorXd convPloss_dc_k;       // Solved converter power loss

    // --- AC-side results ---
    std::vector<Eigen::VectorXd> vn2_ac_k;   // AC bus voltage squared 
    std::vector<Eigen::VectorXd> theta_ac_k;   // AC bus voltage squared 
    std::vector<Eigen::VectorXd> pn_ac_k;    // AC nodal active injection
    std::vector<Eigen::VectorXd> qn_ac_k;    // AC nodal reactive injection
    std::vector<Eigen::VectorXd> pgen_ac_k;  // Generator active power
    std::vector<Eigen::VectorXd> qgen_ac_k;  // Generator reactive power
    std::vector<Eigen::MatrixXd> pij_ac_k;   // AC branch active power
    std::vector<Eigen::MatrixXd> qij_ac_k;   // AC branch reactive power
    std::vector<Eigen::MatrixXd> ss_ac_k;    // SOC relaxation variable (sin)
    std::vector<Eigen::MatrixXd> cc_ac_k;    // SOC relaxation variable (cos)

    std::vector<Element*> conv_point;  // Pointers to converter (MMC) elements, aligned with conv_dc rows

};

#endif // POWERFLOW_H
