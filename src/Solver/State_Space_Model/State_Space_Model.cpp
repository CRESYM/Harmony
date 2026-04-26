#include "State_Space_Model.h"
#include "../../network.h"      
#include "../../Include_components.h"

static int getStateCount(Element* e) {
    int n = e->getNumberOfInternalStates();
    return (n > 0) ? n : e->getInputPins();
}

void StateSpaceModel::finalizeCounts(Network* net) {
    std::unordered_map<std::string, Element*> elements = net->getElements();
    std::unordered_map<std::string, Bus*> buses = net->getBuses();

    bus_indices.clear(); element_indices.clear();
    list_independent_sources.clear(); list_state_variables.clear();
    list_switches.clear(); list_virtual_input_providers.clear();
    virtual_input_bank.clear(); input_groups.clear();
    number_switches = 0; number_independent_sources = 0;
    number_state_variables = 0; number_virtual_inputs = 0; number_nodes = 0;

    for (const auto& [name, element] : elements) {
        if (dynamic_cast<Inductor*>(element) || dynamic_cast<Capacitor*>(element)) {
            number_state_variables += element->getInputPins();
            list_state_variables.push_back(element);
        }
        else if (dynamic_cast<Converter*>(element)) {
            number_state_variables += element->getNumberOfInternalStates();
            list_state_variables.push_back(element);
            auto vi = element->getVirtualInputSymbols();
            if (!vi.empty()) {
                number_virtual_inputs += static_cast<int>(vi.size());
                list_virtual_input_providers.push_back(element);
                virtual_input_bank[element] = vi;
            }
        }
        else if (dynamic_cast<Switch*>(element)) {
            number_switches += element->getInputPins();
            list_switches.push_back(element);
        }
        else if (dynamic_cast<Source_base*>(element)) {
            number_independent_sources += element->getInputPins();
            list_independent_sources.push_back(element);
        }
    }

    // Deterministic ordering
    auto byName = [](Element* a, Element* b) {
        return a->getElementSymbol() < b->getElementSymbol();
        };
    std::sort(list_independent_sources.begin(), list_independent_sources.end(), byName);
    std::sort(list_switches.begin(), list_switches.end(), byName);
    std::sort(list_state_variables.begin(), list_state_variables.end(), byName);
    std::sort(list_virtual_input_providers.begin(), list_virtual_input_providers.end(), byName);

    for (const auto& [name, busPtr] : buses) {
        if (busPtr->isGround()) continue;
        bus_indices[busPtr] = number_nodes;
        number_nodes += busPtr->getPinNumber();
    }

    number_outputs = 0;
    for (const auto& bus : output) {
        if (bus->isGround()) continue;
        number_outputs += bus->getPinNumber();
    }

    int location = number_nodes;
    for (const auto& element : list_independent_sources) {
        element_indices[element] = location;
        location += element->getInputPins();
    }
    for (const auto& element : list_switches) {
        element_indices[element] = location;
        location += element->getInputPins();
    }
    for (const auto& element : list_state_variables) {
        element_indices[element] = location;
        int n = element->getNumberOfInternalStates();
        location += (n > 0) ? n : element->getInputPins();
    }

    total_number_equations = number_nodes + number_independent_sources
        + number_switches + number_state_variables;

    std::cout << "[SSM] Nodes:" << number_nodes
        << " Sources:" << number_independent_sources
        << " VirtInputs:" << number_virtual_inputs
        << " States:" << number_state_variables
        << " Total:" << total_number_equations << std::endl;
}

void StateSpaceModel::substituteParameters(SymEngine::DenseMatrix& matrix) {
    using namespace SymEngine;
    for (const auto& element : list_state_variables) {
        map_basic_basic subs_map = element->getParameterSubstitutions();
        if (subs_map.empty()) continue;
        for (int i = 0; i < total_number_equations; ++i) {
            RCP<const Basic> val = matrix.get(i, total_number_equations);
            val = subs(val, subs_map);
            matrix.set(i, total_number_equations, val);
        }
    }
}

// ===================================================================
//  Optimized extraction helper — batch substitution
// ===================================================================
//
//  Instead of substituting symbols one-at-a-time (N_syms calls per entry),
//  build a full substitution map and do ONE subs() call per (i,j).
//
// ===================================================================
//  extractCoefficient — handles both symbol banks
// ===================================================================

static double extractCoefficient(
    const RCP<const Basic>& expression,
    const std::map<Element*, std::vector<RCP<const Basic>>>& symbols_bank,
    const std::map<Element*, std::vector<RCP<const Basic>>>& virtual_input_bank,
    Element* target_elem, int target_sym_idx, bool is_virtual)
{
    map_basic_basic sub;
    for (auto& [elem, syms] : symbols_bank)
        for (auto& s : syms) sub[s] = real_double(0.0);
    for (auto& [elem, syms] : virtual_input_bank)
        for (auto& s : syms) sub[s] = real_double(0.0);

    if (is_virtual) {
        auto it = virtual_input_bank.find(target_elem);
        if (it != virtual_input_bank.end() && target_sym_idx < (int)it->second.size())
            sub[it->second[target_sym_idx]] = real_double(1.0);
    }
    else {
        auto it = symbols_bank.find(target_elem);
        if (it != symbols_bank.end() && target_sym_idx < (int)it->second.size())
            sub[it->second[target_sym_idx]] = real_double(1.0);
    }

    RCP<const Basic> val = SymEngine::subs(expression, sub);
    return eval_basic(val);
}



void StateSpaceModel::formState(Network* net, const vector<Bus*>& out, SSMMode mode) {
    output = out;
    finalizeCounts(net);
    std::unordered_map<std::string, Element*> elements = net->getElements();

    SymEngine::DenseMatrix matrix = createZeroMatrix(
        total_number_equations, total_number_equations + 1);

    for (const auto& [name, element] : elements) {
        int location = (element_indices.count(element) != 0)
            ? element_indices[element] : 0;
        element->writeMNAmatrix(matrix, bus_indices, location, symbols_bank);
    }

    std::cout << "[StateSpaceModel] MNA populated. RREF..." << std::endl;
    vec_uint pivot_cols;
    reduced_row_echelon_form(matrix, matrix, pivot_cols);

    substituteParameters(matrix);

    A = Eigen::MatrixXd::Zero(number_state_variables, number_state_variables);
    int nu_raw = number_independent_sources + number_virtual_inputs;
    B = Eigen::MatrixXd::Zero(number_state_variables, nu_raw);
    C = Eigen::MatrixXd::Zero(number_outputs, number_state_variables);
    D = Eigen::MatrixXd::Zero(number_outputs, nu_raw);

    // ---- A matrix ----
    int row_off = 0;
    for (int i = 0; i < (int)list_state_variables.size(); i++) {
        int loc_i = element_indices[list_state_variables[i]];
        int n_i = getStateCount(list_state_variables[i]);
        int col_off = 0;
        for (int j = 0; j < (int)list_state_variables.size(); j++) {
            int n_j = getStateCount(list_state_variables[j]);
            for (int ki = 0; ki < n_i; ++ki) {
                RCP<const Basic> base = matrix.get(loc_i + ki, total_number_equations);
                for (int kj = 0; kj < n_j; ++kj) {
                    A(row_off + ki, col_off + kj) =
                        extractCoefficient(base, symbols_bank, virtual_input_bank,
                            list_state_variables[j], kj, false);
                }
            }
            col_off += n_j;
        }
        row_off += n_i;
    }

    // ---- B matrix ----
    row_off = 0;
    for (int i = 0; i < (int)list_state_variables.size(); i++) {
        int loc_i = element_indices[list_state_variables[i]];
        int n_i = getStateCount(list_state_variables[i]);
        int col_off = 0;

        // Source columns
        for (int j = 0; j < (int)list_independent_sources.size(); j++) {
            int n_j = list_independent_sources[j]->getInputPins();
            for (int ki = 0; ki < n_i; ++ki) {
                RCP<const Basic> base = matrix.get(loc_i + ki, total_number_equations);
                for (int kj = 0; kj < n_j; ++kj)
                    B(row_off + ki, col_off + kj) =
                    extractCoefficient(base, symbols_bank, virtual_input_bank,
                        list_independent_sources[j], kj, false);
            }
            col_off += n_j;
        }

        // Virtual input columns
        for (int j = 0; j < (int)list_virtual_input_providers.size(); j++) {
            int n_vi = static_cast<int>(virtual_input_bank[list_virtual_input_providers[j]].size());
            for (int ki = 0; ki < n_i; ++ki) {
                RCP<const Basic> base = matrix.get(loc_i + ki, total_number_equations);
                for (int kj = 0; kj < n_vi; ++kj)
                    B(row_off + ki, col_off + kj) =
                    extractCoefficient(base, symbols_bank, virtual_input_bank,
                        list_virtual_input_providers[j], kj, true);
            }
            col_off += n_vi;
        }

        row_off += n_i;
    }

    // ---- C matrix ----
    int out_off = 0;
    for (int i = 0; i < (int)output.size(); ++i) {
        Bus* bus = output[i];
        if (bus->isGround()) continue;
        int bus_idx = bus_indices[bus];
        int n_pins = bus->getPinNumber();
        int col_off = 0;
        for (int j = 0; j < (int)list_state_variables.size(); ++j) {
            int n_j = getStateCount(list_state_variables[j]);
            for (int ki = 0; ki < n_pins; ++ki) {
                RCP<const Basic> base = matrix.get(bus_idx + ki, total_number_equations);
                for (int kj = 0; kj < n_j; ++kj) {
                    C(out_off + ki, col_off + kj) =
                        extractCoefficient(base, symbols_bank, virtual_input_bank, list_state_variables[j], kj, false);
                }
            }
            col_off += n_j;
        }
        out_off += n_pins;
    }

    // ---- D matrix ----
    out_off = 0;
    for (int i = 0; i < (int)output.size(); ++i) {
        Bus* bus = output[i];
        if (bus->isGround()) continue;
        int bus_idx = bus_indices[bus];
        int n_pins = bus->getPinNumber();
        int col_off = 0;

        for (int j = 0; j < (int)list_independent_sources.size(); ++j) {
            int n_j = list_independent_sources[j]->getInputPins();
            for (int ki = 0; ki < n_pins; ++ki) {
                RCP<const Basic> base = matrix.get(bus_idx + ki, total_number_equations);
                for (int kj = 0; kj < n_j; ++kj) {
                    D(out_off + ki, col_off + kj) =
                        extractCoefficient(base, symbols_bank, virtual_input_bank, list_independent_sources[j], kj, false);
                }
            }
            col_off += n_j;
        }

        // Virtual input columns
        for (int j = 0; j < (int)list_virtual_input_providers.size(); j++) {
            int n_vi = static_cast<int>(virtual_input_bank[list_virtual_input_providers[j]].size());
            for (int ki = 0; ki < n_pins; ++ki) {
                RCP<const Basic> base = matrix.get(bus_idx + ki, total_number_equations);
                for (int kj = 0; kj < n_vi; ++kj)
                    D(out_off + ki, col_off + kj) =
                    extractCoefficient(base, symbols_bank, virtual_input_bank,
                        list_virtual_input_providers[j], kj, true);
            }
            col_off += n_vi;
        }

        out_off += n_pins;
    }

    //std::cout << "[StateSpaceModel] A(" << A.rows() << "x" << A.cols()
    //    << ") B(" << B.rows() << "x" << B.cols()
    //    << ") C(" << C.rows() << "x" << C.cols()
    //    << ") D(" << D.rows() << "x" << D.cols() << ")" << std::endl;

    mode_ = mode;
    buildMappings();
    if (mode_ == SSMMode::DQsym) expandBForDQsym();
    //printMapping();
}

// ===================================================================
//  expandBForDQsym — post-process B so all columns are in groups of 3
// ===================================================================
//
//  For a DC source with 2 pins:
//    Raw B has 2 columns. Each column has identical entries across
//    all 3 phase rows (iΔ_a,b,c all see the same V_dc+).
//    Expansion: replicate each column to 3 identical columns.
//    Result: 6 columns (2 groups of 3).
//    After convertToPhasor: DC value appears in zero-sequence only.
//
//  For an AC source with 3 pins: already a group of 3, no change.
//  For virtual inputs (12 = 4 groups of 3): no change.
//

void StateSpaceModel::expandBForDQsym()
{
    // Build input groups
    input_groups.clear();
    int raw_col = 0;

    for (const auto& elem : list_independent_sources) {
        int n = elem->getInputPins();
        if (n > 3) {
            std::cerr << "[StateSpaceModel] Warning: Element " << elem->getElementSymbol()
                << " has " << n << " pins, which exceeds the expected maximum of 3 for DQsym. "
                << "Only the first 3 pins will be considered for DQsym expansion.\n";
            n = 3; // Limit to 3 for DQsym purposes
		}
		int expanded = (n % 3 == 0) ? n : 3 * n;   // create multiples of 3 (1→3, 2→6, 3→3)
        input_groups.push_back({ elem, n, expanded, raw_col, 0, false });
        raw_col += n;
    }
	// Virtual input providers are skipped. They are nonlinear and contribute via simulateTimeStep, not directly through B. If needed, they can be handled with a custom approach.
    for (const auto& elem : list_virtual_input_providers) {
        int n = static_cast<int>(virtual_input_bank[elem].size());
        int expanded = ((n + 2) / 3) * 3;
        input_groups.push_back({ elem, n, expanded, raw_col, 0, true });
        raw_col += n;
    }

    // Compute DQsym column offsets
    int dqsym_col = 0;
    for (auto& g : input_groups) {
        g.dqsymStartCol = dqsym_col;
        dqsym_col += g.dqsymCols;
    }

    int nu_dqsym = dqsym_col;
    int nx = B.rows();
    B_dqsym = Eigen::MatrixXd::Zero(nx, nu_dqsym);

    for (const auto& g : input_groups) {
        if (g.rawCols == g.dqsymCols) {
            // Already multiple of 3 — copy directly
            B_dqsym.block(0, g.dqsymStartCol, nx, g.rawCols) =
                B.block(0, g.rawStartCol, nx, g.rawCols);
        }
        else {
			// Need expansion: replicate as diagonal blocks of 3
            for (int p = 0; p < g.rawCols; ++p) {
                int group_base = g.dqsymStartCol + p * 3;
                for (int i = 0; i < nx; ++i) {
                    if (i % 3 == 0) {
                        B_dqsym(i, group_base) = B(i, g.rawStartCol + p);
					}
                    else if (i % 3 == 1) {
                        B_dqsym(i, group_base + 1) = B(i - 1, g.rawStartCol + p);
                    }
                    else if (i % 3 == 2) {
                        B_dqsym(i, group_base + 2) = B(i - 2, g.rawStartCol + p);
                    }
                }
            }
        }
    }

    // D_dqsym: same expansion pattern applied to D
    if (D.cols() > 0) {
        int ny = D.rows();
        D_dqsym = Eigen::MatrixXd::Zero(ny, nu_dqsym);
        for (const auto& g : input_groups) {
            if (g.rawCols == g.dqsymCols) {
                D_dqsym.block(0, g.dqsymStartCol, ny, g.rawCols) =
                    D.block(0, g.rawStartCol, ny, g.rawCols);
            }
            else {
                for (int p = 0; p < g.rawCols; ++p) {
                    int group_base = g.dqsymStartCol + p * 3;
                    for (int i = 0; i < nu_dqsym; ++i) {
                        if (i % 3 == 0) {
                            D_dqsym(i, group_base) = D(i, g.rawStartCol + p);
                        }
                        else if (i % 3 == 1) {
                            D_dqsym(i, group_base + 1) = D(i - 1, g.rawStartCol + p);
                        }
                        else if (i % 3 == 2) {
                            D_dqsym(i, group_base + 2) = D(i - 2, g.rawStartCol + p);
                        }
                    }
                }
            }

        }
    }

    std::cout << "[StateSpaceModel] DQsym B expanded: " << nx << "x" << nu_dqsym
        << " (raw was " << nx << "x" << B.cols() << ")\n";
}


// ===================================================================
//  buildMappings — accounts for DQsym expansion
// ===================================================================

void StateSpaceModel::buildMappings()
{
    input_map.clear(); state_map.clear(); output_map.clear();

    if (mode_ == SSMMode::DQsym && !input_groups.empty()) {
        // DQsym mode: use expanded column indices
        for (const auto& g : input_groups) {
            for (int p = 0; p < g.rawCols; ++p) {
                int group_base = g.dqsymStartCol + p * 3;
                for (int ph = 0; ph < 3; ++ph) {
                    input_map.push_back({
                        g.element->getElementSymbol(),
                        p * 3 + ph,          // linearized pin index in DQsym space
                        group_base + ph,     // B_dqsym column
                        g.isVirtual,
                        (group_base + ph) / 3,   // group index
                        ph                        // phase in group
                        });
                }
            }
        }
    }
    else {
        // Standard mode: raw column indices
        int col = 0;
        for (const auto& elem : list_independent_sources) {
            for (int p = 0; p < elem->getInputPins(); ++p)
                input_map.push_back({ elem->getElementSymbol(), p, col++, false, 0, 0 });
        }
        for (const auto& elem : list_virtual_input_providers) {
            int n = static_cast<int>(virtual_input_bank[elem].size());
            for (int s = 0; s < n; ++s)
                input_map.push_back({ elem->getElementSymbol(), s, col++, true, 0, 0 });
        }
    }

    // State map (same for both modes)
    int row = 0;
    for (const auto& elem : list_state_variables) {
        int n = getStateCount(elem);
        for (int s = 0; s < n; ++s)
            state_map.push_back({ elem->getElementSymbol(), s, row++ });
    }

    // Output map
    int orow = 0;
    for (const auto& bus : output) {
        if (bus->isGround()) continue;
        for (int p = 0; p < bus->getPinNumber(); ++p)
            output_map.push_back({ bus->getBusName(), p, orow++ });
    }
}


// ===================================================================
//  buildInputVector — DQsym mode: 3 rows per group
// ===================================================================

MatrixXcd StateSpaceModel::buildInputVector(
    int nKeep,
    const std::map<std::string, std::vector<MatrixXcd>>& elementStates) const
{
    int nu = B_dqsym.cols();
    MatrixXcd u = MatrixXcd::Zero(nu, nKeep);

    for (const auto& g : input_groups) {
        std::string name = g.element->getElementSymbol();
        if (!g.isVirtual) {
            for (const auto& g : input_groups) {
                std::string name = g.element->getElementSymbol();

                // Source element — call simulateInputStep
                auto vals = g.element->simulateInputStep({}, nKeep);
                if (vals.empty()) continue;
                MatrixXcd V = vals[0];  // Assuming simulateInputStep returns a matrix with rows corresponding to pins

                if (mode_ == SSMMode::DQsym) {
                    // Expand: each raw pin → 3 identical rows (zero-sequence broadcast)
                    for (int p = 0; p < g.rawCols && p < V.rows(); ++p) {
                        int base = g.dqsymStartCol + p * 3;
                        for (int ph = 0; ph < 3; ++ph)
                            if (base + ph < nu)
                                u.row(base + ph) = V.row(3 * p + ph);
                    }
                }
                else {
                    // Standard: direct placement
                    for (int p = 0; p < g.rawCols && p < V.rows(); ++p)
                        u.row(g.rawStartCol + p) = V.row(p);
                }
            }
        }
        else {
            // Virtual input — call simulateInputStep with states
            std::vector<MatrixXcd> states;
            auto it = elementStates.find(name);
            if (it != elementStates.end())
                states = it->second;

            auto feedback = g.element->simulateInputStep(states, nKeep);
            // feedback = [u_vMΔ(3×nKeep), u_vMΣ(3×nKeep), u_PΔ(3×nKeep), u_PΣ(3×nKeep)]
            // Already in groups of 3 — place directly

            int col = (mode_ == SSMMode::DQsym) ? g.dqsymStartCol : g.rawStartCol;
            for (const auto& fb : feedback) {
                for (int ph = 0; ph < fb.rows() && ph < 3; ++ph) {
                    if (col < nu)
                        u.row(col) = fb.row(ph);
                    ++col;
                }
            }
        }
    }

    return u;
}


// ===================================================================
//  printMapping
// ===================================================================

void StateSpaceModel::printMapping() const
{
    std::cout << "\n[SSM] Mode: " << (mode_ == SSMMode::DQsym ? "DQsym" : "Standard") << "\n";

    std::cout << "[SSM] Input groups:\n";
    for (const auto& g : input_groups)
        std::cout << "  " << g.element->getElementSymbol()
        << ": raw=" << g.rawCols << " cols"
        << (mode_ == SSMMode::DQsym ?
            " → dqsym=" + std::to_string(g.dqsymCols) + " cols (start=" +
            std::to_string(g.dqsymStartCol) + ")" :
            " (start=" + std::to_string(g.rawStartCol) + ")")
        << (g.isVirtual ? " [virtual]" : " [source]") << "\n";

    std::cout << "[SSM] State mapping:\n";
    for (const auto& m : state_map)
        std::cout << "  A[" << m.aRowIndex << "] = " << m.elementName
        << " state " << m.stateIndex << "\n";

    std::cout << std::endl;
}



// LOOPS AND CUTSETS
std::vector<std::vector<Bus*>> StateSpaceModel::form_cutset_nodes(
    const std::vector<Bus*>& buses,
    const std::vector<std::pair<Bus*, Bus*>>& connections)
{
    std::vector<std::vector<Bus*>> result;

    for (size_t i = 0; i < buses.size(); ++i) {
        // Start with the current node
        std::vector<Bus*> single_node = { buses[i] };
        result.push_back(single_node);

        std::vector<std::vector<Bus*>> temp;

        // Build new combinations by adding the next node to existing ones
        for (const auto& sublist : result) {
            if (i < buses.size() - 1) {
                std::vector<Bus*> newlist = sublist;
                newlist.push_back(buses[i + 1]);

                // Sort by bus index
                std::sort(newlist.begin(), newlist.end(), [](Bus* a, Bus* b) {
                    return a->getBusName() < b->getBusName();
                    });

                temp.push_back(newlist);
            }
        }

        // Add the new combinations to the result
        for (const auto& sublist : temp) {
            result.push_back(sublist);
        }

        // Debug print 
        std::cout << "All cutset node combinations:" << std::endl;
        for (const auto& cutset : result) {
            std::cout << "[";
            for (size_t j = 0; j < cutset.size(); ++j) {
                std::cout << cutset[j]->getBusName();
                if (j != cutset.size() - 1) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
        }
    }

    return result;
}

std::vector<std::vector<Element*>> StateSpaceModel::form_cutsets(
    const std::vector<std::vector<Bus*>>& cutset_nodes,
    const std::map<Bus*, std::vector<Element*>>& busToElementsMap)
{
    std::vector<std::vector<Element*>> cutset_collection;

    auto bus_in_cutset = [](Bus* bus, const std::vector<Bus*>& cutset) -> bool {
        return std::find(cutset.begin(), cutset.end(), bus) != cutset.end();
        };

    for (const auto& cutset : cutset_nodes) {
        std::set<Element*> all_elements;

        for (Bus* bus : cutset) {
            auto it = busToElementsMap.find(bus);
            //if (it == busToElementsMap.end()) continue;
            if (it != busToElementsMap.end()) {
                all_elements.insert(it->second.begin(), it->second.end());
            }
        }
        std::set<Element*> unique_elements;
        // Filter elements that connect cutset to outside
        for (Element* element : all_elements) {
            std::vector<Bus*> connected_buses = element->getBuses();

            bool in = false;
            bool out = false;
            for (Bus* b : connected_buses) {
                if (bus_in_cutset(b, cutset)) in = true;
                else out = true;
            }

            if (in && out) {
                unique_elements.insert(element);
            }
        }

        cutset_collection.emplace_back(unique_elements.begin(), unique_elements.end());
    }
    return cutset_collection;
}

std::map<Bus*, std::vector<Element*>> StateSpaceModel::generateBusToElementsMap(
    const std::vector<Element*>& elements) {

    std::map<Bus*, std::vector<Element*>> map;
    for (Element* e : elements) {
        for (Bus* b : e->getBuses()) {
            map[b].push_back(e);
        }
    }
    return map;
}
// Finds all loops in the given set of buses and their connected elements
std::vector<std::vector<Element*>> StateSpaceModel::findLoops(
    const std::vector<Bus*>& nodes,
    const std::map<Bus*, std::vector<Element*>>& node_collection)
{
    std::vector<std::vector<Element*>> loop_collection;

    // Create bus index mapping
    std::map<Bus*, int> busIndices;
    for (size_t i = 0; i < nodes.size(); ++i) {
        busIndices[nodes[i]] = static_cast<int>(i);
    }

    for (Bus* node : nodes) {
        traverseForLoops(node, node, nodes, loop_collection, node_collection, nullptr, busIndices);
    }

    return loop_collection;
}

//builds the path (vector of elements) from the root to this node
std::vector<Element*> StateSpaceModel::Tree::getPath() const {
    std::vector<Element*> path;
    if (parent) path = parent->getPath();
    if (element) path.push_back(element);
    return path;
}

// Checks if the given Bus* is already in the current branch
bool StateSpaceModel::Tree::containsNode(Bus* node) const {
    if (current_node == node) return true;
    if (parent) return parent->containsNode(node);
    return false;
}

std::string StateSpaceModel::Tree::toString() const {
    std::string str = "Root: " + (root ? root->getBusName() : "null") +
        ", Current: " + (current_node ? current_node->getBusName() : "null") +
        ", Element: " + (element ? element->getElementSymbol() : "None") +
        ", Subelements: [";

    for (size_t i = 0; i < subelements.size(); ++i) {
        str += subelements[i]->getElementSymbol();
        if (i != subelements.size() - 1) str += ", ";
    }
    str += "]";
    return str;
}

//Compares two loops (vectors of Element*) for strict order-based equality
bool equalLoops(const std::vector<Element*>& a, const std::vector<Element*>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

void StateSpaceModel::traverseForLoops(
    Bus* current_node,
    Bus* start_node,
    const std::vector<Bus*>& nodes,
    std::vector<std::vector<Element*>>& loop_collection,
    const std::map<Bus*, std::vector<Element*>>& node_collection,
    std::shared_ptr<Tree> current_branch,
    const std::map<Bus*, int>& busIndices)
{
    auto it = node_collection.find(current_node);
    if (it == node_collection.end()) return;  // No elements connected to this bus

    const std::vector<Element*>& connected_elements = it->second;

    for (Element* element : connected_elements) {
        Bus* other_node = element->getOtherBus(current_node); // The bus on the other side of the element
        if (!other_node) continue;

        auto new_branch = std::make_shared<Tree>(element, current_node, start_node, current_branch);

        if (busIndices.at(other_node) > busIndices.at(current_node)) {
            if (current_branch) current_branch->addPathElement(element);
            traverseForLoops(other_node, start_node, nodes, loop_collection, node_collection, new_branch, busIndices);
        }
        else if (other_node == start_node) {
            if (!current_branch || !current_branch->getElement() || element != current_branch->getElement()) {

                std::shared_ptr<Tree> iter = new_branch;
                std::vector<Element*> loop;
                std::vector<Bus*> loop_nodes;
                loop_nodes.push_back(start_node);

                while (iter) {
                    Element* e = iter->getElement();
                    if (e) loop.push_back(e);
                    Bus* n = iter->getCurrentNode();
                    if (n) loop_nodes.push_back(n);
                    iter = iter->getParent();
                }

                std::reverse(loop.begin(), loop.end());
                std::reverse(loop_nodes.begin(), loop_nodes.end());

                // compare by ordered vector equality instead of set
                bool duplicate = false;
                for (const auto& existing_loop : loop_collection) {
                    if (equalLoops(existing_loop, loop)) {
                        duplicate = true;
                        break;
                    }
                }

                if (!duplicate) {
                    loop_collection.push_back(loop);
                }
                continue;
            }
        }

    }
}
