#include "params_ac.h"
#include "create_ac.h"  
#include "makeYbus.h"   
#include <cmath>
#include <set>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <unordered_map>
#include <vector>
#include <iostream>

/*
 * Function: params_ac
 * -------------------
 * The function processes the entire AC network data grid-by-grid,
 * compute and return AC grid parameters.
 *
 * Input:
 *    acgrid_name: A string for the AC network case.
 *
 * Returns:
 *    ACNetworkParams Structure containing:
 *    - network_ac        : Umap. AC network containing all network data.
 *    - baseMVA_ac        : Scalar. AC base MVA.
 *    - bus_entire_ac     : Matrix. Complete bus data (filtered by voltage level).
 *    - branch_entire_ac  : Matrix. Complete branch data (filtered by a branch flag).
 *    - gencost_entire_ac : Matrix. Complete generation cost data.
 *    - ngrids            : Scalar. Number of AC grids.
 *    - bus_ac            : Vector. Bus data for each AC grid.
 *    - branch_ac         : Vector. Branch data for each AC grid.
 *    - generator_ac      : Vector. Generator data for each AC grid.
 *    - gencost_ac        : Vector. Generator cost for each AC grid.
 *    - recBuses_ac       : Vector. Recording buses for each AC grid.
 *    - recBranches_ac    : Vector. Recording branches for each AC grid.
 *    - recRef_ac         : Vector. Recording the reference bus for each AC grid.
 *    - pd_ac             : Vector. Active/reactive loads (per unit).
 *    - qd_ac             : Vector. Mapping AC bus IDs to connected DC converters.
 *    - convids           : Vector. Mapping AC bus IDs to connected generators.
 *    - genids            : Vector. Mapping AC bus IDs to connected generators.
 *    - nbuses_ac         : Vector. Number of buses for each AC grid.
 *    - nbranches_ac      : Vector. Number of branches for each AC grid.
 *    - ngens_ac          : Vector. Number of generators for each AC grid.
 *    - GG_ac, BB_ac      : Vector. Real and Imaginary parts of the AC admittance matrix (per unit).
 *    - refbuscount       : Vector. Indices of the reference bus for each AC grid.
 *    - GG_ft_ac, BB_ft_ac: Vectors. Branch off-diagonal entries (from-to) (per unit).
 *    - GG_tf_ac, BB_tf_ac: Vectors. Branch off-diagonal entries (to-from) (per unit).
 *    - fbus_ac, tbus_ac  : Vectors. "From" and "to” bus indices per grid.
 *
 * See also: create_dc.cpp, makeYbus.cpp
 */

ACNetworkParams params_ac(const std::string& acgrid_name) {

    ACNetworkParams sys_ac;

    // Create aliases for the AC network parameters (preserving auto& as requested)
    auto& network_ac = sys_ac.network_ac;
    auto& baseMVA_ac = sys_ac.baseMVA_ac;
    auto& bus_ac_entire = sys_ac.bus_ac_entire;
    auto& branch_ac_entire = sys_ac.branch_ac_entire;
    auto& generator_ac_entire = sys_ac.generator_ac_entire;
    auto& gencost_ac_entire = sys_ac.gencost_ac_entire;
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

    // Load the AC grid data from CSV files
    network_ac = create_ac(acgrid_name);

    // Extract base MVA and the raw data matrices
    baseMVA_ac = network_ac["baseMVA"](0, 0);
    bus_ac_entire = network_ac["bus"];
    branch_ac_entire = network_ac["branch"];
    generator_ac_entire = network_ac["generator"];
    gencost_ac_entire = network_ac["gencost"];

    /* Determine the number of AC grids based on unique grid IDs (assumed to be in column 14) */
    std::set<int> unique_values;
    for (int i = 0; i < bus_ac_entire.rows(); ++i) {
        unique_values.insert(static_cast<int>(bus_ac_entire(i, 13)));
    }
    ngrids = static_cast<int>(unique_values.size());

    // Resize vectors for grid-specific data
    bus_ac.resize(ngrids);
    branch_ac.resize(ngrids);
    generator_ac.resize(ngrids);
    gencost_ac.resize(ngrids);
    pd_ac.resize(ngrids);
    qd_ac.resize(ngrids);
    nbuses_ac.resize(ngrids);
    nbranches_ac.resize(ngrids);
    ngens_ac.resize(ngrids);
    GG_ac.resize(ngrids);
    BB_ac.resize(ngrids);
    fbus_ac.resize(ngrids);
    tbus_ac.resize(ngrids);
    GG_ft_ac.resize(ngrids);
    BB_ft_ac.resize(ngrids);
    GG_tf_ac.resize(ngrids);
    BB_tf_ac.resize(ngrids);
    IDtoCountmap.resize(ngrids);
    refbuscount_ac.resize(ngrids, -1);
    recRef.resize(ngrids);

    /* Partition raw AC data into individual grids */
    for (int ng = 0; ng < ngrids; ++ng) {

        std::vector<int> bus_rows, branch_rows, generator_rows, gencost_rows;

        // Partition bus data: grid ID is in column 14 (1-indexed)
        for (int i = 0; i < bus_ac_entire.rows(); ++i) {
            if (static_cast<int>(bus_ac_entire(i, 13)) == ng + 1) {
                bus_rows.push_back(i);
            }
        }
        bus_ac[ng] = Eigen::MatrixXd(bus_rows.size(), bus_ac_entire.cols());
        for (size_t i = 0; i < bus_rows.size(); ++i) {
            bus_ac[ng].row(i) = bus_ac_entire.row(bus_rows[i]);
        }

        // Partition branch data: grid ID is in column 14
        for (int i = 0; i < branch_ac_entire.rows(); ++i) {
            if (static_cast<int>(branch_ac_entire(i, 13)) == ng + 1) {
                branch_rows.push_back(i);
            }
        }
        branch_ac[ng] = Eigen::MatrixXd(branch_rows.size(), branch_ac_entire.cols());
        for (size_t i = 0; i < branch_rows.size(); ++i) {
            branch_ac[ng].row(i) = branch_ac_entire.row(branch_rows[i]);
        }

        // Partition generator data: grid ID is in column 21
        for (int i = 0; i < generator_ac_entire.rows(); ++i) {
            if (static_cast<int>(generator_ac_entire(i, 21)) == ng + 1) {
                generator_rows.push_back(i);
            }
        }
        generator_ac[ng] = Eigen::MatrixXd(generator_rows.size(), generator_ac_entire.cols());
        for (size_t i = 0; i < generator_rows.size(); ++i) {
            generator_ac[ng].row(i) = generator_ac_entire.row(generator_rows[i]);
        }

        // Partition generator cost data: grid ID is in column 7
        for (int i = 0; i < gencost_ac_entire.rows(); ++i) {
            if (static_cast<int>(gencost_ac_entire(i, 7)) == ng + 1) {
                gencost_rows.push_back(i);
            }
        }
        gencost_ac[ng] = Eigen::MatrixXd(gencost_rows.size(), gencost_ac_entire.cols());
        for (size_t i = 0; i < gencost_rows.size(); ++i) {
            gencost_ac[ng].row(i) = gencost_ac_entire.row(gencost_rows[i]);
        }

        // Record bus and branch counts for this grid
        nbuses_ac[ng] = static_cast<int>(bus_ac[ng].rows());
        nbranches_ac[ng] = static_cast<int>(branch_ac[ng].rows());

        // Build a mapping from global bus IDs to local indices and determine the reference bus.
        IDtoCountmap[ng] = Eigen::VectorXi::Zero(nbuses_ac[ng]);
        for (int i = 0; i < nbuses_ac[ng]; ++i) {
            int bus_id = static_cast<int>(bus_ac[ng](i, 0));
            IDtoCountmap[ng](bus_id - 1) = i;
            if (bus_ac[ng](i, 1) == 3) {
                refbuscount_ac[ng] = i;
            }
        }
        recRef[ng].push_back(refbuscount_ac[ng]);

        // Record the number of generators in this grid.
        ngens_ac[ng] = static_cast<int>(generator_ac[ng].rows());

        // Compute the AC nodal admittance matrix using makeYbus() and extract its real and imaginary parts.
        Eigen::SparseMatrix<std::complex<double>> YY_ac = makeYbus(baseMVA_ac, bus_ac[ng], branch_ac[ng]);
        GG_ac[ng] = YY_ac.real();
        BB_ac[ng] = YY_ac.imag();

        // Extract branch connectivity: from bus and to bus indices.
        fbus_ac[ng] = branch_ac[ng].col(0).cast<int>();
        tbus_ac[ng] = branch_ac[ng].col(1).cast<int>();

        // Compute per-unit load values.
        pd_ac[ng] = bus_ac[ng].col(2) / baseMVA_ac;
        qd_ac[ng] = bus_ac[ng].col(3) / baseMVA_ac;
    }

    return sys_ac;
}
