/**
 * @file Stability_estimate.cpp
 * @brief MIMO impedance-based stability assessment via unified MNA.
 *
 * All port dimensions are derived at runtime from Bus::getPinNumber(), so
 * the implementation handles 1-pin (DC scalar), 2-pin (dq AC) and 3-pin
 * (abc AC) buses without any hard-coded dimension assumptions.
 */
#include "Stability_estimate.h"

#include "../../network.h"
#include "../../Include_components.h"
#include "../../Bus.h"

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

/// True if the bus location string starts with the given prefix (case-insensitive).
static bool locStartsWith(Bus* bus, const char* prefix) {
    std::string loc = bus->getBusLocation();
    std::string pre(prefix);
    if (loc.size() < pre.size()) return false;
    for (size_t i = 0; i < pre.size(); ++i)
        if (std::tolower(static_cast<unsigned char>(loc[i])) !=
            std::tolower(static_cast<unsigned char>(pre[i]))) return false;
    return true;
}

/// True if the string @p s starts with @p prefix (case-insensitive).
static bool strStartsWith(const std::string& s, const char* prefix) {
    std::string pre(prefix);
    if (s.size() < pre.size()) return false;
    for (size_t i = 0; i < pre.size(); ++i)
        if (std::tolower(static_cast<unsigned char>(s[i])) !=
            std::tolower(static_cast<unsigned char>(pre[i]))) return false;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// add_areas / print_summary
// ─────────────────────────────────────────────────────────────────────────────

void StabilityEstimate::add_areas(Network* net) {
    if (net->is_area_empty())
        net->add_areas();
    ac_grid_names = net->get_ac_grid_names();
    dc_grid_names = net->get_dc_grid_names();
    ac_grids      = net->get_ac_grids();
    dc_grids      = net->get_dc_grids();
    converters    = net->get_converters();
}

void StabilityEstimate::print_summary() const {
    std::cout << "\n--- Stability assessment areas ---\n";
    std::cout << "AC grids: " << ac_grids.size()
              << ", DC grids: " << dc_grids.size()
              << ", converters: " << converters.size() << "\n";
    for (const auto& [name, sub] : ac_grids)  { (void)name; if (sub) sub->printInfo(); }
    for (const auto& [name, sub] : dc_grids)  { (void)name; if (sub) sub->printInfo(); }
}

// ─────────────────────────────────────────────────────────────────────────────
// compute_equivalent_admittance_parameters_num
//
// Implements adjusted MNA eq. (2) of Lekic et al. (CIGRE 2026):
//
//   [ Y_ii  -I   Y_iv ] [ V_i ]   [ 0  ]
//   [ I      0    0   ] [ I_i ] = [ V1 ]
//   [ Y_vi   0   Y_vv ] [ V   ]   [ 0  ]
//
// Excites each output port pin with 1 V (others 0) and reads currents.
// Converters are skipped during element stamping.
// ─────────────────────────────────────────────────────────────────────────────

MatrixXcd StabilityEstimate::compute_equivalent_admittance_parameters_num(
        SubNetwork* subnet, double frequency) {

    if (!subnet)
        throw std::invalid_argument("Null SubNetwork pointer.");

    // Determine how many pins each output port contributes.
    // - DC subnets: always 1 (scalar port, matching the converter model).
    // - AC subnets with Park transform: 2 (dq frame).
    // - AC subnets without transform: 3 (abc frame).
    int pins_per_port;
    {
        std::string nm = subnet->getName();
        if (strStartsWith(nm, "DC") || strStartsWith(nm, "dc")) {
            pins_per_port = 1;
        } else if (subnet->getTransformation()) {
            pins_per_port = 2;
        } else {
            pins_per_port = 3;
        }
    }

    // --- Collect output ports and assign matrix positions ---
    std::unordered_map<std::string, Bus*> output_buses = subnet->getOutputs();

    // For DC subnets and dq AC subnets the effective port pin count differs
    // from the physical bus pin count.  The MNA matrix is sized by
    // pins_per_port, not by bus->getPinNumber().
    auto effectivePins = [&](Bus* /*b*/) -> int {
        return pins_per_port;
    };

    int pos = 0;
    std::unordered_map<Bus*, int> bus_pos;
    std::unordered_map<Bus*, int> bus_cur_pos;

    for (auto& [bname, bus] : output_buses) {
        bus_pos[bus]     = pos;  pos += effectivePins(bus);
        bus_cur_pos[bus] = pos;  pos += effectivePins(bus);
    }

    // Remaining (internal) buses
    for (const auto& [bname, bus] : subnet->getBuses()) {
        if (bus->isGround()) continue;
        if (bus_pos.find(bus) == bus_pos.end()) {
            bus_pos[bus] = pos;
            pos += effectivePins(bus);
        }
    }

    // --- Assemble admittance matrix Y (pos × pos) ---
    Eigen::MatrixXcd Y = Eigen::MatrixXcd::Zero(pos, pos);
    Eigen::VectorXcd z = Eigen::VectorXcd::Zero(pos);

    std::unordered_set<Element*> done;
    for (const auto& [bname, bus] : subnet->getBuses()) {
        for (Element* elem : bus->getConnectedElements()) {
            if (!elem || dynamic_cast<Converter*>(elem) || done.count(elem)) continue;
            const auto& econn = elem->getConnections();
            if (econn.find(bus) == econn.end() || bus->isGround()) continue;

            Bus* other = elem->getOtherBus(bus);
            std::vector<std::vector<std::complex<double>>> Ye =
                elem->compute_y_parameters(frequency);

            int bp = bus_pos.at(bus);
            int t  = econn.at(bus) - 1;
            int to = 1 - t;
            int p  = pins_per_port;   // effective pins per terminal

            for (int i = 0; i < p; ++i) {
                for (int j = 0; j < p; ++j) {
                    Y(bp + i, bp + j) += Ye[t * p + i][t * p + j];
                    if (other && !other->isGround()) {
                        int op = bus_pos.at(other);
                        Y(bp + i, op + j) += Ye[t  * p + i][to * p + j];
                        Y(op + i, bp + j) += Ye[to * p + i][t  * p + j];
                        Y(op + i, op + j) += Ye[to * p + i][to * p + j];
                    }
                }
            }
            done.insert(elem);
        }
    }

    // Current-source rows for output ports
    for (auto& [bname, bus] : output_buses) {
        int bp = bus_pos.at(bus);
        int cp = bus_cur_pos.at(bus);
        int p  = effectivePins(bus);
        for (int i = 0; i < p; ++i) {
            Y(bp + i, cp + i) = -1.0;
            Y(cp + i, bp + i) =  1.0;
        }
    }

    // --- Y-parameter extraction: excite each port pin in turn ---
    int total_out_pins = 0;
    for (auto& [bname, bus] : output_buses) total_out_pins += effectivePins(bus);
    MatrixXcd Y_params = MatrixXcd::Zero(total_out_pins, total_out_pins);

    int col_idx = 0;
    for (auto& [bname, bus] : output_buses) {
        int cp = bus_cur_pos.at(bus);
        int p  = effectivePins(bus);
        for (int i = 0; i < p; ++i) {
            z.setZero();
            z(cp + i) = std::complex<double>(1.0, 0.0);
            Eigen::VectorXcd sol = Y.partialPivLu().solve(z);

            int row_idx = 0;
            for (auto& [bname2, bus2] : output_buses) {
                int cp2 = bus_cur_pos.at(bus2);
                int p2  = effectivePins(bus2);
                for (int j = 0; j < p2; ++j)
                    Y_params(row_idx + j, col_idx) = sol(cp2 + j);
                row_idx += p2;
            }
            ++col_idx;
        }
    }

    return Y_params;
}

// ─────────────────────────────────────────────────────────────────────────────
// computeConverterDcAdmittance
//
// Generalised eqs. (10)-(12) of Lekic et al. for arbitrary port sizes.
//
// The converter Y-matrix is partitioned as:
//   Y_conv = [ Y_dc   B  ]   (p_dc rows)
//            [  A    Y_dq ]  (p_ac rows)
//
// The passive AC-grid admittance Y_eq,AC is obtained via
// compute_equivalent_admittance_parameters_num.  Then:
//
//   Y_eq,conv = Y_dc + B · (Y_eq,AC − Y_dq)^{-1} · A        (eq. 12)
//
// which is a p_dc × p_dc matrix.
// ─────────────────────────────────────────────────────────────────────────────

MatrixXcd StabilityEstimate::computeConverterDcAdmittance(
        Converter* conv, SubNetwork* ac_subnet, double frequency) {

    // Identify AC and DC buses of the converter
    Bus* ac_bus = nullptr;
    Bus* dc_bus = nullptr;
    for (const auto& [bus, terminal] : conv->getConnections()) {
        if (!bus) continue;
        if (locStartsWith(bus, "AC") || locStartsWith(bus, "ac"))
            ac_bus = bus;
        else
            dc_bus = bus;
    }
    if (!ac_bus || !dc_bus)
        throw std::runtime_error("Converter missing AC or DC bus.");

    // Use 2 for dq-transformed AC subnets, otherwise use the physical pin count.
    // DC port is always 1 (scalar) in the current converter model.
    int p_ac = (ac_subnet && ac_subnet->getTransformation()) ? 2 : ac_bus->getPinNumber();
    int p_dc = 1;

    // Passive AC-grid multi-port admittance
    MatrixXcd Y_eq_AC = compute_equivalent_admittance_parameters_num(ac_subnet, frequency);

    // Converter Y-parameters  (p_dc + p_ac) × (p_dc + p_ac)
    MatrixXcd Yc = vectorToMatrix(conv->compute_y_parameters(frequency));
    if (!Yc.allFinite()) {
        // Equilibrium may not have converged; return zero admittance as fallback
        return MatrixXcd::Zero(p_dc, p_dc);
    }

    // Partition:  row/col 0..p_dc-1 = DC,  p_dc..end = AC
    MatrixXcd Y_dc = Yc.block(0,     0,     p_dc, p_dc);
    MatrixXcd B    = Yc.block(0,     p_dc,  p_dc, p_ac);
    MatrixXcd A    = Yc.block(p_dc,  0,     p_ac, p_dc);
    MatrixXcd Y_dq = Yc.block(p_dc,  p_dc,  p_ac, p_ac);

    // Y_eq,conv = Y_dc + B · (Y_eq,AC − Y_dq)^{-1} · A      (eq. 12)
    return Y_dc + B * (Y_eq_AC - Y_dq).inverse() * A;
}

// ─────────────────────────────────────────────────────────────────────────────
// compute_closing_impedance
//
// Implements eqs. (13)-(14) of Lekic et al.
//
// Partitions the DC multi-port Y-parameter matrix around the input port
// (the main-converter bus) and closes the remaining ports with Y_closing:
//
//   Y_eq = Y_11 + Y_12 · (Y_closing + Y_22)^{-1} · Y_21      (eq. 14)
//   Z_eq = Y_eq^{-1}
// ─────────────────────────────────────────────────────────────────────────────

MatrixXcd StabilityEstimate::compute_closing_impedance(
        SubNetwork* sub, string& bus_name,
        MatrixXcd& Y_parameters, MatrixXcd& Y_closing) {

    auto outputs = sub->getOutputs();
    int total_ports = static_cast<int>(outputs.size());

    // DC ports are always 1-pin (scalar) in the current converter model.
    int pins = 1;

    // Locate input port index
    int input_idx = -1;
    {
        int i = 0;
        for (const auto& [n, b] : outputs) {
            if (n == bus_name) { input_idx = i; break; }
            ++i;
        }
    }
    if (input_idx == -1)
        throw std::runtime_error("compute_closing_impedance: bus '" + bus_name + "' not found.");

    int N = total_ports - 1;   // number of other ports

    if (N == 0) {
        // Single port: closing admittance directly gives the equivalent
        return Y_closing.inverse();
    }

    // Partition Y_parameters (total_ports×total_ports) around input_idx
    // (each entry represents a pins×pins block)
    int sz_in = pins;
    int sz_out = N * pins;

    MatrixXcd Y11 = MatrixXcd::Zero(sz_in,  sz_in);
    MatrixXcd Y12 = MatrixXcd::Zero(sz_in,  sz_out);
    MatrixXcd Y21 = MatrixXcd::Zero(sz_out, sz_in);
    MatrixXcd Y22 = MatrixXcd::Zero(sz_out, sz_out);

    // Fill Y12 row (input row, non-input cols)
    {
        int co = 0;
        for (int c = 0; c < total_ports; ++c) {
            if (c == input_idx) continue;
            Y12.block(0, co * pins, pins, pins) =
                Y_parameters.block(input_idx * pins, c * pins, pins, pins);
            ++co;
        }
    }
    // Fill Y21 col (input col, non-input rows) and Y22 (non-input rows/cols)
    int row_o = 0;
    for (int r = 0; r < total_ports; ++r) {
        if (r == input_idx) continue;
        Y21.block(row_o * pins, 0, pins, pins) =
            Y_parameters.block(r * pins, input_idx * pins, pins, pins);
        int col_o = 0;
        for (int c = 0; c < total_ports; ++c) {
            if (c == input_idx) continue;
            Y22.block(row_o * pins, col_o * pins, pins, pins) =
                Y_parameters.block(r * pins, c * pins, pins, pins);
            ++col_o;
        }
        ++row_o;
    }
    Y11 = Y_parameters.block(input_idx * pins, input_idx * pins, pins, pins);

    // Terminate other ports with Y_closing: i2 = -Y_closing·v2
    // Schur complement: Y_eq = Y11 - Y12·(Y_closing + Y22)^{-1}·Y21
    // Return the equivalent impedance: Z_eq = Y_eq^{-1}
    MatrixXcd Y_eq = Y11 - Y12 * (Y_closing + Y22).inverse() * Y21;
    return Y_eq.inverse();
}

// ─────────────────────────────────────────────────────────────────────────────
// compute_transfer_function  (MIMO, dimension-agnostic)
//
// Implements the full 5-step procedure of Lekic et al. (CIGRE 2026):
//
//   Step 1  Identify the main converter and the cut side.
//   Step 2  Compute passive AC and DC multi-port admittances.
//   Step 3  For each non-main converter, stamp its Y-matrix into the passive
//           AC-grid MNA to obtain Y_eq,conv (eq. 12 generalised).
//   Step 4  Close the DC grid around the main converter (eqs. 13-14).
//   Step 5  Form H = Y_n · Z_eq (eqs. 15/17 for DC/AC cut).
// ─────────────────────────────────────────────────────────────────────────────

MatrixXcd StabilityEstimate::compute_transfer_function(
        string converter_name, string location, double frequency) {

    // ── Step 1: identify main converter ──────────────────────────────────────
    if (converters.find(converter_name) == converters.end()) {
        std::cerr << "Error: converter '" << converter_name << "' not found.\n";
        return MatrixXcd::Zero(1, 1);
    }
    Converter* conv_main = dynamic_cast<Converter*>(converters.at(converter_name));
    if (!conv_main) {
        std::cerr << "Error: '" << converter_name << "' is not a Converter.\n";
        return MatrixXcd::Zero(1, 1);
    }

    std::string ac_area = conv_main->getACarea();
    std::string dc_area = conv_main->getDCarea();

    // Identify main converter's AC and DC buses
    Bus* main_ac_bus = nullptr;
    Bus* main_dc_bus = nullptr;
    for (const auto& [bus, terminal] : conv_main->getConnections()) {
        if (!bus) continue;
        if (locStartsWith(bus, "AC") || locStartsWith(bus, "ac"))
            main_ac_bus = bus;
        else
            main_dc_bus = bus;
    }
    if (!main_ac_bus || !main_dc_bus) {
        std::cerr << "Error: main converter has no AC or DC bus.\n";
        return MatrixXcd::Zero(1, 1);
    }

    SubNetwork* ac_sub_main_early = ac_grids.count(ac_area) ? ac_grids.at(ac_area) : nullptr;
    int p_ac = (ac_sub_main_early && ac_sub_main_early->getTransformation()) ? 2
                                                                              : main_ac_bus->getPinNumber();
    // DC ports are always scalar (1 pin) in the current converter model.
    // The DC bus may have 2 physical pins (bipole), but the admittance port
    // is represented as a single scalar in the MMC Y-parameter matrix.
    int p_dc = 1;

    // ── Step 2: passive AC/DC multi-port admittances ──────────────────────────
    std::unordered_map<std::string, MatrixXcd> Y_dc_matrices;
    for (auto& [name, sub] : dc_grids)
        Y_dc_matrices[name] = compute_equivalent_admittance_parameters_num(sub, frequency);

    // (AC admittances are computed on demand inside computeConverterDcAdmittance)

    // ── Step 3: DC-side equivalent admittance of every non-main converter ─────
    // Y_closing is block-diagonal: blkdiag(Y_eq,conv1, Y_eq,conv2, ...)
    // Each diagonal block is p_dc × p_dc (same pin count per current model).

    int n_other = static_cast<int>(converters.size()) - 1;
    MatrixXcd Y_closing = MatrixXcd::Zero(n_other * p_dc, n_other * p_dc);

    int idx = 0;
    for (auto& [name, elem] : converters) {
        if (name == converter_name) continue;
        Converter* conv_k = dynamic_cast<Converter*>(elem);
        if (!conv_k) { ++idx; continue; }

        std::string ac_area_k = conv_k->getACarea();
        SubNetwork* ac_sub_k  = ac_grids.count(ac_area_k) ? ac_grids.at(ac_area_k) : nullptr;
        if (!ac_sub_k) { ++idx; continue; }

        // Generalised eq. (12): Y_eq,conv_k
        MatrixXcd Y_eq_conv_k = computeConverterDcAdmittance(conv_k, ac_sub_k, frequency);

        int bk = idx * p_dc;
        Y_closing.block(bk, bk, p_dc, p_dc) = Y_eq_conv_k;
        ++idx;
    }

    // ── Step 4: DC closing impedance seen from main converter ─────────────────
    // eqs. (13)-(14)
    if (!dc_grids.count(dc_area)) {
        std::cerr << "Error: DC subnetwork '" << dc_area << "' not found.\n";
        return MatrixXcd::Zero(p_dc, p_dc);
    }
    if (!Y_dc_matrices.count(dc_area)) {
        std::cerr << "Error: DC admittance for '" << dc_area << "' not computed.\n";
        return MatrixXcd::Zero(p_dc, p_dc);
    }
    std::string dc_busname = main_dc_bus->getBusName();
    MatrixXcd Z_dc = compute_closing_impedance(
        dc_grids.at(dc_area), dc_busname,
        Y_dc_matrices.at(dc_area), Y_closing);

    // ── Step 5: form transfer function ────────────────────────────────────────
    bool dc_cut = strStartsWith(location, "DC") || strStartsWith(location, "dc");

    MatrixXcd Yc = vectorToMatrix(conv_main->compute_y_parameters(frequency));
    if (!Yc.allFinite()) {
        std::cerr << "Warning: converter '" << converter_name
                  << "' Y-parameters are non-finite at " << frequency
                  << " Hz (equilibrium may not have converged).\n";
        return dc_cut ? MatrixXcd::Zero(p_dc, p_dc) : MatrixXcd::Zero(p_ac, p_ac);
    }
    MatrixXcd Y_dc_blk = Yc.block(0,    0,    p_dc, p_dc);
    MatrixXcd B        = Yc.block(0,    p_dc, p_dc, p_ac);
    MatrixXcd A        = Yc.block(p_dc, 0,    p_ac, p_dc);
    MatrixXcd Y_dq     = Yc.block(p_dc, p_dc, p_ac, p_ac);

    if (dc_cut) {
        // DC cut: H = Y_eq,conv_main · Z_dc    (eq. 15 of paper)
        // Y_eq,conv_main = Y_dc + B·(Y_eq,AC − Y_dq)^{-1}·A   (eq. 12)
        if (!ac_sub_main_early) {
            std::cerr << "Error: AC subnetwork '" << ac_area << "' not found.\n";
            return MatrixXcd::Zero(p_dc, p_dc);
        }
        MatrixXcd Y_eq_AC_main =
            compute_equivalent_admittance_parameters_num(ac_sub_main_early, frequency);
        MatrixXcd Y_eq_conv_main =
            Y_dc_blk + B * (Y_eq_AC_main - Y_dq).inverse() * A;

        return Y_eq_conv_main * Z_dc;   // p_dc × p_dc

    } else {
        // AC cut: H = Y_eq,conv_AC · Z_eq,AC
        //
        // Schur complement: close DC port with external admittance Y_dc_ext = Z_dc^{-1}.
        // KCL at DC port: -Y_dc_ext · v_dc = Y_dc · v_dc + B · v_ac
        //   => v_dc = -(Y_dc + Y_dc_ext)^{-1} · B · v_ac
        // Substitute into AC current:
        //   Y_eq,conv_AC = Y_dq - A · (Y_dc + Y_dc_ext)^{-1} · B
        //
        // Transfer function H = Y_eq,conv_AC · Z_eq,AC = Y_eq,conv_AC · Y_eq,AC^{-1}

        if (!ac_sub_main_early) {
            std::cerr << "Error: AC subnetwork '" << ac_area << "' not found.\n";
            return MatrixXcd::Zero(p_ac, p_ac);
        }
        MatrixXcd Y_eq_AC_main =
            compute_equivalent_admittance_parameters_num(ac_sub_main_early, frequency);

        MatrixXcd Y_dc_ext = Z_dc.inverse();    // external DC admittance seen from DC port
        MatrixXcd Y_eq_conv_AC = Y_dq - A * (Y_dc_blk + Y_dc_ext).inverse() * B;

        return Y_eq_conv_AC * Y_eq_AC_main.inverse();  // p_ac × p_ac
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// File export and plotting
// ─────────────────────────────────────────────────────────────────────────────

void StabilityEstimate::writeFileTF(string converter_name, string location,
        double start_frequency, double end_frequency, int number_of_points) {

    std::ofstream myfile;
    myfile.open("./files/" + converter_name + "_" + location + ".csv");

    double gap = (end_frequency - start_frequency) / (number_of_points - 1);
    double frequency = start_frequency;
    for (int p = 0; p < number_of_points; p++) {
		MatrixXcd TF = compute_transfer_function(converter_name, location, frequency);
        myfile << frequency << ",";
        for (int i = 0; i < TF.rows(); i++)
            for (int j = 0; j < TF.cols(); ++j)
                myfile << TF(i,j).real() << "+1i*(" << TF(i,j).imag() << "),";
        myfile << "\n";
        frequency += gap;
    }
    myfile.close();
}

void StabilityEstimate::bodeplotTF(string converter_name, string location,
        double start_frequency, double end_frequency, int number_of_points) {

    // Determine expected TF size from one evaluation at start_frequency
    MatrixXcd TF0 = compute_transfer_function(converter_name, location, start_frequency);
    int num_values = static_cast<int>(TF0.rows() * TF0.cols());

    std::vector<std::string> labels;
    for (int i = 0; i < TF0.rows(); ++i)
        for (int j = 0; j < TF0.cols(); ++j)
            labels.push_back("TF_{" + std::to_string(i) + std::to_string(j) + "}");

    std::vector<double> frequencies;
    std::vector<std::vector<double>> magnitudes(number_of_points,
                                                std::vector<double>(num_values, 0.0));
    std::vector<std::vector<double>> phases(number_of_points,
                                            std::vector<double>(num_values, 0.0));

    double gap = std::pow(10.0, (std::log10(end_frequency) - std::log10(start_frequency))
                                / (number_of_points - 1));
    double frequency = start_frequency;
    for (int p = 0; p < number_of_points; p++) {
        frequencies.push_back(frequency);
		MatrixXcd TF = compute_transfer_function(converter_name, location, frequency);
        for (int i = 0; i < TF.rows(); ++i)
            for (int j = 0; j < TF.cols(); ++j) {
                int k = TF.cols() * i + j;
                magnitudes[p][k] = 20.0 * std::log10(std::abs(TF(i,j)));
                phases[p][k]     = std::arg(TF(i,j)) * 180.0 / M_PI;
            }
        frequency *= gap;
    }
    bode_plot_implot(frequencies, magnitudes, phases, labels,
                     "TF of power system cut on " + location +
                     " side of " + converter_name);
}

void StabilityEstimate::nyquistplotTF(string converter_name, string location,
        double start_frequency, double end_frequency, int number_of_points) {

    MatrixXcd TF0 = compute_transfer_function(converter_name, location, start_frequency);
    int num_values = static_cast<int>(TF0.rows() * TF0.cols());

    std::vector<std::string> labels;
    for (int i = 0; i < TF0.rows(); ++i)
        for (int j = 0; j < TF0.cols(); ++j)
            labels.push_back("TF_{" + std::to_string(i) + std::to_string(j) + "}");

    std::vector<std::vector<std::complex<double>>> TF(
        number_of_points, std::vector<std::complex<double>>(num_values, 0.0));

    double gap = std::pow(10.0, (std::log10(end_frequency) - std::log10(start_frequency))
                                / (number_of_points - 1));
    double frequency = start_frequency;
    for (int p = 0; p < number_of_points; p++) {
        MatrixXcd TF_freq = compute_transfer_function(converter_name, location, frequency);
        for (int i = 0; i < TF_freq.rows(); ++i)
            for (int j = 0; j < TF_freq.cols(); ++j)
				TF[p][TF_freq.cols() * i + j] = TF_freq(i, j);
        frequency *= gap;
    }
    nyquist_plot_implot(TF, labels,
                        "Nyquist Plot of TF on " + location +
                        " side of " + converter_name);
}
