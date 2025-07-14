// power_network_params.cpp

#include "powerflow.h"
#include <set>
#include <iostream>

void PowerFlow::params_ac(const std::string& acgrid_name) {

    // Load AC grid data
    network_ac = create_ac(acgrid_name);

    baseMVA_ac = network_ac["baseMVA"](0, 0);
    bus_entire_ac = network_ac["bus"];
    branch_entire_ac = network_ac["branch"];
    gen_entire_ac = network_ac["generator"];
    gencost_entire_ac = network_ac["gencost"];

    // Determine number of AC grids by unique area IDs (assuming col 13 holds area)
    std::set<int> unique_areas;
    for (int i = 0; i < bus_entire_ac.rows(); ++i) {
        unique_areas.insert(static_cast<int>(bus_entire_ac(i, 13)));
    }
    ngrids = static_cast<int>(unique_areas.size());

    // Resize containers
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

    // Partition data per grid
    for (int ng = 0; ng < ngrids; ++ng) {
        std::vector<int> bus_rows, branch_rows, generator_rows, gencost_rows;

        // Filter buses
        for (int i = 0; i < bus_entire_ac.rows(); ++i) {
            if (static_cast<int>(bus_entire_ac(i, 13)) == ng + 1) {
                bus_rows.push_back(i);
            }
        }
        bus_ac[ng].resize(bus_rows.size(), bus_entire_ac.cols());
        for (size_t i = 0; i < bus_rows.size(); ++i) {
            bus_ac[ng].row(i) = bus_entire_ac.row(bus_rows[i]);
        }

        // Filter branches
        for (int i = 0; i < branch_entire_ac.rows(); ++i) {
            if (static_cast<int>(branch_entire_ac(i, 13)) == ng + 1) {
                branch_rows.push_back(i);
            }
        }
        branch_ac[ng].resize(branch_rows.size(), branch_entire_ac.cols());
        for (size_t i = 0; i < branch_rows.size(); ++i) {
            branch_ac[ng].row(i) = branch_entire_ac.row(branch_rows[i]);
        }

        // Filter generators
        for (int i = 0; i < gen_entire_ac.rows(); ++i) {
            if (static_cast<int>(gen_entire_ac(i, 21)) == ng + 1) {
                generator_rows.push_back(i);
            }
        }
        generator_ac[ng].resize(generator_rows.size(), gen_entire_ac.cols());
        for (size_t i = 0; i < generator_rows.size(); ++i) {
            generator_ac[ng].row(i) = gen_entire_ac.row(generator_rows[i]);
        }

        // Filter gencost
        for (int i = 0; i < gencost_entire_ac.rows(); ++i) {
            if (static_cast<int>(gencost_entire_ac(i, 7)) == ng + 1) {
                gencost_rows.push_back(i);
            }
        }
        gencost_ac[ng].resize(gencost_rows.size(), gencost_entire_ac.cols());
        for (size_t i = 0; i < gencost_rows.size(); ++i) {
            gencost_ac[ng].row(i) = gencost_entire_ac.row(gencost_rows[i]);
        }

        nbuses_ac[ng] = static_cast<int>(bus_ac[ng].rows());
        nbranches_ac[ng] = static_cast<int>(branch_ac[ng].rows());

        // Map bus IDs to local indices
        IDtoCountmap[ng] = Eigen::VectorXi::Zero(nbuses_ac[ng]);
        for (int i = 0; i < nbuses_ac[ng]; ++i) {
            int bus_id = static_cast<int>(bus_ac[ng](i, 0));
            IDtoCountmap[ng](bus_id - 1) = i;
            if (bus_ac[ng](i, 1) == 3) {  // PQ bus type 3 = reference bus
                refbuscount_ac[ng] = i;
            }
        }
        recRef[ng].push_back(refbuscount_ac[ng]);

        ngens_ac[ng] = static_cast<int>(generator_ac[ng].rows());

        // Compute admittance matrices
        Eigen::SparseMatrix<std::complex<double>> YY_ac = makeYbus(baseMVA_ac, bus_ac[ng], branch_ac[ng]);
        GG_ac[ng] = YY_ac.real();
        BB_ac[ng] = YY_ac.imag();

        // Store "from" and "to" bus indices
        fbus_ac[ng] = branch_ac[ng].col(0).cast<int>();
        tbus_ac[ng] = branch_ac[ng].col(1).cast<int>();

        // Normalize loads by baseMVA
        pd_ac[ng] = bus_ac[ng].col(2) / baseMVA_ac;
        qd_ac[ng] = bus_ac[ng].col(3) / baseMVA_ac;
    }
}
