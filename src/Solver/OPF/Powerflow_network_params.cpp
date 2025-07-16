#include "powerflow.h"
#include <set>
#include <iostream>

Eigen::SparseMatrix<double> PowerFlow::absoluteSparseMatrix(const Eigen::SparseMatrix<std::complex<double>>& matrix) {
    Eigen::SparseMatrix<double> absMatrix(matrix.rows(), matrix.cols());
    for (int k = 0; k < matrix.outerSize(); ++k) {
        for (Eigen::SparseMatrix<std::complex<double>>::InnerIterator it(matrix, k); it; ++it) {
            absMatrix.insert(it.row(), it.col()) = std::abs(it.value());
        }
    }
    return absMatrix;
}

void PowerFlow::load_params_ac(const std::string& acgrid_name, const std::unordered_map<std::string, Eigen::MatrixXd>& dataOPF) {
    // Load AC data
    if (dataOPF.empty()) {
        // From file
        network_ac = create_ac(acgrid_name);
        baseMVA_ac = network_ac["baseMVA"](0, 0);
        bus_entire_ac = network_ac["bus"];
        branch_entire_ac = network_ac["branch"];
        gen_entire_ac = network_ac["generator"];
        gencost_entire_ac = network_ac["gencost"];
    }
    else {
        // From OPF input map
        baseMVA_ac = 100.0;  // or dataOPF["baseMVA"](0, 0) if available
        bus_entire_ac = dataOPF.at("busAC");
        branch_entire_ac = dataOPF.at("branchAC");
        gen_entire_ac = dataOPF.at("generator");
        gencost_entire_ac = dataOPF.at("gencost");
    }

    //Identify number of grids by unique area ID
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

    // Partition per grid
    for (int ng = 0; ng < ngrids; ++ng) {
        std::vector<int> bus_rows, branch_rows, gen_rows, gencost_rows;

        // Filter buses
        for (int i = 0; i < bus_entire_ac.rows(); ++i) {
            if (static_cast<int>(bus_entire_ac(i, 13)) == ng + 1)
                bus_rows.push_back(i);
        }
        bus_ac[ng].resize(bus_rows.size(), bus_entire_ac.cols());
        for (size_t i = 0; i < bus_rows.size(); ++i)
            bus_ac[ng].row(i) = bus_entire_ac.row(bus_rows[i]);

        // Filter branches
        for (int i = 0; i < branch_entire_ac.rows(); ++i) {
            if (static_cast<int>(branch_entire_ac(i, 13)) == ng + 1)
                branch_rows.push_back(i);
        }
        branch_ac[ng].resize(branch_rows.size(), branch_entire_ac.cols());
        for (size_t i = 0; i < branch_rows.size(); ++i)
            branch_ac[ng].row(i) = branch_entire_ac.row(branch_rows[i]);

        // Filter generators
        for (int i = 0; i < gen_entire_ac.rows(); ++i) {
            if (static_cast<int>(gen_entire_ac(i, 21)) == ng + 1)
                gen_rows.push_back(i);
        }
        generator_ac[ng].resize(gen_rows.size(), gen_entire_ac.cols());
        for (size_t i = 0; i < gen_rows.size(); ++i)
            generator_ac[ng].row(i) = gen_entire_ac.row(gen_rows[i]);

        // Filter gencosts
        for (int i = 0; i < gencost_entire_ac.rows(); ++i) {
            if (static_cast<int>(gencost_entire_ac(i, 7)) == ng + 1)
                gencost_rows.push_back(i);
        }
        gencost_ac[ng].resize(gencost_rows.size(), gencost_entire_ac.cols());
        for (size_t i = 0; i < gencost_rows.size(); ++i)
            gencost_ac[ng].row(i) = gencost_entire_ac.row(gencost_rows[i]);

        //Local index and references
        nbuses_ac[ng] = static_cast<int>(bus_ac[ng].rows());
        nbranches_ac[ng] = static_cast<int>(branch_ac[ng].rows());
        ngens_ac[ng] = static_cast<int>(generator_ac[ng].rows());

        IDtoCountmap[ng] = Eigen::VectorXi::Zero(nbuses_ac[ng]);
        for (int i = 0; i < nbuses_ac[ng]; ++i) {
            int bus_id = static_cast<int>(bus_ac[ng](i, 0));
            IDtoCountmap[ng](bus_id - 1) = i;
            if (bus_ac[ng](i, 1) == 3) {
                refbuscount_ac[ng] = i;
            }
        }
        recRef[ng].push_back(refbuscount_ac[ng]);

        //Compute admittance matrix
        Eigen::SparseMatrix<std::complex<double>> YY_ac = makeYbus(baseMVA_ac, bus_ac[ng], branch_ac[ng]);
        GG_ac[ng] = YY_ac.real();
        BB_ac[ng] = YY_ac.imag();

        //from/to buses and loads
        fbus_ac[ng] = branch_ac[ng].col(0).cast<int>();
        tbus_ac[ng] = branch_ac[ng].col(1).cast<int>();
        pd_ac[ng] = bus_ac[ng].col(2) / baseMVA_ac;
        qd_ac[ng] = bus_ac[ng].col(3) / baseMVA_ac;
    }
}
void PowerFlow::load_params_dc(const std::string& dcgrid_name, const std::unordered_map<std::string, Eigen::MatrixXd>& dataOPF) {
    if (!dcgrid_name.empty()) {
        // Load DC grid from file
        network_dc = create_dc(dcgrid_name);
        baseMW_dc = network_dc["baseMW"](0, 0);
        pol_dc = network_dc["pol"](0, 0);
        bus_dc = network_dc["bus"];
        branch_dc = network_dc["branch"];
        conv_dc = network_dc["converter"];
    }
    else {
        // Load DC grid from OPF dictionary
        baseMW_dc = 100.0;  // Default base
        pol_dc = 2.0;
        bus_dc = dataOPF.at("busDC");
        branch_dc = dataOPF.at("branchDC");
        conv_dc = dataOPF.at("converter");
    }

    basekV_dc = conv_dc.col(13);

    // Sizes
    nbuses_dc = bus_dc.rows();
    nbranches_dc = branch_dc.rows();
    nconvs_dc = conv_dc.rows();

    fbus_dc = branch_dc.col(0).cast<int>();
    tbus_dc = branch_dc.col(1).cast<int>();

    Y_dc = makeYbus(baseMW_dc, bus_dc, branch_dc);
    y_dc = absoluteSparseMatrix(Y_dc);

    rtf_dc = conv_dc.col(8);
    xtf_dc = conv_dc.col(9);
    bf_dc = conv_dc.col(10);
    rc_dc = conv_dc.col(11);
    xc_dc = conv_dc.col(12);

    ztfc_dc = (rtf_dc + rc_dc).array().cast<std::complex<double>>()
        + std::complex<double>(0.0, 1.0) * (xtf_dc + xc_dc).array().cast<std::complex<double>>();

    gtfc_dc = ztfc_dc.array().inverse().real();
    btfc_dc = ztfc_dc.array().inverse().imag();

    closs_dc = Eigen::VectorXd::Zero(nconvs_dc);
    convState_dc = Eigen::VectorXi::Zero(nconvs_dc);

    for (int i = 0; i < nconvs_dc; ++i) {
        if (conv_dc(i, 5) >= 0) {
            closs_dc(i) = conv_dc(i, 21);
            convState_dc(i) = 1;
        }
        else {
            closs_dc(i) = conv_dc(i, 20);
            convState_dc(i) = 0;
        }
    }

    aloss_dc = conv_dc.col(18) / baseMW_dc;
    bloss_dc = conv_dc.col(19).array() / basekV_dc.array();
    closs_dc = closs_dc.array() / (basekV_dc.array().square() / baseMW_dc);
}
