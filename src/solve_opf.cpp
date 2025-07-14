#include <string>      
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <chrono>

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <matplot/matplot.h>
#include "gurobi_c++.h"
#include "create_dc.h"
#include "params_dc.h"
#include "Powerflow.h"
//#include "makeYbus.h"
//#include "create_ac.h"
//#include "params_ac.h"
#include "viz_opf.h"


using namespace std;

void solve_opf(const std::string& dc_name, const std::string& ac_name,
    bool vscControl = true, bool  writeTxt = false, bool  plotResult = true) {

    auto start = std::chrono::high_resolution_clock::now();
    try {

        DCNetworkParams sys_dc = params_dc(dc_name);
        auto& network_dc = sys_dc.network_dc;
        auto& baseMW_dc = sys_dc.baseMW_dc;
        auto& pol_dc = sys_dc.pol_dc;
        auto& bus_dc = sys_dc.bus_dc;
        auto& branch_dc = sys_dc.branch_dc;
        auto& conv_dc = sys_dc.conv_dc;
        auto& basekV_dc = sys_dc.basekV_dc;
        auto& nbuses_dc = sys_dc.nbuses_dc;
        auto& nbranches_dc = sys_dc.nbranches_dc;
        auto& nconvs_dc = sys_dc.nconvs_dc;
        auto& Y_dc = sys_dc.Y_dc;
        auto& y_dc = sys_dc.y_dc;
        auto& rtf_dc = sys_dc.rtf_dc;
        auto& xtf_dc = sys_dc.xtf_dc;
        auto& bf_dc = sys_dc.bf_dc;
        auto& rc_dc = sys_dc.rc_dc;
        auto& xc_dc = sys_dc.xc_dc;
        auto& ztfc_dc = sys_dc.ztfc_dc;
        auto& gtfc_dc = sys_dc.gtfc_dc;
        auto& btfc_dc = sys_dc.btfc_dc;
        auto& aloss_dc = sys_dc.aloss_dc;
        auto& bloss_dc = sys_dc.bloss_dc;
        auto& closs_dc = sys_dc.closs_dc;
        auto& convState_dc = sys_dc.convState_dc;
        auto& fbus_dc = sys_dc.fbus_dc;
        auto& tbus_dc = sys_dc.tbus_dc;

        //ACNetworkParams sys_ac = params_ac(ac_name);
        PowerFlow sys_ac;
        sys_ac.params_ac(ac_name);
        auto& network_ac = sys_ac.network_ac;
        auto& baseMVA_ac = sys_ac.baseMVA_ac;
        auto& bus_entire_ac = sys_ac.bus_entire_ac;
        auto& branch_entire_ac = sys_ac.branch_entire_ac;
        auto& gen_entire_ac = sys_ac.gen_entire_ac;
        auto& gencost_entire_ac = sys_ac.gencost_entire_ac;
        auto& ngrids = sys_ac.ngrids;
        auto& bus_ac = sys_ac.bus_ac;
        auto& branch_ac = sys_ac.branch_ac;
        auto& generator_ac = sys_ac.generator_ac;
        auto& gencost_ac = sys_ac.gencost_ac;
        auto& recRef = sys_ac.recRef;
        auto& pd_ac = sys_ac.pd_ac;
        auto& qd_ac = sys_ac.qd_ac;
        auto& nbuses_ac = sys_ac.nbuses_ac;
        auto& nbranches_ac = sys_ac.nbranches_ac;
        auto& ngens_ac = sys_ac.ngens_ac;
        auto& GG_ac = sys_ac.GG_ac;
        auto& BB_ac = sys_ac.BB_ac;
        auto& GG_ft_ac = sys_ac.GG_ft_ac;
        auto& BB_ft_ac = sys_ac.BB_ft_ac;
        auto& GG_tf_ac = sys_ac.GG_tf_ac;
        auto& BB_tf_ac = sys_ac.BB_tf_ac;
        auto& fbus_ac = sys_ac.fbus_ac;
        auto& tbus_ac = sys_ac.tbus_ac;
        auto& anglelim_rad = sys_ac.anglelim_rad;
        auto& IDtoCountmap = sys_ac.IDtoCountmap;
        auto& refbuscount_ac = sys_ac.refbuscount_ac;

        GRBEnv env = GRBEnv(true);
        env.start();
        GRBModel model = GRBModel(env);

        /**************************************************
        * DEFINE MAIN DECISION VARIABLES AND BOUNDS IN DC SIDE
         **************************************************/

         // 1. vn2_dc: the square of DC nodal voltage
        Eigen::Matrix<GRBVar, Eigen::Dynamic, 1> vn2_dc(nbuses_dc);
        for (int i = 0; i < nbuses_dc; ++i) {
            vn2_dc(i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
            model.addConstr(vn2_dc(i) >= pow(bus_dc(i, 12), 2));
            model.addConstr(vn2_dc(i) <= pow(bus_dc(i, 11), 2));
        }
        // 2. pn_dc: dc nodal active power injection
        Eigen::Matrix<GRBVar, Eigen::Dynamic, 1> pn_dc(nbuses_dc);
        for (int i = 0; i < nbuses_dc; ++i) {
            pn_dc(i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
        }
        // 3. ps_dc: active power injection at node s of vsc ac side
        Eigen::Matrix<GRBVar, Eigen::Dynamic, 1> ps_dc(nconvs_dc);
        for (int i = 0; i < nconvs_dc; ++i) {
            ps_dc(i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
        }
        // 4. qs_dc: reactive power injection at node s of vsc ac side
        Eigen::Matrix<GRBVar, Eigen::Dynamic, 1> qs_dc(nconvs_dc);
        for (int i = 0; i < nconvs_dc; ++i) {
            qs_dc(i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
        }
        // 5. pc_dc: active power injection at node c of vsc ac side
        Eigen::Matrix<GRBVar, Eigen::Dynamic, 1> pc_dc(nconvs_dc);
        for (int i = 0; i < nconvs_dc; ++i) {
            pc_dc(i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
        }
        // 6. qc_dc: reactive power injection at node c of vsc ac side
        Eigen::Matrix<GRBVar, Eigen::Dynamic, 1> qc_dc(nconvs_dc);
        for (int i = 0; i < nconvs_dc; ++i) {
            qc_dc(i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
        }
        // 7. v2s_dc: the squared nodal voltage amplitude at node s of vsc ac side
        Eigen::Matrix<GRBVar, Eigen::Dynamic, 1> v2s_dc(nconvs_dc);
        for (int i = 0; i < nconvs_dc; ++i) {
            v2s_dc(i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
            model.addConstr(v2s_dc(i) >= pow(conv_dc(i, 15), 2));
            model.addConstr(v2s_dc(i) <= pow(conv_dc(i, 14), 2));
        }
        // 8. v2c_dc: the squared nodal voltage amplitude at node c of vsc ac side
        Eigen::Matrix<GRBVar, Eigen::Dynamic, 1> v2c_dc(nconvs_dc);
        for (int i = 0; i < nconvs_dc; ++i) {
            v2c_dc(i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
            model.addConstr(v2c_dc(i) >= pow(conv_dc(i, 15), 2));
            model.addConstr(v2c_dc(i) <= pow(conv_dc(i, 14), 2));
        }
        // 9. Ic_dc: current amplitude of vsc
        Eigen::Matrix<GRBVar, Eigen::Dynamic, 1> Ic_dc(nconvs_dc);
        for (int i = 0; i < nconvs_dc; ++i) {
            Ic_dc(i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
            model.addConstr(Ic_dc(i) >= 0);
            model.addConstr(Ic_dc(i) <= conv_dc(i, 16));
        }
        // 10. Ic_dc: squared current amplitude of vsc
        Eigen::Matrix<GRBVar, Eigen::Dynamic, 1> lc_dc(nconvs_dc);
        for (int i = 0; i < nconvs_dc; ++i) {
            lc_dc(i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
            model.addConstr(lc_dc(i) >= 0);
            model.addConstr(lc_dc(i) <= pow(conv_dc(i, 16), 2));
        }
        // 11. pij_dc: dc branch power flow
        Eigen::Matrix<GRBVar, Eigen::Dynamic, Eigen::Dynamic> pij_dc(nbuses_dc, nbuses_dc);
        for (int i = 0; i < nbuses_dc; ++i) {
            for (int j = 0; j < nbuses_dc; ++j) {
                pij_dc(i, j) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
            }
        }
        // 12. lij_dc: the squared dc branch current
        Eigen::Matrix<GRBVar, Eigen::Dynamic, Eigen::Dynamic> lij_dc(nbuses_dc, nbuses_dc);
        for (int i = 0; i < nbuses_dc; ++i) {
            for (int j = 0; j < nbuses_dc; ++j) {
                lij_dc(i, j) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
            }
        }
        // 13. Ctt_dc, Ctc_dc, Cct_dc, Ccc_dc, Stc_dc, Sct_dc: related to second-order cone relaxed AC power flow
        Eigen::Matrix<GRBVar, Eigen::Dynamic, 1> Ctt_dc(nconvs_dc);
        Eigen::Matrix<GRBVar, Eigen::Dynamic, 1> Ccc_dc(nconvs_dc);
        Eigen::Matrix<GRBVar, Eigen::Dynamic, 1> Ctc_dc(nconvs_dc);
        Eigen::Matrix<GRBVar, Eigen::Dynamic, 1> Stc_dc(nconvs_dc);
        Eigen::Matrix<GRBVar, Eigen::Dynamic, 1> Cct_dc(nconvs_dc);
        Eigen::Matrix<GRBVar, Eigen::Dynamic, 1> Sct_dc(nconvs_dc);
        for (int i = 0; i < nconvs_dc; ++i) {
            Ctt_dc(i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
            Ccc_dc(i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
            Ctc_dc(i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
            Stc_dc(i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
            Cct_dc(i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
            Sct_dc(i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
        }

        // 15. converterPloss_dc: converter power loss
        Eigen::Matrix<GRBVar, Eigen::Dynamic, 1> convPloss_dc(nconvs_dc);
        for (int i = 0; i < nconvs_dc; ++i) {
            convPloss_dc(i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
        }


        /**************************************************
        * ADD OPERATIONAL CONSTRAINTS FOR VSC-MTDC GRID
        **************************************************/

        // 1. Constrains for dc power flow - second-order cone relaxation
        Eigen::MatrixXd y_dc_dense = Eigen::MatrixXd(y_dc);
        Eigen::MatrixXd zij_dc = y_dc_dense.array().abs().cwiseInverse().matrix();
        zij_dc -= zij_dc.diagonal().asDiagonal();
        for (int i = 0; i < zij_dc.rows(); ++i) {
            for (int j = 0; j < zij_dc.cols(); ++j) {
                if (std::isinf(zij_dc(i, j))) {
                    zij_dc(i, j) = 1e4;
                }
            }
        }

        for (int i = 0; i < nbuses_dc; ++i) {
            GRBLinExpr power_flow_sum = 0.0;
            for (int j = 0; j < nbuses_dc; ++j) {
                power_flow_sum += pij_dc(i, j);
            }
            model.addConstr(pn_dc(i) == power_flow_sum * pol_dc);
        }

        for (int i = 0; i < nbuses_dc; ++i) {
            for (int j = 0; j < nbuses_dc; ++j) {
                model.addConstr(pij_dc(i, j) + pij_dc(j, i) == zij_dc(i, j) * lij_dc(i, j));
                model.addQConstr(pij_dc(i, j) * pij_dc(i, j) <= lij_dc(i, j) * vn2_dc(i));
                model.addConstr(vn2_dc(i) - vn2_dc(j) == zij_dc(i, j) * (pij_dc(i, j) - pij_dc(j, i)));
            }
        }

        for (int i = 0; i < nbuses_dc; ++i) {
            model.addConstr(vn2_dc(i) >= 0.0);
            for (int j = 0; j < nbuses_dc; ++j) {
                model.addConstr(lij_dc(i, j) >= 0.0);
            }
        }

        // 2. Constraints for vsc ac side power flow -second-order cone relaxation
        for (int i = 0; i < nconvs_dc; ++i) {
            model.addConstr(ps_dc(i) == Ctt_dc(i) * gtfc_dc(i) - Ctc_dc(i) * gtfc_dc(i) + Stc_dc(i) * btfc_dc(i));
            model.addConstr(qs_dc(i) == -Ctt_dc(i) * btfc_dc(i) + Ctc_dc(i) * btfc_dc(i) + Stc_dc(i) * gtfc_dc(i));
            model.addConstr(pc_dc(i) == Ccc_dc(i) * gtfc_dc(i) - Cct_dc(i) * gtfc_dc(i) + Sct_dc(i) * btfc_dc(i));
            model.addConstr(qc_dc(i) == -Ccc_dc(i) * btfc_dc(i) + Cct_dc(i) * btfc_dc(i) + Sct_dc(i) * gtfc_dc(i));
            model.addConstr(Ctc_dc(i) == Cct_dc(i));
            model.addConstr(Stc_dc(i) + Sct_dc(i) == 0.0);
            model.addQConstr(Cct_dc(i) * Cct_dc(i) + Sct_dc(i) * Sct_dc(i) <= Ccc_dc(i) * Ctt_dc(i));
            model.addQConstr(Ctc_dc(i) * Ctc_dc(i) + Stc_dc(i) * Stc_dc(i) <= Ccc_dc(i) * Ctt_dc(i));
            model.addConstr(Ccc_dc(i) >= 0.0);
            model.addConstr(Ctt_dc(i) >= 0.0);
            model.addConstr(v2s_dc(i) == Ctt_dc(i));
            model.addConstr(v2c_dc(i) == Ccc_dc(i));
        }

        // 3. Constraint for power loss inside converter
        for (int i = 0; i < nconvs_dc; ++i) {
            model.addConstr(pc_dc(i) + pn_dc(i) + convPloss_dc(i) == 0.0);
            model.addConstr(convPloss_dc(i) >= 0.0);
            model.addConstr(convPloss_dc(i) <= 1.0);
            model.addConstr(convPloss_dc(i) == aloss_dc(i) + bloss_dc(i) * Ic_dc(i) + closs_dc(i) * lc_dc(i));
            model.addQConstr(pc_dc(i) * pc_dc(i) + qc_dc(i) * qc_dc(i) <= lc_dc(i) * v2c_dc(i));
            model.addConstr(lc_dc(i) >= 0.0);
            model.addConstr(Ic_dc(i) >= 0.0);
            model.addConstr(v2c_dc(i) >= 0.0);
            model.addQConstr(Ic_dc(i) * Ic_dc(i) + 0 * 0 <= lc_dc(i) * 1);
        }

        // 4. Constraint for vsc control model
        if (vscControl) {
            for (int i = 0; i < nconvs_dc; ++i) {
                /*********** dc side control model ***********/
                if (conv_dc(i, 3) == 1) { // p control
                    model.addConstr(pn_dc(i) == -conv_dc(i, 5) / baseMW_dc);
                }
                else if (conv_dc(i, 3) == 2) { //dc v control
                    model.addConstr(vn2_dc(i) == conv_dc(i, 7) * conv_dc(i, 7));
                }
                else { // droop control
                    model.addConstr(pn_dc(i) == (conv_dc(i, 23) - (1.0 / conv_dc(i, 22)) * (0.5 + 0.5 * vn2_dc(i) - conv_dc(i, 24))) / baseMW_dc * -1);
                }
                /*********** ac side control model ***********/
                if (conv_dc(i, 4) == 1) { // q control
                    model.addConstr(qs_dc(i) == -conv_dc(i, 6) / baseMW_dc);
                }
                else { // ac v control
                    model.addConstr(v2s_dc(i) == conv_dc(i, 7) * conv_dc(i, 7));
                }
                /*********** inverter or rectifier model ***********/
                if (convState_dc(i) == 0) { // rectifier
                    model.addConstr(ps_dc(i) >= 0);
                    model.addConstr(pn_dc(i) >= 0);
                    model.addConstr(pc_dc(i) <= 0);
                }
                else { // inverter
                    model.addConstr(ps_dc(i) <= 0);
                    model.addConstr(pn_dc(i) <= 0);
                    model.addConstr(pc_dc(i) >= 0);
                }
            }
        }


        /**************************************************
        DEFINE THE MAIN DECISION VARIABLES AND THEIR BOUNDS IN AC SIDE
        **************************************************/

        std::vector<std::vector<double>> lb_ac(ngrids), ub_ac(ngrids);
        std::vector<std::vector<GRBVar>> var_ac(ngrids);
        std::vector<Eigen::Matrix<GRBVar, Eigen::Dynamic, 1>> vn2_ac(ngrids), pn_ac(ngrids), qn_ac(ngrids);
        std::vector<Eigen::Matrix<GRBVar, Eigen::Dynamic, 1>> pgen_ac(ngrids), qgen_ac(ngrids);
        std::vector<Eigen::Matrix<GRBVar, Eigen::Dynamic, 1>> slfwd_ac(ngrids), slbwd_ac(ngrids);
        std::vector<Eigen::Matrix<GRBVar, Eigen::Dynamic, 1>> plfwd_ac(ngrids), qlfwd_ac(ngrids);
        std::vector<Eigen::Matrix<GRBVar, Eigen::Dynamic, 1>> plbwd_ac(ngrids), qlbwd_ac(ngrids);
        std::vector<Eigen::Matrix<GRBVar, Eigen::Dynamic, Eigen::Dynamic>> pij_ac(ngrids), qij_ac(ngrids);
        std::vector<Eigen::Matrix<GRBVar, Eigen::Dynamic, Eigen::Dynamic>> ss_ac(ngrids), cc_ac(ngrids);
        std::vector<std::vector<int>> genindex(ngrids);
        std::vector<std::vector<int>> convindex(ngrids);
        std::vector<Eigen::VectorXd> actgen_ac(ngrids);

        for (int ng = 0; ng < ngrids; ++ng) {

            // 1. vn2_ac: squared nodal voltage amplitude
            vn2_ac[ng].resize(nbuses_ac[ng]);
            for (int i = 0; i < nbuses_ac[ng]; ++i) {
                vn2_ac[ng](i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
                model.addConstr(vn2_ac[ng](i) >= pow(bus_ac[ng](i, 12), 2));
                model.addConstr(vn2_ac[ng](i) <= pow(bus_ac[ng](i, 11), 2));
            }

            // 2. pn_ac: nodal active power injection
            pn_ac[ng].resize(nbuses_ac[ng]);
            for (int i = 0; i < nbuses_ac[ng]; ++i) {
                pn_ac[ng](i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
            }

            // 3. qn_ac: nodal reactive power injection
            qn_ac[ng].resize(nbuses_ac[ng]);
            for (int i = 0; i < nbuses_ac[ng]; ++i) {
                qn_ac[ng](i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
            }

            // 4. pgen_ac: generator active power output
            pgen_ac[ng].resize(ngens_ac[ng]);
            for (int i = 0; i < ngens_ac[ng]; ++i) {
                pgen_ac[ng](i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
                model.addConstr(pgen_ac[ng](i) >= generator_ac[ng](i, 9) * generator_ac[ng](i, 7) / baseMVA_ac);
                model.addConstr(pgen_ac[ng](i) <= generator_ac[ng](i, 8) * generator_ac[ng](i, 7) / baseMVA_ac);
            }

            // 5. qgen_ac: generator reactive power output
            qgen_ac[ng].resize(ngens_ac[ng]);
            for (int i = 0; i < ngens_ac[ng]; ++i) {
                qgen_ac[ng](i) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
                model.addConstr(qgen_ac[ng](i) >= generator_ac[ng](i, 4) * generator_ac[ng](i, 7) / baseMVA_ac);
                model.addConstr(qgen_ac[ng](i) <= generator_ac[ng](i, 3) * generator_ac[ng](i, 7) / baseMVA_ac);
            }

            // 6. pij_ac, qij_ac - branch active and reactive power
            pij_ac[ng] = Eigen::Matrix<GRBVar, Eigen::Dynamic, Eigen::Dynamic>(nbuses_ac[ng], nbuses_ac[ng]);
            qij_ac[ng] = Eigen::Matrix<GRBVar, Eigen::Dynamic, Eigen::Dynamic>(nbuses_ac[ng], nbuses_ac[ng]);
            for (int i = 0; i < nbuses_ac[ng]; ++i) {
                for (int j = 0; j < nbuses_ac[ng]; ++j) {
                    pij_ac[ng](i, j) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
                    qij_ac[ng](i, j) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
                }
            }

            // 7. ss_ac, cc_ac - second-order cone relaxation terms for AC power flow
            ss_ac[ng] = Eigen::Matrix<GRBVar, Eigen::Dynamic, Eigen::Dynamic>(nbuses_ac[ng], nbuses_ac[ng]);
            cc_ac[ng] = Eigen::Matrix<GRBVar, Eigen::Dynamic, Eigen::Dynamic>(nbuses_ac[ng], nbuses_ac[ng]);
            for (int i = 0; i < nbuses_ac[ng]; ++i) {
                for (int j = 0; j < nbuses_ac[ng]; ++j) {
                    ss_ac[ng](i, j) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
                    cc_ac[ng](i, j) = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
                }
            }
        }


        /**************************************************
         * ADD OPERATIONAL CONSTRAINTS FOR INTERCONNECTED AC GRID
         **************************************************/

        for (int ng = 0; ng < ngrids; ++ng) {

            // 1. Constraint for ac power flow -second-order cone relaxation.
            for (int i = 0; i < nbuses_ac[ng]; ++i) {
                // calculate pn_ac[ng]
                GRBLinExpr pn_constraint = 0.0;
                pn_constraint += BB_ac[ng].coeff(i, i) * ss_ac[ng](i, i);
                for (int j = 0; j < nbuses_ac[ng]; ++j) {
                    pn_constraint += cc_ac[ng](i, j) * GG_ac[ng].coeff(i, j);
                    pn_constraint -= ss_ac[ng](i, j) * BB_ac[ng].coeff(i, j);
                }
                model.addConstr(pn_ac[ng](i) == pn_constraint);
                // calculate qn_ac[ng]
                GRBLinExpr qn_constraint = 0.0;
                qn_constraint += GG_ac[ng].coeff(i, i) * ss_ac[ng](i, i);
                for (int j = 0; j < nbuses_ac[ng]; ++j) {
                    qn_constraint -= cc_ac[ng](i, j) * BB_ac[ng].coeff(i, j);
                    qn_constraint -= ss_ac[ng](i, j) * GG_ac[ng].coeff(i, j);
                }
                model.addConstr(qn_ac[ng](i) == qn_constraint);
            }

            for (int i = 0; i < nbuses_ac[ng]; ++i) {
                for (int j = 0; j < nbuses_ac[ng]; ++j) {
                    // calculate pij_ac[ng]
                    GRBLinExpr pij_constraint = (cc_ac[ng](i, i) - cc_ac[ng](i, j)) * (-GG_ac[ng].coeff(i, j))
                        + ss_ac[ng](i, j) * (-BB_ac[ng].coeff(i, j));
                    model.addConstr(pij_ac[ng](i, j) == pij_constraint);
                    // calculate qij_ac[ng]
                    GRBLinExpr qij_constraint = -(cc_ac[ng](i, i) - cc_ac[ng](i, j)) * (-BB_ac[ng].coeff(i, j))
                        + ss_ac[ng](i, j) * (-GG_ac[ng].coeff(i, j));
                    model.addConstr(qij_ac[ng](i, j) == qij_constraint);
                }
            }

            // sysmmetry constraints
            for (int i = 0; i < nbuses_ac[ng]; ++i) {
                for (int j = i; j < nbuses_ac[ng]; ++j) {
                    model.addConstr(cc_ac[ng](i, j) == cc_ac[ng](j, i));
                    model.addConstr(ss_ac[ng](i, j) + ss_ac[ng](j, i) == 0.0);
                }
            }

            // relaxation constraints
            for (int i = 0; i < nbuses_ac[ng]; ++i) {
                for (int j = i; j < nbuses_ac[ng]; ++j) {
                    model.addQConstr(cc_ac[ng](i, j) * cc_ac[ng](i, j) + ss_ac[ng](i, j) * ss_ac[ng](i, j) <= cc_ac[ng](i, i) * cc_ac[ng](j, j));
                }
            }

            for (int i = 0; i < nbuses_ac[ng]; ++i) {
                model.addConstr(cc_ac[ng](i, i) == vn2_ac[ng](i));
                model.addConstr(cc_ac[ng](i, i) >= 0);
            }

            Eigen::Matrix<GRBLinExpr, Eigen::Dynamic, 1> pm_ac(nbuses_ac[ng]);
            Eigen::Matrix<GRBLinExpr, Eigen::Dynamic, 1> qm_ac(nbuses_ac[ng]);

            for (int i = 0; i < nbuses_ac[ng]; ++i) {
                pm_ac(i) = 0.0;
                qm_ac(i) = 0.0;
            }

            for (int i = 0; i < ngens_ac[ng]; ++i) {
                int index = static_cast<int>(generator_ac[ng](i, 0)) - 1;
                pm_ac(index) += pgen_ac[ng](i);
                qm_ac(index) += qgen_ac[ng](i);
            }
            for (int i = 0; i < nconvs_dc; ++i) {
                if (static_cast<int>(conv_dc(i, 2)) == ng + 1) {
                    int index = static_cast<int>(conv_dc(i, 1)) - 1;
                    pm_ac(index) -= ps_dc(i);
                    qm_ac(index) -= qs_dc(i);
                }
            }
            for (int i = 0; i < nbuses_ac[ng]; ++i) {
                model.addConstr(pn_ac[ng](i) == pm_ac(i) - pd_ac[ng](i));
                model.addConstr(qn_ac[ng](i) == qm_ac(i) - qd_ac[ng](i));
            }

            // 2. Constraint for acgrid and vsc voltage coupling
            for (int i = 0; i < nconvs_dc; ++i) {
                int j = static_cast<int>(conv_dc(i, 1)) - 1;
                int k = static_cast<int>(conv_dc(i, 2)) - 1;
                model.addConstr(vn2_ac[k](j) == v2s_dc(i));
            }

        }

        /**************************************************
        * SET OPTIMIZATION OBJECTIVE
        **************************************************/

        GRBQuadExpr obj = 0.0;
        for (int ng = 0; ng < ngrids; ++ng) {
            actgen_ac[ng] = generator_ac[ng].col(7);
            if (gencost_ac[ng](0, 3) == 3) {
                for (int i = 0; i < actgen_ac[ng].size(); ++i) {
                    obj += actgen_ac[ng](i) * (
                        baseMVA_ac * baseMVA_ac * gencost_ac[ng](i, 4) * (pgen_ac[ng](i) * pgen_ac[ng](i)) +
                        baseMVA_ac * gencost_ac[ng](i, 5) * pgen_ac[ng](i) +
                        gencost_ac[ng](i, 6)
                        );
                }
            }
            else if (gencost_ac[ng](0, 3) == 2) {
                for (int i = 0; i < actgen_ac[ng].size(); ++i) {
                    obj += actgen_ac[ng](i) * (
                        baseMVA_ac * gencost_ac[ng](i, 4) * pgen_ac[ng](i) +
                        gencost_ac[ng](i, 5)
                        );
                }
            }
        }
        GRBVar genCost = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS);

        model.setObjective(obj);
        model.set(GRB_DoubleParam_TimeLimit, 600);
        model.set(GRB_IntParam_Threads, 8);
        model.set(GRB_IntParam_Presolve, 2);
        model.set(GRB_IntParam_OutputFlag, 1);
        model.optimize();

        auto end = std::chrono::high_resolution_clock::now();


        cout << "The value of Obj is : " << model.get(GRB_DoubleAttr_ObjVal) << endl;

        if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL || model.get(GRB_IntAttr_Status) == GRB_SUBOPTIMAL)
        {
            std::cout << "Optimization succeeded. Solution found!" << std::endl;


            /**************************************************
            * PRINT OPTIMIZATION RESULTS
            **************************************************/
            std::ofstream fout;
            std::ostream* pio = &std::cout;
            std::string   outfile;
            if (writeTxt) {
                outfile = (std::filesystem::current_path() / "opf_results.txt").string();
                fout.open(outfile, std::ofstream::out | std::ofstream::trunc);
                pio = &fout;
            }

#define OUT (*pio)

            //  " ac bus print " 
            OUT << "\n================================================================================";
            OUT << "\n|   AC  Bus Data                                                               |";
            OUT << "\n================================================================================";
            OUT << "\n Area     Bus      Voltage          Generation               Load        ";
            OUT << "\n #        #    Mag [pu] Ang [deg]   Pg [MW]  Qg [MVAr]   P [MW]  Q [MVAr]";
            OUT << "\n-----   -----  --------  --------  --------  ---------   ------  --------";

            for (int ng = 0; ng < ngrids; ++ng) {
                const auto& genidx = generator_ac[ng].col(0);
                for (int i = 0; i < nbuses_ac[ng]; ++i) {
                    double vmag = std::sqrt(vn2_ac[ng](i).get(GRB_DoubleAttr_X));
                    OUT << "\n"
                        << std::setw(3) << ng + 1
                        << std::setw(8) << i + 1
                        << std::setw(11) << std::fixed << std::setprecision(3) << vmag
                        << std::setw(10) << 0.0;  // Assuming angle is zero for now

                    if (std::find(recRef[ng].begin(), recRef[ng].end(), i) != recRef[ng].end()) {
                        OUT << "*";
                    }

                    bool is_generator = (genidx.array() == i + 1).any();
                    if (is_generator) {
                        int gen_idx = -1;
                        for (int j = 0; j < genidx.size(); ++j) {
                            if (genidx(j) == i + 1) {
                                gen_idx = j;
                                break;
                            }
                        }

                        double pgen = pgen_ac[ng](gen_idx).get(GRB_DoubleAttr_X) * baseMVA_ac;
                        double qgen = qgen_ac[ng](gen_idx).get(GRB_DoubleAttr_X) * baseMVA_ac;

                        if (std::find(recRef[ng].begin(), recRef[ng].end(), i) != recRef[ng].end()) {
                            OUT << std::setw(10) << pgen << std::setw(9) << qgen;
                        }
                        else {
                            OUT << std::setw(11) << pgen << std::setw(9) << qgen;
                        }
                        double pd = pd_ac[ng](i) * baseMVA_ac;
                        double qd = qd_ac[ng](i) * baseMVA_ac;
                        OUT << std::setw(11) << pd << std::setw(9) << qd;
                    }
                    else {
                        OUT << "          -       -";
                        double pd = pd_ac[ng](i) * baseMVA_ac;
                        double qd = qd_ac[ng](i) * baseMVA_ac;
                        OUT << std::setw(12) << pd << std::setw(9) << qd;
                    }

                }
            }
            OUT << "\n-----   -----  --------  --------  --------  ---------   ------  --------";

            double GenCostResUSA = model.get(GRB_DoubleAttr_ObjVal);;
            double GenCostResEURO = GenCostResUSA / 1.08;
            OUT << "\n The total generation cost is $" << std::fixed << std::setprecision(2)
                << GenCostResUSA << "/MWh (€" << GenCostResEURO << "/MWh)";
            OUT << "\n\n";

            OUT << "\n===========================================================================================";
            OUT << "\n|     AC Grids Branch Data                                                                |";
            OUT << "\n===========================================================================================";
            OUT << "\n Area   Branch  From   To        From Branch Flow         To Branch Flow      Branch Loss";
            OUT << "\n #      #       Bus#   Bus#    Pij [MW]   Qij [MVAr]    Pij [MW]   Qij [MVAr]  Pij_loss [MW]";
            OUT << "\n ----   ------  -----  -----  ---------  ----------   ----------  ----------  -------------";

            for (size_t ng = 0; ng < ngrids; ++ng) {
                for (int i = 0; i < nbranches_ac[ng]; ++i) {
                    int from = fbus_ac[ng](i) - 1;
                    int to = tbus_ac[ng](i) - 1;
                    double pij_from_to = pij_ac[ng](from, to).get(GRB_DoubleAttr_X) * baseMVA_ac;
                    double qij_from_to = qij_ac[ng](from, to).get(GRB_DoubleAttr_X) * baseMVA_ac;
                    double pij_to_from = pij_ac[ng](to, from).get(GRB_DoubleAttr_X) * baseMVA_ac;
                    double qij_to_from = qij_ac[ng](to, from).get(GRB_DoubleAttr_X) * baseMVA_ac;

                    double pij_loss = std::abs(pij_from_to + pij_to_from);

                    OUT << "\n "
                        << std::setw(2) << ng + 1
                        << " " << std::setw(6) << i + 1
                        << " " << std::setw(7) << fbus_ac[ng](i)
                        << " " << std::setw(6) << tbus_ac[ng](i)
                        << " " << std::setw(12) << std::fixed << std::setprecision(3) << pij_from_to
                        << " " << std::setw(11) << qij_from_to
                        << " " << std::setw(12) << pij_to_from
                        << " " << std::setw(11) << qij_to_from
                        << " " << std::setw(11) << pij_loss;
                }
            }

            OUT << "\n ----   ------  -----  -----  ---------  -----------    --------  ----------  -------------";

            double NetPloss_ac = 0.0;

            for (size_t ng = 0; ng < ngrids; ++ng) {
                for (int i = 0; i < nbranches_ac[ng]; ++i) {
                    int from = fbus_ac[ng](i) - 1;
                    int to = tbus_ac[ng](i) - 1;

                    double pij_from_to = pij_ac[ng](from, to).get(GRB_DoubleAttr_X) * baseMVA_ac;
                    double pij_to_from = pij_ac[ng](to, from).get(GRB_DoubleAttr_X) * baseMVA_ac;

                    NetPloss_ac += std::abs(pij_from_to + pij_to_from);
                }
            }

            OUT << "\n The total AC network losses is " << std::fixed << std::setprecision(3) << NetPloss_ac << " MW.";
            OUT << "\n";

            // " dc bus print " 
            OUT << "\n================================================================================\n";
            OUT << "|   MTDC Bus Data                                                              |\n";
            OUT << "================================================================================\n";
            OUT << " Bus   Bus    AC   DC Voltage   DC Power   PCC Bus Injection   Converter loss\n";
            OUT << " DC #  AC #  Area   Vdc [pu]    Pdc [MW]   Ps [MW]  Qs [MVAr]  Conv_Ploss [MW]\n";
            OUT << "-----  ----  ----  ---------    --------   -------  --------    --------";

            double totalConverterLoss = 0.0;
            for (int i = 0; i < nbuses_dc; ++i) {
                int acBus = static_cast<int>(conv_dc(i, 1));
                int acArea = static_cast<int>(conv_dc(i, 2));
                double vdc = std::sqrt(vn2_dc(i).get(GRB_DoubleAttr_X));
                double pdc = pn_dc(i).get(GRB_DoubleAttr_X) * baseMW_dc;
                double ps = ps_dc(i).get(GRB_DoubleAttr_X) * baseMW_dc;
                double qs = qs_dc(i).get(GRB_DoubleAttr_X) * baseMW_dc;
                double ploss = convPloss_dc(i).get(GRB_DoubleAttr_X) * baseMW_dc;
                totalConverterLoss += ploss;
                OUT << std::fixed << std::setprecision(3);
                OUT << "\n" << std::setw(4) << i + 1
                    << std::setw(6) << acBus
                    << std::setw(6) << acArea
                    << std::setw(10) << vdc
                    << std::setw(14) << pdc
                    << std::setw(10) << ps
                    << std::setw(9) << qs
                    << std::setw(11) << ploss;
            }

            OUT << "\n-----  ----  ----  ---------    --------   -------  --------    --------";
            OUT << "\n The total converter losses is " << std::fixed << std::setprecision(3)
                << totalConverterLoss << " MW";
            OUT << "\n";

            // " dc branch print " 
            OUT << "\n ===================================================================\n";
            OUT << " |     MTDC Branch Data                                            |\n";
            OUT << " ===================================================================\n";
            OUT << " Branch  From   To     From Branch    To Branch      Branch Loss\n";
            OUT << " #       Bus#   Bus#   Flow Pij [MW]  Flow Pij [MW]  Pij_loss [MW]\n";
            OUT << " ------  -----  -----   ---------      ---------      ---------\n";

            double NetPloss_dc = 0.0;
            for (int i = 0; i < nbranches_dc; ++i) {
                int from = fbus_dc(i) - 1;
                int to = tbus_dc(i) - 1;

                double pij_from_to = pij_dc(from, to).get(GRB_DoubleAttr_X) * baseMW_dc * pol_dc;
                double pij_to_from = pij_dc(to, from).get(GRB_DoubleAttr_X) * baseMW_dc * pol_dc;
                double pij_loss = std::abs(pij_from_to + pij_to_from);

                OUT << std::setw(5) << (i + 1) << " "
                    << std::setw(6) << fbus_dc(i) << " "
                    << std::setw(6) << tbus_dc(i) << " "
                    << std::setw(11) << std::fixed << std::setprecision(3) << pij_from_to << " "
                    << std::setw(14) << pij_to_from << " "
                    << std::setw(13) << pij_loss << "\n";

                NetPloss_dc += pij_loss;
            }

            OUT << " ------  -----  -----   ---------      ---------      ---------\n";
            OUT << " The total DC network losses is " << std::fixed << std::setprecision(3) << NetPloss_dc << " MW.\n";

            auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

            OUT << "\n Execution time is " << duration << " s" << std::endl;

            if (fout.is_open()) fout.close();

            if (writeTxt)
                std::cout << "[info] OPF results saved to " << outfile << '\n';


            /**************************************************
            * EXTRACT THE OPTIMIZED RESULTS
            **************************************************/

            // 1. dc variable results
            Eigen::VectorXd vn2_dc_k(nbuses_dc), pn_dc_k(nbuses_dc), ps_dc_k(nconvs_dc),
                qs_dc_k(nconvs_dc), pc_dc_k(nconvs_dc), qc_dc_k(nconvs_dc),
                v2s_dc_k(nconvs_dc), v2c_dc_k(nconvs_dc),
                Ic_dc_k(nconvs_dc), lc_dc_k(nconvs_dc),
                convPloss_dc_k(nconvs_dc);

            Eigen::MatrixXd pij_dc_k(nbuses_dc, nbuses_dc), lij_dc_k(nbuses_dc, nbuses_dc),
                Ctt_dc_k(nconvs_dc, 1), Ccc_dc_k(nconvs_dc, 1), Ctc_dc_k(nconvs_dc, 1),
                Stc_dc_k(nconvs_dc, 1), Cct_dc_k(nconvs_dc, 1), Sct_dc_k(nconvs_dc, 1);

            for (int i = 0; i < nbuses_dc; ++i) {
                vn2_dc_k(i) = vn2_dc(i).get(GRB_DoubleAttr_X);
                pn_dc_k(i) = pn_dc(i).get(GRB_DoubleAttr_X);
            }
            for (int i = 0; i < nconvs_dc; ++i) {
                ps_dc_k(i) = ps_dc(i).get(GRB_DoubleAttr_X);
                qs_dc_k(i) = qs_dc(i).get(GRB_DoubleAttr_X);
                pc_dc_k(i) = pc_dc(i).get(GRB_DoubleAttr_X);
                qc_dc_k(i) = qc_dc(i).get(GRB_DoubleAttr_X);
                v2s_dc_k(i) = v2s_dc(i).get(GRB_DoubleAttr_X);
                v2c_dc_k(i) = v2c_dc(i).get(GRB_DoubleAttr_X);
                Ic_dc_k(i) = Ic_dc(i).get(GRB_DoubleAttr_X);
                lc_dc_k(i) = lc_dc(i).get(GRB_DoubleAttr_X);
                convPloss_dc_k(i) = convPloss_dc(i).get(GRB_DoubleAttr_X);

                Ctt_dc_k(i, 0) = Ctt_dc(i).get(GRB_DoubleAttr_X);
                Ccc_dc_k(i, 0) = Ccc_dc(i).get(GRB_DoubleAttr_X);
                Ctc_dc_k(i, 0) = Ctc_dc(i).get(GRB_DoubleAttr_X);
                Stc_dc_k(i, 0) = Stc_dc(i).get(GRB_DoubleAttr_X);
                Cct_dc_k(i, 0) = Cct_dc(i).get(GRB_DoubleAttr_X);
                Sct_dc_k(i, 0) = Sct_dc(i).get(GRB_DoubleAttr_X);
            }
            for (int i = 0; i < nbuses_dc; ++i)
                for (int j = 0; j < nbuses_dc; ++j) {
                    pij_dc_k(i, j) = pij_dc(i, j).get(GRB_DoubleAttr_X);
                    lij_dc_k(i, j) = lij_dc(i, j).get(GRB_DoubleAttr_X);
                }

            // 2. ac variable results
            std::vector<Eigen::VectorXd> vn2_ac_k(ngrids), pn_ac_k(ngrids), qn_ac_k(ngrids),
                pgen_ac_k(ngrids), qgen_ac_k(ngrids);
            std::vector<Eigen::MatrixXd> pij_ac_k(ngrids), qij_ac_k(ngrids),
                ss_ac_k(ngrids), cc_ac_k(ngrids);

            for (int ng = 0; ng < ngrids; ++ng) {
                vn2_ac_k[ng].resize(nbuses_ac[ng]);
                pn_ac_k[ng].resize(nbuses_ac[ng]);
                qn_ac_k[ng].resize(nbuses_ac[ng]);
                pgen_ac_k[ng].resize(ngens_ac[ng]);
                qgen_ac_k[ng].resize(ngens_ac[ng]);
                pij_ac_k[ng].resize(nbuses_ac[ng], nbuses_ac[ng]);
                qij_ac_k[ng].resize(nbuses_ac[ng], nbuses_ac[ng]);
                ss_ac_k[ng].resize(nbuses_ac[ng], nbuses_ac[ng]);
                cc_ac_k[ng].resize(nbuses_ac[ng], nbuses_ac[ng]);

                for (int i = 0; i < nbuses_ac[ng]; ++i) {
                    vn2_ac_k[ng](i) = vn2_ac[ng](i).get(GRB_DoubleAttr_X);
                    pn_ac_k[ng](i) = pn_ac[ng](i).get(GRB_DoubleAttr_X);
                    qn_ac_k[ng](i) = qn_ac[ng](i).get(GRB_DoubleAttr_X);
                }
                for (int i = 0; i < ngens_ac[ng]; ++i) {
                    pgen_ac_k[ng](i) = pgen_ac[ng](i).get(GRB_DoubleAttr_X);
                    qgen_ac_k[ng](i) = qgen_ac[ng](i).get(GRB_DoubleAttr_X);
                }

                for (int i = 0; i < nbuses_ac[ng]; ++i)
                    for (int j = 0; j < nbuses_ac[ng]; ++j) {
                        pij_ac_k[ng](i, j) = pij_ac[ng](i, j).get(GRB_DoubleAttr_X);
                        qij_ac_k[ng](i, j) = qij_ac[ng](i, j).get(GRB_DoubleAttr_X);
                        ss_ac_k[ng](i, j) = ss_ac[ng](i, j).get(GRB_DoubleAttr_X);
                        cc_ac_k[ng](i, j) = cc_ac[ng](i, j).get(GRB_DoubleAttr_X);
                    }

            }

            OPFVisualData vis_data;

            vis_data.bus_entire_ac = bus_entire_ac;
            vis_data.branch_entire_ac = branch_entire_ac;
            vis_data.gen_entire_ac = gen_entire_ac;
            vis_data.bus_dc = bus_dc;
            vis_data.branch_dc = branch_dc;
            vis_data.conv_dc = conv_dc;

            vis_data.vn2_dc_k = vn2_dc_k;
            vis_data.ps_dc_k = ps_dc_k;
            vis_data.qs_dc_k = qs_dc_k;
            vis_data.pij_dc_k = pij_dc_k;

            vis_data.nbuses_ac = nbuses_ac;
            vis_data.ngens_ac = ngens_ac;

            vis_data.vn2_ac_k = vn2_ac_k;
            vis_data.pgen_ac_k = pgen_ac_k;
            vis_data.qgen_ac_k = qgen_ac_k;

            vis_data.pij_ac_k = pij_ac_k;
            vis_data.qij_ac_k = qij_ac_k;

            vis_data.nconvs_dc = nconvs_dc;
            vis_data.nbuses_dc = nbuses_dc;
            vis_data.ngrids = ngrids;

            vis_data.baseMVA_ac = baseMVA_ac;
            vis_data.baseMW_dc = baseMW_dc;
            vis_data.pol_dc = pol_dc;

            if (plotResult) {
                viz_opf(vis_data);
            }

        }


        else {
            std::cerr << "Optimization did not succeed. Status code: "
                << model.get(GRB_IntAttr_Status) << std::endl;
        }

    }

    catch (GRBException& e) {
        std::cout << "Error code = " << e.getErrorCode() << endl;
        std::cout << e.getMessage() << endl;
    }
    catch (...) {
        std::cout << "Exception during optimization" << endl;
    }

}