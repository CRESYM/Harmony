//State_Space_Model.cpp

#include "State_Space_Model.h"
using SymEngine::RCP;
using SymEngine::Basic;
using SymEngine::DenseMatrix;
using SymEngine::integer;
using SymEngine::mul;
using SymEngine::symbol;
using SymEngine::one;
#include <symengine/expression.h> 
#include <symengine/simplify.h>  


using namespace SymEngine;


std::vector<std::vector<Bus*>> StateSpaceModel::from_cutset_nodes(
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

std::vector<std::vector<Element*>> StateSpaceModel::from_cutsets(
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

void fillWithZero(DenseMatrix& mat) {
    for (unsigned i = 0; i < mat.nrows(); ++i)
        for (unsigned j = 0; j < mat.ncols(); ++j)
            mat.set(i, j, zero);
}

//void StateSpaceModel::formState(Network* net)
//{
//    // Retrieve system information
//    int state_variables = net->getNumberStateVariables();
//    int state_variables_position = net->getStateVariablePosition();
//    int independent_sources = net->getNumberIndependentSource();
//    int number_outputs = net->getNumberOutputs();
//    int number_equations = net->getNumberEquations();
//    const std::vector<RCP<const Symbol>>& state_variables_symbols = net->getStateVariableSymbols();
//    const std::vector<RCP<const Symbol>>& independent_sources_symbols = net->getSourceSymbols();
//    const std::vector<int>& output_indexes = net->getOutputIndexes();
//
//    // Store symbols
//    x_symbols = state_variables_symbols;
//    u_symbols = independent_sources_symbols;
//    y_symbols.clear(); // optional if you want symbolic outputs as well
//    
//    // Initialize A, B, C, D matrices
//    A = DenseMatrix(state_variables, state_variables); fillWithZero(A);
//    B = DenseMatrix(state_variables, independent_sources); fillWithZero(B);
//    C = DenseMatrix(number_outputs, state_variables); fillWithZero(C);
//    D = DenseMatrix(number_outputs, independent_sources); fillWithZero(D);
//    
//    DenseMatrix MNA_matrix(number_equations, number_equations + 1);
//    fillWithZero(MNA_matrix);
//    
//    auto busIndex = net->getBusIndexMap();
//    auto elements = net->getElements();
//    
//    try {
//        for (const auto& el : elements) {
//            el.second->writeMatrixSymbolic(MNA_matrix, busIndex);
//        }
//    }
//    catch (const std::exception& e) {
//        std::cerr << "[StateSpaceModel::formState] Exception caught during MNA matrix stamping: " << e.what() << "\n";
//    }
//    std::cout << "MNA matrix before RREF:\n";
//    
//    for (int r = 0; r < number_equations; ++r) {
//    for (int c = 0; c < number_equations + 1; ++c) {
//        std::cout << MNA_matrix.get(r, c)->__str__() << " ";
//    }
//    std::cout << "\n";
//}
//
//vec_uint pivot_cols;
//DenseMatrix rref_matrix_raw(MNA_matrix.nrows(), MNA_matrix.ncols()); // Use a temporary for raw RREF
//fillWithZero(rref_matrix_raw);
//
//try {
//    reduced_row_echelon_form(MNA_matrix, rref_matrix_raw, pivot_cols);
//    std::cout << "RREF complete, number of pivots: " << pivot_cols.size() << "\n";
//}
//catch (const std::exception& e) {
//    std::cerr << "[StateSpaceModel::formState] Exception during RREF: " << e.what() << "\n";
//}
//catch (...) {
//    std::cerr << "[StateSpaceModel::formState] Unknown exception during RREF\n";
//}
//
//std::cout << "RREF complete, number of pivots: " << pivot_cols.size() << "\n";
//

//if (rref_matrix_raw.nrows() > 0 && rref_matrix_raw.ncols() > 0) {

//    RCP<const Basic> complex_element = rref_matrix_raw.get(0, 0); // Or pick another index
//    std::cout << "\nOriginal complex element at (0,0): " << complex_element->__str__() << "\n";
//
//    RCP<const Basic> simplified_element = SymEngine::simplify(complex_element);
//    std::cout << "Simplified element: " << simplified_element->__str__() << "\n";
//
//    RCP<const Basic> expanded_element = expand(complex_element);
//    std::cout << "Expanded element: " << expanded_element->__str__() << "\n";
//}


//DenseMatrix rref_matrix_simplified(rref_matrix_raw.nrows(), rref_matrix_raw.ncols());
//for (unsigned r = 0; r < rref_matrix_raw.nrows(); ++r) {
//    for (unsigned c = 0; c < rref_matrix_raw.ncols(); ++c) {
//        // CHANGE HERE: Use SymEngine::expand instead of SymEngine::simplify
//        rref_matrix_simplified.set(r, c, SymEngine::expand(rref_matrix_raw.get(r, c)));
//    }
//}
//
//// Print SIMPLIFIED 
//std::cout << "Expanded RREF matrix:\n"; // Changed label for clarity
//for (int r = 0; r < rref_matrix_simplified.nrows(); ++r) {
//    for (int c = 0; c < rref_matrix_simplified.ncols(); ++c) {
//        std::cout << rref_matrix_simplified.get(r, c)->__str__() << " ";
//    }
//    std::cout << "\n";
//}
//
//std::cout << "state_variables_position: " << state_variables_position << "\n";
//std::cout << "state_variables: " << state_variables << "\n";
//std::cout << "independent_sources: " << independent_sources << "\n";
//std::cout << "number_outputs: " << number_outputs << "\n";
//
//std::cout << "output_indexes: ";
//for (int idx : output_indexes) std::cout << idx << " ";
//std::cout << "\n";
//
//// Loop through pivot rows to extract A, B, C, D
//for (size_t rref_row_idx = 0; rref_row_idx < pivot_cols.size(); ++rref_row_idx) {
//    int original_col_pivot = pivot_cols[rref_row_idx];
// 
//    RCP<const Basic> rhs_expression_rcp = rref_matrix_simplified.get(rref_row_idx, number_equations);
//    Expression rhs_expression = Expression(rhs_expression_rcp);
//
//
//    //  A matrix row population:
//    if (original_col_pivot >= state_variables_position &&
//        original_col_pivot < (state_variables_position + state_variables))
//    {
//        int state_output_row_idx = original_col_pivot - state_variables_position;
//        std::cout << "  Processing state variable (dx/dt) for row index " << state_output_row_idx << "\n";
//        // Print the RHS expression 
//        std::cout << "  RHS Expression for dx/dt row: " << rhs_expression_rcp->__str__() << "\n";
//

//        // Populate A matrix row
//        for (int j = 0; j < state_variables; ++j) {
//            std::cout << "    Populating A(" << state_output_row_idx << "," << j << "). Target symbol: " << state_variables_symbols[j]->__str__() << "\n";
//
//            map_basic_basic subs_map;
//            subs_map[state_variables_symbols[j]] = one; // Target variable = 1
//            for (int k = 0; k < state_variables; ++k) {
//                if (k != j) {
//                    subs_map[state_variables_symbols[k]] = zero; // Other state variables = 0
//                }
//            }
//            for (int k = 0; k < independent_sources; ++k) {
//                subs_map[independent_sources_symbols[k]] = zero; // All sources = 0
//            }
//
//            // Print the substitution map
//            std::cout << "      Substitution map: {";
//            for (auto const& [key, val] : subs_map) {
//                std::cout << key->__str__() << ": " << val->__str__() << ", ";
//            }
//            std::cout << "}\n";
//
//    
//            try {
//                RCP<const Basic> substituted_val_rcp = rhs_expression.subs(subs_map);
//                std::cout << "      Substituted value (before set): " << substituted_val_rcp->__str__() << "\n";
//                A.set(state_output_row_idx, j, substituted_val_rcp);
//                std::cout << "      Set A(" << state_output_row_idx << "," << j << ") successfully.\n";
//            }
//            catch (const std::exception& e) {
//                std::cerr << "!!! CRASH DETECTED during A matrix substitution for A("
//                    << state_output_row_idx << "," << j << "): " << e.what() << "\n";
//                // Re-throw or exit gracefully if this happens, so you know exactly where it broke
//                throw; // This will stop execution and give you the stack trace if you're debugging
//            }
//            catch (...) {
//                std::cerr << "!!! CRASH DETECTED during A matrix substitution for A("
//                    << state_output_row_idx << "," << j << "): Unknown exception.\n";
//                throw;
//            }
//        }
//    }
//
//    // Output Equations (y = Cx + Du) 
//    auto it_output = std::find(output_indexes.begin(), output_indexes.end(), original_col_pivot);
//    if (it_output != output_indexes.end()) {
//        int output_row_idx = std::distance(output_indexes.begin(), it_output);
//        std::cout << "  Processing output equation for row index " << output_row_idx << "\n";
//
//        // Populate C matrix row
//        for (int j = 0; j < state_variables; ++j) {
//            map_basic_basic subs_map;
//            subs_map[x_symbols[j]] = one;
//            for (int k = 0; k < state_variables; ++k) {
//                if (k != j) {
//                    subs_map[x_symbols[k]] = zero;
//                }
//            }
//            for (int k = 0; k < independent_sources; ++k) {
//                subs_map[u_symbols[k]] = zero;
//            }
//            C.set(output_row_idx, j, rhs_expression.subs(subs_map));
//        }
//
//        // Populate D matrix row
//        for (int j = 0; j < independent_sources; ++j) {
//            map_basic_basic subs_map;
//            subs_map[u_symbols[j]] = one;
//            for (int k = 0; k < independent_sources; ++k) {
//                if (k != j) {
//                    subs_map[u_symbols[k]] = zero;
//                }
//            }
//            for (int k = 0; k < state_variables; ++k) {
//                subs_map[x_symbols[k]] = zero;
//            }
//            D.set(output_row_idx, j, rhs_expression.subs(subs_map));
//        }
//    }
//}
//
//std::cout << "Matrix A = \n" << A << "\n";
//std::cout << "Matrix B = \n" << B << "\n";
//std::cout << "Matrix C = \n" << C << "\n";
//std::cout << "Matrix D = \n" << D << "\n";
//}

void StateSpaceModel::formState(Network* net)
{
    int state_vars = net->getNumberStateVariables();
    int state_pos = net->getStateVariablePosition();
    int input_vars = net->getNumberIndependentSource();
    int outputs = net->getNumberOutputs();
    int eqs = net->getNumberEquations();

    std::cout << "state_vars = " << state_vars << "\n";
    std::cout << "input_vars = " << input_vars << "\n";
    std::cout << "outputs = " << outputs << "\n";
    std::cout << "eqs = " << eqs << "\n";

    if (eqs <= 0) {
        std::cerr << "[ERROR] Number of equations is zero or negative! Cannot proceed.\n";
        return;
    }

    auto state_syms = net->getStateVariableSymbols();
    auto input_syms = net->getSourceSymbols();
    auto output_idx = net->getOutputIndexes();

    // Initialize symbolic matrices A,B,C,D with zeros
    A = DenseMatrix(state_vars, state_vars);
    B = DenseMatrix(state_vars, input_vars);
    C = DenseMatrix(outputs, state_vars);
    D = DenseMatrix(outputs, input_vars);

    for (int r = 0; r < state_vars; ++r)
        for (int c = 0; c < state_vars; ++c)
            A.set(r, c, Expression(0).get_basic());
    for (int r = 0; r < state_vars; ++r)
        for (int c = 0; c < input_vars; ++c)
            B.set(r, c, Expression(0).get_basic());
    for (int r = 0; r < outputs; ++r)
        for (int c = 0; c < state_vars; ++c)
            C.set(r, c, Expression(0).get_basic());
    for (int r = 0; r < outputs; ++r)
        for (int c = 0; c < input_vars; ++c)
            D.set(r, c, Expression(0).get_basic());

    // Build numeric MNA matrix and stamp all elements
    Eigen::MatrixXd MNA = Eigen::MatrixXd::Zero(eqs, eqs + 1);
    auto busIndex = net->getBusIndexMap();

    std::cout << "Stamping elements into MNA matrix...\n";
    int el_idx = 0;
    for (const auto& [name, el] : net->getElements())
    {
        std::cout << "Stamping element " << name << " with index " << el_idx << "\n";
        el->writeMNAmatrixNumeric(MNA, eqs, el_idx, busIndex);
        el_idx++;
    }

    std::cout << "MNA matrix after stamping (partial view):\n";
    for (int r = 0; r < std::min(5, eqs); ++r) {
        for (int c = 0; c < std::min(5, eqs + 1); ++c) {
            std::cout << MNA(r, c) << " ";
        }
        std::cout << "\n";
    }

    // Convert to symbolic matrix for RREF
    DenseMatrix MNA_sym(eqs, eqs + 1);
    for (int r = 0; r < eqs; ++r)
        for (int c = 0; c < eqs + 1; ++c)
            MNA_sym.set(r, c, Expression(MNA(r, c)).get_basic());

    std::cout << "Converted MNA matrix to symbolic form successfully.\n";

    // Compute RREF and get pivot columns
    vec_uint pivots;
    DenseMatrix RREF(eqs, eqs + 1);
    reduced_row_echelon_form(MNA_sym, RREF, pivots);

    // Expand expressions for better substitution
    DenseMatrix RREF_expanded(eqs, eqs + 1);
    for (int r = 0; r < eqs; ++r)
        for (int c = 0; c < eqs + 1; ++c)
            RREF_expanded.set(r, c, SymEngine::expand(RREF.get(r, c)));

    // Populate A, B, C, D 
    for (size_t i = 0; i < pivots.size(); ++i)
    {
        int pivot_col = pivots[i];
        Expression rhs_expr(RREF_expanded.get(i, eqs)); 

        if (pivot_col >= state_pos && pivot_col < state_pos + state_vars)
        {
            int idx = pivot_col - state_pos;

            // Fill row idx of A
            for (int j = 0; j < state_vars; ++j)
            {
                Expression val = rhs_expr;
                for (int k = 0; k < state_vars; ++k)
                {
                    val = val.subs({ { Expression(state_syms[k]).get_basic(), (k == j) ? Expression(1).get_basic() : Expression(0).get_basic() } });
                }
                for (int k = 0; k < input_vars; ++k)
                {
                    val = val.subs({ { Expression(input_syms[k]).get_basic(), Expression(0).get_basic() } });
                }
                A.set(idx, j, val.get_basic());
            }

            // Fill row idx of B
            for (int j = 0; j < input_vars; ++j)
            {
                Expression val = rhs_expr;
                for (int k = 0; k < input_vars; ++k)
                {
                    val = val.subs({ { Expression(input_syms[k]).get_basic(), (k == j) ? Expression(1).get_basic() : Expression(0).get_basic() } });
                }
                for (int k = 0; k < state_vars; ++k)
                {
                    val = val.subs({ { Expression(state_syms[k]).get_basic(), Expression(0).get_basic() } });
                }
                B.set(idx, j, val.get_basic());
            }
        }

        // Similar fixes for C and D below...
        auto it = std::find(output_idx.begin(), output_idx.end(), pivot_col);
        if (it != output_idx.end())
        {
            int out_idx = it - output_idx.begin();

            // Fill row out_idx of C
            for (int j = 0; j < state_vars; ++j)
            {
                Expression val = rhs_expr;
                for (int k = 0; k < state_vars; ++k)
                {
                    val = val.subs({ { Expression(state_syms[k]).get_basic(), (k == j) ? Expression(1).get_basic() : Expression(0).get_basic() } });
                }
                for (int k = 0; k < input_vars; ++k)
                {
                    val = val.subs({ { Expression(input_syms[k]).get_basic(), Expression(0).get_basic() } });
                }
                C.set(out_idx, j, val.get_basic());
            }

            // Fill row out_idx of D
            for (int j = 0; j < input_vars; ++j)
            {
                Expression val = rhs_expr;
                for (int k = 0; k < input_vars; ++k)
                {
                    val = val.subs({ { Expression(input_syms[k]).get_basic(), (k == j) ? Expression(1).get_basic() : Expression(0).get_basic() } });
                }
                for (int k = 0; k < state_vars; ++k)
                {
                    val = val.subs({ { Expression(state_syms[k]).get_basic(), Expression(0).get_basic() } });
                }
                D.set(out_idx, j, val.get_basic());
            }
        }
    }
    // print A, B, C, D for debugging
    std::cout << "A = \n" << A << "\n";
    std::cout << "B = \n" << B << "\n";
    std::cout << "C = \n" << C << "\n";
    std::cout << "D = \n" << D << "\n";
}

