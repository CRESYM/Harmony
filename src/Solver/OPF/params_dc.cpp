#include "params_dc.h"
#include "create_dc.h"
#include "makeYbus.h"
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <unordered_map>
#include <iostream>
#include <stdexcept>
#include <complex>


static Eigen::SparseMatrix<double> absoluteSparseMatrix(const Eigen::SparseMatrix<std::complex<double>>& matrix) {
    Eigen::SparseMatrix<double> absMatrix(matrix.rows(), matrix.cols());
    for (int k = 0; k < matrix.outerSize(); ++k) {
        for (Eigen::SparseMatrix<std::complex<double>>::InnerIterator it(matrix, k); it; ++it) {
            absMatrix.insert(it.row(), it.col()) = std::abs(it.value());
        }
    }
    return absMatrix;
}

/*
 * Function: params_dc
 * -------------------
 * This function extracts and computes parameters for the DC network from the
 * specified case.
 *
 * Inputs:
 *    dcgrid_name: A string for the DC network case.
 *
 * Returns:
 *     DCNetworkParams Structure containing:
 *     - network_dc     : UMap.DC network containing all network data.
 *     - baseMW_dc      : Scalar. DC network MW base.
 *     - pol_dc         : Scalar. DC network pole.
 *     - nbuses_dc      : Scalar. Number of DC buses.
 *     - nbranches_dc   : Scalar. Number of DC branches.
 *     - nconvs_dc      : Scalar. Number of converters.
 *     - bus_dc         : Matrix. DC network bus data.
 *     - branch_dc      : Matrix. DC network branch data.
 *     - conv_dc        : Matrix. VSC converter data.
 *     - ybus_dc        : Matrix. DC network nodal admittance matrix (absolute values).
 *     - baseKV_dc      : Vector. AC voltage base for VSC converters.
 *     - fbus_dc        : Vector. "From" bus indices for branches.
 *     - tbus_dc        : Vector. "To" bus indices for branches.
 *     - rtf_dc         : Vector. PCC side transformer resistances (p.u.).
 *     - xtf_dc         : Vector. PCC side transformer reactances (p.u.).
 *     - bf_dc          : Vector. Filter susceptances (p.u.).
 *     - rc_dc          : Vector. AC terminal side converter resistances (p.u.).
 *     - xc_dc          : Vector. AC terminal side converter reactances (p.u.).
 *     - ztfc_dc        : Vector. VSC impedances for PCC side to AC terminal side (p.u.).
 *     - gtfc_dc        : Vector. Real parts of 1./ztfc_dc (p.u.).
 *     - btfc_dc        : Vector. Imaginary parts of 1./ztfc_dc (p.u.).
 *     - aloss_dc       : Vector. Converter loss parameters A (p.u.).
 *     - bloss_dc       : Vector. Converter loss parameters B (p.u.).
 *     - closs_dc       : Vector. Converter loss parameters C (p.u.).
 *     - convState      : Vector. Converter state indicators (1 for inverter, 0 for rectifier).
 *
 * See also: create_dc.cpp, makeYbus.cpp
 */

DCNetworkParams params_dc(const std::string& dcgrid_name) {
    DCNetworkParams sys_dc;

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

    /**************************************************
    * LOAD DC GRID DATA
    **************************************************/

    network_dc = create_dc(dcgrid_name);
    baseMW_dc = network_dc["baseMW"](0, 0);
    pol_dc = network_dc["pol"](0, 0);
    bus_dc = network_dc["bus"];
    branch_dc = network_dc["branch"];
    conv_dc = network_dc["converter"];
    basekV_dc = conv_dc.col(13);

    //  Determine the number of buses, branches, and converters
    nbuses_dc = bus_dc.rows();
    nbranches_dc = branch_dc.rows();
    nconvs_dc = conv_dc.rows();

    // Extract branch "from" and "to" bus indices
    fbus_dc = branch_dc.col(0).cast<int>();
    tbus_dc = branch_dc.col(1).cast<int>();


    /**************************************************
    * COMPUTE DC GRID ADMITTANCE (YBUS)
    **************************************************/

    Y_dc = makeYbus(baseMW_dc, bus_dc, branch_dc);
    y_dc = absoluteSparseMatrix(Y_dc);

    /* -------------------------------------------------------------------------
      Extract VSC Impedance parameters
      The equivalent circuit for the VSC converter is as follows :

      U_s  ●--[r_tf + j * x_tf]--●--[r_c + j * x_c]--● U_c--(VSC)--
           PCC                   Filter               AC terminal

      PCC side parameters(r_tf, x_tf, bf_dc) and AC terminal side
      parameters(r_c, x_c) are combined to form the overall converter impedance.
     -------------------------------------------------------------------------*/

     //  PCC side transformer parameters
    rtf_dc = conv_dc.col(8);
    xtf_dc = conv_dc.col(9);

    // Filter parameters
    bf_dc = conv_dc.col(10);

    // AC terminal side converter impedance parameters
    rc_dc = conv_dc.col(11);
    xc_dc = conv_dc.col(12);

    // Combined converter impedance
    ztfc_dc = (rtf_dc + rc_dc).array() + std::complex<double>(0, 1) * (xtf_dc + xc_dc).array();
    gtfc_dc = ztfc_dc.array().inverse().real();
    btfc_dc = ztfc_dc.array().inverse().imag();


    /**************************************************
    * CONVERTER LOSS PARAMETERS AND STATE DETERMINATION
    **************************************************/

    closs_dc = Eigen::VectorXd::Zero(nconvs_dc);
    convState_dc = Eigen::VectorXi::Zero(nconvs_dc);

    for (int i = 0; i < nconvs_dc; ++i) {
        if (conv_dc(i, 5) >= 0) { // Inverter state
            closs_dc(i) = conv_dc(i, 21);
            convState_dc(i) = 1;
        }
        else {             // Rectifier state
            closs_dc(i) = conv_dc(i, 20);
            convState_dc(i) = 0;
        }
    }

    aloss_dc = conv_dc.col(18) / baseMW_dc;
    bloss_dc = conv_dc.col(19).array() / basekV_dc.array();
    closs_dc = closs_dc.array() / (basekV_dc.array().square() / baseMW_dc);

    return sys_dc;
}

