#ifndef POWERFLOW_H
#define POWERFLOW_H

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <unordered_map>
#include <string>
#include <vector>
#include <complex>

class PowerFlow {
public:

    // Internal data - AC
    std::unordered_map<std::string, Eigen::MatrixXd> network_ac;
    double baseMVA_ac;
    Eigen::MatrixXd bus_entire_ac, branch_entire_ac, gen_entire_ac, gencost_entire_ac;
    int ngrids;
    std::vector<Eigen::MatrixXd> bus_ac, branch_ac, generator_ac, gencost_ac;
    std::vector<std::vector<int>> recRef;
    std::vector<Eigen::VectorXd> pd_ac, qd_ac;
    std::vector<int> nbuses_ac, nbranches_ac, ngens_ac;
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

    // Visualization buffers
    Eigen::VectorXd vn2_dc_k, ps_dc_k, qs_dc_k;
    Eigen::MatrixXd pij_dc_k;
    std::vector<int> nbuses_ac_viz, ngens_ac_viz;
    std::vector<Eigen::VectorXd> vn2_ac_k, pgen_ac_k, qgen_ac_k;
    std::vector<Eigen::MatrixXd> pij_ac_k, qij_ac_k;
    int nconvs_dc_viz, nbuses_dc_viz, ngrids_viz; 
    
    PowerFlow() {};

    // Data loading
    std::unordered_map<std::string, Eigen::MatrixXd> create_ac(const std::string& case_name);
    std::unordered_map<std::string, Eigen::MatrixXd> create_dc(const std::string& case_name);
    static Eigen::SparseMatrix<double> absoluteSparseMatrix(const Eigen::SparseMatrix<std::complex<double>>& matrix);

    void load_params_ac(const std::string& acgrid_name, const std::unordered_map<std::string, Eigen::MatrixXd>& dataOPF);
    void load_params_dc(const std::string& dcgrid_name, const std::unordered_map<std::string, Eigen::MatrixXd>& dataOPF);

    // OPF solving
    //void solve_opf(const std::string& dc_name, const std::string& ac_name,
    //    bool vscControl = true, bool writeTxt = false, bool plotResult = true);

    //void solveHmo_opf(std::unordered_map<std::string, Eigen::MatrixXd>& dataOPF,
    //    bool vscControl = true, bool writeTxt = false, bool plotResult = true);
    void solve_unified_opf(
        const std::string& dc_name,
        const std::string& ac_name,
        std::unordered_map<std::string, Eigen::MatrixXd>* dataOPF,
        bool vscControl,
        bool writeTxt,
        bool plotResult);

    // Visualization
    //void viz_opf();

private:
    // Internal helper
    Eigen::MatrixXd readCSVtoCpp(const std::string& filename);
    Eigen::SparseMatrix<std::complex<double>> makeYbus(double baseMVA, const Eigen::MatrixXd& bus, const Eigen::MatrixXd& branch);
};

#endif // POWERFLOW_H
