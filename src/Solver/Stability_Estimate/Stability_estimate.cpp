/**
 * @file Stability_estimate.cpp
 * @brief MIMO impedance-based stability assessment via unified MNA.
 *
 * All port dimensions are derived at runtime from Bus::getPinNumber(), so
 * the implementation handles 1-pin (DC scalar), 2-pin (dq AC) and 3-pin
 * (abc AC) buses without any hard-coded dimension assumptions.
 */
#include "Stability_estimate.h"

#include "network/network.h"
#include "core/Include_components.h"
#include "network/Bus.h"
#include "ui/Visualization.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <unordered_set>

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

/// Solve A X = B. Ridge only if A is singular or non-finite (ideal source).
static MatrixXcd solveRegularized(const MatrixXcd& A, const MatrixXcd& B)
{
    if (A.size() == 0)
        throw std::runtime_error("solveRegularized: empty matrix.");
    if (A.rows() != A.cols())
        throw std::runtime_error("solveRegularized: A must be square.");
    if (B.rows() != A.rows())
        throw std::runtime_error("solveRegularized: right-hand side row mismatch.");
    MatrixXcd Ause = A;
    if (!Ause.allFinite())
        Ause = MatrixXcd::Identity(A.rows(), A.cols()) * 1e-12;
    Eigen::FullPivLU<MatrixXcd> lu(Ause);
    if (!lu.isInvertible()) {
        Ause += MatrixXcd::Identity(Ause.rows(), Ause.cols()) * 1e-12;
        lu.compute(Ause);
    }
    return lu.solve(B);
}

/// Invert Y via LU solve against I (never form Y.inverse() explicitly).
static MatrixXcd invertRegularized(const MatrixXcd& Y)
{
    return solveRegularized(Y, MatrixXcd::Identity(Y.rows(), Y.cols()));
}

/// H = Yn · Yeq^{-1} by solving Yeq^T H^T = Yn^T.
static MatrixXcd mulRightInverse(const MatrixXcd& Yn, const MatrixXcd& Yeq)
{
    return solveRegularized(Yeq.transpose(), Yn.transpose()).transpose();
}

static std::complex<double> yAt(const std::vector<std::vector<std::complex<double>>>& Ye, int r, int c)
{
    if (r < 0 || c < 0 || r >= static_cast<int>(Ye.size()))
        return { 0.0, 0.0 };
    if (Ye[static_cast<size_t>(r)].empty() || c >= static_cast<int>(Ye[static_cast<size_t>(r)].size()))
        return { 0.0, 0.0 };
    return Ye[static_cast<size_t>(r)][static_cast<size_t>(c)];
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

// Matches Element::compute_y_parameters (omega_0 = 100π).
static constexpr double kParkF0Hz = 50.0;

// ─────────────────────────────────────────────────────────────────────────────
// assemblePassivePortY
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

static MatrixXcd assemblePassivePortY(
        SubNetwork* subnet, double frequency, int pins_per_port, bool abc_y)
{
    std::unordered_map<std::string, Bus*> output_buses = subnet->getOutputs();

    int pos = 0;
    std::unordered_map<Bus*, int> bus_pos;
    std::unordered_map<Bus*, int> bus_cur_pos;

    for (auto& [bname, bus] : output_buses) {
        bus_pos[bus]     = pos;  pos += pins_per_port;
        bus_cur_pos[bus] = pos;  pos += pins_per_port;
    }

    for (const auto& [bname, bus] : subnet->getBuses()) {
        if (bus->isGround()) continue;
        if (bus_pos.find(bus) == bus_pos.end()) {
            bus_pos[bus] = pos;
            pos += pins_per_port;
        }
    }

    Eigen::MatrixXcd Y = Eigen::MatrixXcd::Zero(pos, pos);
    Eigen::VectorXcd z = Eigen::VectorXcd::Zero(pos);

    std::unordered_set<Element*> done;
    for (const auto& [bname, bus] : subnet->getBuses()) {
        for (Element* elem : bus->getConnectedElements()) {
            if (!elem || dynamic_cast<Converter*>(elem) || done.count(elem)) continue;
            const auto& econn = elem->getConnections();
            if (econn.find(bus) == econn.end() || bus->isGround()) continue;

            Bus* other = elem->getOtherBus(bus);
            std::vector<std::vector<std::complex<double>>> Ye = abc_y
                ? elem->compute_y_parameters_abc(frequency)
                : elem->compute_y_parameters(frequency);

            int bp = bus_pos.at(bus);
            int t  = econn.at(bus) - 1;
            int to = 1 - t;
            int p  = pins_per_port;

            for (int i = 0; i < p; ++i) {
                for (int j = 0; j < p; ++j) {
                    Y(bp + i, bp + j) += yAt(Ye, t * p + i, t * p + j);
                    if (other && !other->isGround()) {
                        int op = bus_pos.at(other);
                        Y(bp + i, op + j) += yAt(Ye, t  * p + i, to * p + j);
                        Y(op + i, bp + j) += yAt(Ye, to * p + i, t  * p + j);
                        Y(op + i, op + j) += yAt(Ye, to * p + i, to * p + j);
                    }
                }
            }
            done.insert(elem);
        }
    }

    for (auto& [bname, bus] : output_buses) {
        int bp = bus_pos.at(bus);
        int cp = bus_cur_pos.at(bus);
        for (int i = 0; i < pins_per_port; ++i) {
            Y(bp + i, cp + i) = -1.0;
            Y(cp + i, bp + i) =  1.0;
        }
    }

    int total_out_pins = static_cast<int>(output_buses.size()) * pins_per_port;
    MatrixXcd Y_params = MatrixXcd::Zero(total_out_pins, total_out_pins);

    int col_idx = 0;
    for (auto& [bname, bus] : output_buses) {
        int cp = bus_cur_pos.at(bus);
        for (int i = 0; i < pins_per_port; ++i) {
            z.setZero();
            z(cp + i) = std::complex<double>(1.0, 0.0);
            Eigen::VectorXcd sol = Y.partialPivLu().solve(z);

            int row_idx = 0;
            for (auto& [bname2, bus2] : output_buses) {
                int cp2 = bus_cur_pos.at(bus2);
                for (int j = 0; j < pins_per_port; ++j)
                    Y_params(row_idx + j, col_idx) = sol(cp2 + j);
                row_idx += pins_per_port;
            }
            ++col_idx;
        }
    }

    return Y_params;
}

MatrixXcd StabilityEstimate::compute_equivalent_admittance_parameters_num(
        SubNetwork* subnet, double frequency) {
    return compute_equivalent_admittance_parameters_num(
        subnet, frequency, park_per_component_, yeff_);
}

MatrixXcd StabilityEstimate::compute_equivalent_admittance_parameters_num(
        SubNetwork* subnet, double frequency, bool park_per_component) {
    return compute_equivalent_admittance_parameters_num(
        subnet, frequency, park_per_component, false);
}

MatrixXcd StabilityEstimate::compute_equivalent_admittance_parameters_num(
        SubNetwork* subnet, double frequency, bool park_per_component, bool yeff) {

    if (!subnet)
        throw std::invalid_argument("Null SubNetwork pointer.");

    const std::string nm = subnet->getName();
    const bool is_dc = strStartsWith(nm, "DC") || strStartsWith(nm, "dc");

    if (is_dc)
        return assemblePassivePortY(subnet, frequency, 1, false);

    if (!subnet->getTransformation())
        return assemblePassivePortY(subnet, frequency, 3, true);

    // AC + Park: Yeff and block A0 assemble abc Y; per-component stamps dq Y.
    if (yeff) {
        const double f0 = kParkF0Hz;
        MatrixXcd Y_m3 = assemblePassivePortY(subnet, frequency - 3.0 * f0, 3, true);
        MatrixXcd Y_m1 = assemblePassivePortY(subnet, frequency - f0, 3, true);
        MatrixXcd Y_p1 = assemblePassivePortY(subnet, frequency + f0, 3, true);
        MatrixXcd Y_p3 = assemblePassivePortY(subnet, frequency + 3.0 * f0, 3, true);
        return vectorToMatrix(apply_park_Yeff(
            matrixToVector(Y_m3), matrixToVector(Y_m1),
            matrixToVector(Y_p1), matrixToVector(Y_p3)));
    }

    if (park_per_component)
        return assemblePassivePortY(subnet, frequency, 2, false);

    MatrixXcd Y_minus = assemblePassivePortY(subnet, frequency - kParkF0Hz, 3, true);
    MatrixXcd Y_plus  = assemblePassivePortY(subnet, frequency + kParkF0Hz, 3, true);
    return vectorToMatrix(apply_park_A0(matrixToVector(Y_minus), matrixToVector(Y_plus)));
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
        return MatrixXcd::Zero(p_dc, p_dc);
    }
    if (Yc.rows() < p_dc + p_ac || Yc.cols() < p_dc + p_ac) {
        throw std::runtime_error("Converter Y-matrix is smaller than p_dc+p_ac.");
    }
    if (Y_eq_AC.rows() != p_ac || Y_eq_AC.cols() != p_ac) {
        throw std::runtime_error(
            "AC equivalent Y is " + std::to_string(Y_eq_AC.rows()) + "x"
            + std::to_string(Y_eq_AC.cols()) + ", expected " + std::to_string(p_ac)
            + "x" + std::to_string(p_ac) + ".");
    }

    MatrixXcd Y_dc = Yc.block(0,     0,     p_dc, p_dc);
    MatrixXcd B    = Yc.block(0,     p_dc,  p_dc, p_ac);
    MatrixXcd A    = Yc.block(p_dc,  0,     p_ac, p_dc);
    MatrixXcd Y_dq = Yc.block(p_dc,  p_dc,  p_ac, p_ac);

    return Y_dc + B * solveRegularized(Y_eq_AC - Y_dq, A);
}

// ─────────────────────────────────────────────────────────────────────────────
// compute_closing_admittance
//
// Implements eqs. (13)-(14) of Lekic et al.
//
// Partitions the DC multi-port Y-parameter matrix around the input port
// (the main-converter bus) and closes the remaining ports with Y_closing.
// With i2 = −Y_closing·v2 the Schur complement is
//
//   Y_eq = Y_11 − Y_12 · (Y_closing + Y_22)^{-1} · Y_21      (eq. 14)
//
// The admittance is returned directly so later steps do not invert Y→Z→Y.
// ─────────────────────────────────────────────────────────────────────────────

MatrixXcd StabilityEstimate::compute_closing_admittance(
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
        throw std::runtime_error("compute_closing_admittance: bus '" + bus_name + "' not found.");

    int N = total_ports - 1;   // number of other ports

    if (N == 0) {
        // Single converter: no other DC ports to terminate. Closing admittance
        // is the passive DC Y at this port.
        if (Y_parameters.size() == 0)
            throw std::runtime_error("compute_closing_admittance: empty DC Y-parameters.");
        return Y_parameters;
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
    return Y11 - Y12 * solveRegularized(Y_closing + Y22, Y21);
}

static double logFrequency(double start_frequency, double end_frequency,
        int number_of_points, int p)
{
    const int n = std::max(number_of_points, 2);
    if (start_frequency <= 0.0 || end_frequency <= 0.0) {
        return start_frequency + p * (end_frequency - start_frequency) / (n - 1);
    }
    const double ratio = std::pow(end_frequency / start_frequency,
            static_cast<double>(p) / static_cast<double>(n - 1));
    return start_frequency * ratio;
}

static void writeComplexMatrixCsv(const std::string& path,
        int number_of_points, double start_frequency, double end_frequency,
        const std::function<MatrixXcd(double)>& eval)
{
    std::filesystem::create_directories("./files");
    std::ofstream myfile(path);
    if (!myfile)
        throw std::runtime_error("cannot open " + path);
    const int n = std::max(number_of_points, 2);
    for (int p = 0; p < n; ++p) {
        const double frequency = logFrequency(start_frequency, end_frequency, n, p);
        MatrixXcd M = eval(frequency);
        myfile << frequency << ",";
        for (int i = 0; i < M.rows(); ++i)
            for (int j = 0; j < M.cols(); ++j)
                myfile << M(i, j).real() << "+1i*(" << M(i, j).imag() << "),";
        myfile << "\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// compute_cut_admittances  (MIMO, dimension-agnostic)
//
// Implements the full 5-step procedure of Lekic et al. (CIGRE 2026):
//
//   Step 1  Identify the main converter and the cut side.
//   Step 2  Compute passive AC and DC multi-port admittances.
//   Step 3  For each non-main converter, stamp its Y-matrix into the passive
//           AC-grid MNA to obtain Y_eq,conv (eq. 12 generalised).
//   Step 4  Close the DC grid around the main converter (eqs. 13-14).
//   Step 5  Return Yn (converter, other port closed) and Yeq (grid).
//           H = Yn · Yeq^{-1},  Zin = (Yn + Yeq)^{-1}.
// ─────────────────────────────────────────────────────────────────────────────

bool StabilityEstimate::compute_cut_admittances(
        string converter_name, string location, double frequency,
        MatrixXcd& Yn, MatrixXcd& Yeq) {

    // ── Step 1: identify main converter ──────────────────────────────────────
    if (converters.find(converter_name) == converters.end()) {
        std::cerr << "Error: converter '" << converter_name << "' not found.\n";
        return false;
    }
    Converter* conv_main = dynamic_cast<Converter*>(converters.at(converter_name));
    if (!conv_main) {
        std::cerr << "Error: '" << converter_name << "' is not a Converter.\n";
        return false;
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
        return false;
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
        return false;
    }
    if (!Y_dc_matrices.count(dc_area)) {
        std::cerr << "Error: DC admittance for '" << dc_area << "' not computed.\n";
        return false;
    }
    std::string dc_busname = main_dc_bus->getBusName();
    MatrixXcd Y_dc_ext = compute_closing_admittance(
        dc_grids.at(dc_area), dc_busname,
        Y_dc_matrices.at(dc_area), Y_closing);

    // ── Step 5: form transfer function ────────────────────────────────────────
    bool dc_cut = strStartsWith(location, "DC") || strStartsWith(location, "dc");

    MatrixXcd Yc = vectorToMatrix(conv_main->compute_y_parameters(frequency));
    if (!Yc.allFinite()) {
        std::cerr << "Warning: converter '" << converter_name
                  << "' Y-parameters are non-finite at " << frequency
                  << " Hz (equilibrium may not have converged).\n";
        Yn = dc_cut ? MatrixXcd::Zero(p_dc, p_dc) : MatrixXcd::Zero(p_ac, p_ac);
        Yeq = Yn;
        return false;
    }
    if (Yc.rows() < p_dc + p_ac || Yc.cols() < p_dc + p_ac) {
        throw std::runtime_error("Converter Y-matrix is smaller than p_dc+p_ac.");
    }
    MatrixXcd Y_dc_blk = Yc.block(0,    0,    p_dc, p_dc);
    MatrixXcd B        = Yc.block(0,    p_dc, p_dc, p_ac);
    MatrixXcd A        = Yc.block(p_dc, 0,    p_ac, p_dc);
    MatrixXcd Y_dq     = Yc.block(p_dc, p_dc, p_ac, p_ac);

    if (dc_cut) {
        // DC cut: H = Y_eq,conv_main · Z_dc    (eq. 15 of paper)
        // Y_eq,conv_main = Y_dc + B·(Y_eq,AC − Y_dq)^{-1}·A   (eq. 12)
        // Yeq is the DC closing admittance; H = Yn · Yeq^{-1} uses one LU solve.
        if (!ac_sub_main_early) {
            std::cerr << "Error: AC subnetwork '" << ac_area << "' not found.\n";
            return false;
        }
        MatrixXcd Y_eq_AC_main =
            compute_equivalent_admittance_parameters_num(ac_sub_main_early, frequency);
        Yn = Y_dc_blk + B * solveRegularized(Y_eq_AC_main - Y_dq, A);
        Yeq = Y_dc_ext;
        return true;

    } else {
        // AC cut: H = Y_eq,conv_AC · Z_eq,AC
        //
        // Schur complement: close DC port with the DC-grid admittance Y_dc_ext.
        // KCL at DC port: -Y_dc_ext · v_dc = Y_dc · v_dc + B · v_ac
        //   => v_dc = -(Y_dc + Y_dc_ext)^{-1} · B · v_ac
        // Substitute into AC current:
        //   Y_eq,conv_AC = Y_dq - A · (Y_dc + Y_dc_ext)^{-1} · B
        //
        // Transfer function H = Y_eq,conv_AC · Z_eq,AC = Y_eq,conv_AC · Y_eq,AC^{-1}

        if (!ac_sub_main_early) {
            std::cerr << "Error: AC subnetwork '" << ac_area << "' not found.\n";
            return false;
        }
        MatrixXcd Y_eq_AC_main =
            compute_equivalent_admittance_parameters_num(ac_sub_main_early, frequency);

        Yn = Y_dq - A * solveRegularized(Y_dc_blk + Y_dc_ext, B);
        Yeq = Y_eq_AC_main;
        return true;
    }
}

MatrixXcd StabilityEstimate::compute_transfer_function(
        string converter_name, string location, double frequency) {
    MatrixXcd Yn, Yeq;
    if (!compute_cut_admittances(converter_name, location, frequency, Yn, Yeq)) {
        if (Yn.size() == 0)
            return MatrixXcd::Zero(1, 1);
        return MatrixXcd::Zero(Yn.rows(), Yn.cols());
    }
    return mulRightInverse(Yn, Yeq);
}

MatrixXcd StabilityEstimate::compute_driving_point_impedance(
        string converter_name, string location, double frequency) {
    MatrixXcd Yn, Yeq;
    if (!compute_cut_admittances(converter_name, location, frequency, Yn, Yeq)) {
        if (Yn.size() == 0)
            return MatrixXcd::Zero(1, 1);
        return MatrixXcd::Zero(Yn.rows(), Yn.cols());
    }
    return invertRegularized(Yn + Yeq);
}

// ─────────────────────────────────────────────────────────────────────────────
// File export and plotting
// ─────────────────────────────────────────────────────────────────────────────

void StabilityEstimate::writeFileTF(string converter_name, string location,
        double start_frequency, double end_frequency, int number_of_points) {
    const std::string tag = yeff_ ? "_yeff" : "";
    writeComplexMatrixCsv(
        "./files/" + converter_name + "_" + location + tag + ".csv",
        number_of_points, start_frequency, end_frequency,
        [&](double frequency) {
            return compute_transfer_function(converter_name, location, frequency);
        });
}

void StabilityEstimate::writeFileZin(string converter_name, string location,
        double start_frequency, double end_frequency, int number_of_points) {
    const std::string tag = yeff_ ? "_yeff" : "";
    writeComplexMatrixCsv(
        "./files/" + converter_name + "_" + location + tag + "_Zin.csv",
        number_of_points, start_frequency, end_frequency,
        [&](double frequency) {
            return compute_driving_point_impedance(converter_name, location, frequency);
        });
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
