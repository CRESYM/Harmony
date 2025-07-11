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
//    int state_vars = net->getNumberStateVariables();
//    //int state_pos = net->getStateVariablePosition();
//    int input_vars = net->getNumberIndependentSource();
//    int outputs = net->getNumberOutputs();
//    int eqs = net->getNumberEquations();
//
//    std::cout << "state_vars = " << state_vars << "\n";
//    std::cout << "input_vars = " << input_vars << "\n";
//    std::cout << "outputs = " << outputs << "\n";
//    std::cout << "eqs = " << eqs << "\n";
//
//    if (eqs <= 0) {
//        std::cerr << "[ERROR] Number of equations is zero or negative! Cannot proceed.\n";
//        return;
//    }
//
//    //auto state_syms = net->getStateVariableSymbols();
//    //auto input_syms = net->getSourceSymbols();
//    //auto output_idx = net->getOutputIndexes();
//
//    //// Initialize symbolic matrices A,B,C,D with zeros
//    //A = DenseMatrix(state_vars, state_vars);
//    //B = DenseMatrix(state_vars, input_vars);
//    //C = DenseMatrix(outputs, state_vars);
//    //D = DenseMatrix(outputs, input_vars);
//
//    //for (int r = 0; r < state_vars; ++r)
//    //    for (int c = 0; c < state_vars; ++c)
//    //        A.set(r, c, Expression(0).get_basic());
//    //for (int r = 0; r < state_vars; ++r)
//    //    for (int c = 0; c < input_vars; ++c)
//    //        B.set(r, c, Expression(0).get_basic());
//    //for (int r = 0; r < outputs; ++r)
//    //    for (int c = 0; c < state_vars; ++c)
//    //        C.set(r, c, Expression(0).get_basic());
//    //for (int r = 0; r < outputs; ++r)
//    //    for (int c = 0; c < input_vars; ++c)
//    //        D.set(r, c, Expression(0).get_basic());
//
//    // Build numeric MNA matrix and stamp all elements
//    //Eigen::MatrixXd MNA = Eigen::MatrixXd::Zero(eqs, eqs + 1);
//    Eigen::MatrixXd A_mat = Eigen::MatrixXd::Zero(eqs, eqs);
//    Eigen::MatrixXd E_mat = Eigen::MatrixXd::Zero(eqs, eqs);
//
//    auto busIndex = net->getBusIndexMap();
//    auto currentSourceIndex = net->getCurrentSourceIndexMap();
//    auto stateVarIndex = net->getStateVarIndexMap();
//
//    std::cout << "Stamping elements into MNA matrix...\n";
//    int el_idx = 0;
//    for (const auto& [name, el] : net->getElements())
//    {
//        std::cout << "Stamping element " << name << " with index " << el_idx << "\n";
//        //el->writeMNAmatrixNumeric(MNA, eqs, el_idx, busIndex);
//        el->writeMNAmatrixNumeric(A_mat, E_mat, eqs, el_idx, busIndex, currentSourceIndex, stateVarIndex);
//
//        el_idx++;
//    }
//
//    Eigen::MatrixXd B_mat = Eigen::MatrixXd::Zero(eqs, input_vars);
//
//    //std::cout << "MNA matrix after stamping (partial view):\n";
//    //for (int r = 0; r < std::min(5, eqs); ++r) {
//    //    for (int c = 0; c < std::min(5, eqs + 1); ++c) {
//    //        std::cout << MNA(r, c) << " ";
//    //    }
//    //    std::cout << "\n";
//    //}
//
//    //// Convert to symbolic matrix for RREF
//    //DenseMatrix MNA_sym(eqs, eqs + 1);
//    //for (int r = 0; r < eqs; ++r)
//    //    for (int c = 0; c < eqs + 1; ++c)
//    //        MNA_sym.set(r, c, Expression(MNA(r, c)).get_basic());
//
//    // Construct symbolic MNA = s*E + A
//   // DenseMatrix MNA_sym(eqs, eqs + 1);
//   // RCP<const Symbol> s = symbol("s");
//
//   // for (int r = 0; r < eqs; ++r) {
//   //     for (int c = 0; c < eqs; ++c) {
//   //         Expression e_entry = E_mat(r, c);
//   //         Expression a_entry = A_mat(r, c);
//   //         Expression expr = e_entry * s + a_entry;
//   //         MNA_sym.set(r, c, expr.get_basic());
//   //     }
//   //     // No input source modeling in RHS yet, so fill with 0
//   //     MNA_sym.set(r, eqs, Expression(0).get_basic());
//   // }
//
//   // std::cout << "Converted A and E matrices to symbolic MNA (sE + A).\n";
//
//
//   //// std::cout << "Converted MNA matrix to symbolic form successfully.\n";
//
//   // // Compute RREF and get pivot columns
//   // vec_uint pivots;
//   // DenseMatrix RREF(eqs, eqs + 1);
//   // reduced_row_echelon_form(MNA_sym, RREF, pivots);
//
//   // // Expand expressions for better substitution
//   // DenseMatrix RREF_expanded(eqs, eqs + 1);
//   // for (int r = 0; r < eqs; ++r)
//   //     for (int c = 0; c < eqs + 1; ++c) {
//   //         RREF_expanded.set(r, c, SymEngine::expand(RREF.get(r, c)));
//   //         std::cout << "RREF done!.\n";
//   //     }
//   // std::cout << "RREF computed and expanded.\n";
//   // 
//    Eigen::MatrixXd E_ss = E_mat.block(0, 0, state_vars, state_vars);
//    Eigen::MatrixXd A_ss = A_mat.block(0, 0, state_vars, state_vars);
//    Eigen::MatrixXd B_ss = B_mat.block(0, 0, state_vars, input_vars);
//
//    // Check E_ss invertibility
//    if (E_ss.determinant() == 0) {
//        std::cerr << "E matrix is singular; cannot invert.\n";
//        return;
//    }
//    // Populate A, B, C, D 
//    //for (size_t i = 0; i < pivots.size(); ++i)
//    //{
//    //    int pivot_col = pivots[i];
//    //    Expression rhs_expr(RREF_expanded.get(i, eqs)); 
//
//    //    if (pivot_col >= state_pos && pivot_col < state_pos + state_vars)
//    //    {
//    //        int idx = pivot_col - state_pos;
//
//    //        // Fill row idx of A
//    //        for (int j = 0; j < state_vars; ++j)
//    //        {
//    //            Expression val = rhs_expr;
//    //            for (int k = 0; k < state_vars; ++k)
//    //            {
//    //                val = val.subs({ { Expression(state_syms[k]).get_basic(), (k == j) ? Expression(1).get_basic() : Expression(0).get_basic() } });
//    //            }
//    //            for (int k = 0; k < input_vars; ++k)
//    //            {
//    //                val = val.subs({ { Expression(input_syms[k]).get_basic(), Expression(0).get_basic() } });
//    //            }
//    //            A.set(idx, j, val.get_basic());
//    //        }
//
//    //        // Fill row idx of B
//    //        for (int j = 0; j < input_vars; ++j)
//    //        {
//    //            Expression val = rhs_expr;
//    //            for (int k = 0; k < input_vars; ++k)
//    //            {
//    //                val = val.subs({ { Expression(input_syms[k]).get_basic(), (k == j) ? Expression(1).get_basic() : Expression(0).get_basic() } });
//    //            }
//    //            for (int k = 0; k < state_vars; ++k)
//    //            {
//    //                val = val.subs({ { Expression(state_syms[k]).get_basic(), Expression(0).get_basic() } });
//    //            }
//    //            B.set(idx, j, val.get_basic());
//    //        }
//    //    }
//
//    //    // Similar fixes for C and D below...
//    //    auto it = std::find(output_idx.begin(), output_idx.end(), pivot_col);
//    //    if (it != output_idx.end())
//    //    {
//    //        int out_idx = it - output_idx.begin();
//
//    //        // Fill row out_idx of C
//    //        for (int j = 0; j < state_vars; ++j)
//    //        {
//    //            Expression val = rhs_expr;
//    //            for (int k = 0; k < state_vars; ++k)
//    //            {
//    //                val = val.subs({ { Expression(state_syms[k]).get_basic(), (k == j) ? Expression(1).get_basic() : Expression(0).get_basic() } });
//    //            }
//    //            for (int k = 0; k < input_vars; ++k)
//    //            {
//    //                val = val.subs({ { Expression(input_syms[k]).get_basic(), Expression(0).get_basic() } });
//    //            }
//    //            C.set(out_idx, j, val.get_basic());
//    //        }
//
//    //        // Fill row out_idx of D
//    //        for (int j = 0; j < input_vars; ++j)
//    //        {
//    //            Expression val = rhs_expr;
//    //            for (int k = 0; k < input_vars; ++k)
//    //            {
//    //                val = val.subs({ { Expression(input_syms[k]).get_basic(), (k == j) ? Expression(1).get_basic() : Expression(0).get_basic() } });
//    //            }
//    //            for (int k = 0; k < state_vars; ++k)
//    //            {
//    //                val = val.subs({ { Expression(state_syms[k]).get_basic(), Expression(0).get_basic() } });
//    //            }
//    //            D.set(out_idx, j, val.get_basic());
//    //        }
//    //    }
//    //}
//    
//    // Compute state-space A, B matrices
//    Eigen::MatrixXd A_state = E_ss.inverse() * A_ss;
//    Eigen::MatrixXd B_state = E_ss.inverse() * B_ss;
//
//    // For outputs, define C and D 
//    Eigen::MatrixXd C_state = Eigen::MatrixXd::Identity(outputs, state_vars); 
//    Eigen::MatrixXd D_state = Eigen::MatrixXd::Zero(outputs, input_vars);
//
//    std::cout << "Numerical state-space matrices (real values):\n";
//    std::cout << "A =\n" << A_state << "\n";
//    std::cout << "B =\n" << B_state << "\n";
//    std::cout << "C =\n" << C_state << "\n";
//    std::cout << "D =\n" << D_state << "\n";
//    // print A, B, C, D for debugging
//    std::cout << "A = \n" << A << "\n";
//    std::cout << "B = \n" << B << "\n";
//    std::cout << "C = \n" << C << "\n";
//    std::cout << "D = \n" << D << "\n";
//}

void StateSpaceModel::formState(Network* net)
{
    int state_vars = net->getNumberStateVariables();
    int input_vars = net->getNumberIndependentSource();
    int outputs = net->getNumberOutputs();
    //int eqs = net->getNumberEquations();

    //if (eqs <= 0) {
    //    std::cerr << "Error:Number of equations is zero or negative! Cannot proceed.\n";
    //    return;
    //}

    // Get all index maps from network
    auto busIndex = net->getBusIndexMap();
    auto currentSourceIndex = net->getCurrentSourceIndexMap();
    auto stateVarIndex = net->getStateVarIndexMap();

    int eqs = static_cast<int>(busIndex.size() + currentSourceIndex.size() + stateVarIndex.size());

    if (eqs <= 0) {
        std::cerr << "Error: Number of equations is zero or negative! Cannot proceed.\n";
        return;
    }
    std::cout << "formState:Total equations (from indices): " << eqs << "\n";
    std::cout << "Stamping elements into MNA matrix...\n";

    // Initialize MNA matrices with appropriate dimensions
    Eigen::MatrixXd A_full_mna = Eigen::MatrixXd::Zero(eqs, eqs);
    Eigen::MatrixXd E_full_mna = Eigen::MatrixXd::Zero(eqs, eqs);
    Eigen::MatrixXd B_full_mna = Eigen::MatrixXd::Zero(eqs, std::max(1, input_vars));

    std::cout << "Stamping elements into MNA matrix...\n";

    // Stamp all elements with full index maps
    for (const auto& [name, el] : net->getElements())
    {
        std::cout << "Stamping element " << name << "\n";
       
        el->writeMNAmatrixNumeric(A_full_mna, E_full_mna, B_full_mna, eqs, -1, busIndex, currentSourceIndex, stateVarIndex);
    }

    std::cout << "\nFull MNA Matrices after Stamping\n";
    std::cout << "A_full_mna:\n" << A_full_mna << "\n";
    std::cout << "E_full_mna:\n" << E_full_mna << "\n";
    std::cout << "B_full_mna:\n" << B_full_mna << "\n";


    if (state_vars == 0) {
        std::cerr << "Error: No state variables found. Cannot extract state-space matrices.\n";
        return;
    }
    if (state_vars > eqs) {
        std::cerr << "Error: Number of state variables (" << state_vars << ") exceeds total equations (" << eqs << "). This indicates a logic error in index assignment.\n";
        return;
    }

    // Get state variable indices, sorted to ensure consistent submatrix extraction
    std::vector<int> state_indices;
    for (const auto& [el_ptr, idx] : stateVarIndex) {
        state_indices.push_back(idx);
    }
    std::sort(state_indices.begin(), state_indices.end());

    Eigen::MatrixXd E_ss(state_vars, state_vars);
    Eigen::MatrixXd A_ss(state_vars, state_vars);
    Eigen::MatrixXd B_ss(state_vars, input_vars);

    for (int i = 0; i < state_vars; i++) {
        for (int j = 0; j < state_vars; j++) {
            E_ss(i, j) = E_full_mna(state_indices[i], state_indices[j]);
            A_ss(i, j) = A_full_mna(state_indices[i], state_indices[j]);
        }
        for (int j = 0; j < input_vars; j++) {
            B_ss(i, j) = B_full_mna(state_indices[i], j);
        }
    }

    // Check invertibility of E_ss
    if (E_ss.determinant() == 0) {
        std::cerr << "E_ss matrix is singular (determinant = " << E_ss.determinant() << "); cannot invert. "
            << "This often indicates an issue with state variable selection or E_mat stamping.\n";
        std::cerr << "E_ss:\n" << E_ss << "\n";
        return;
    }

    // Calculate the final state-space matrices: x_dot = A_state * x + B_state * u
    A_mat = E_ss.inverse() * A_ss;
    B_mat = E_ss.inverse() * B_ss;

    // C and D matrices for output equation: y = C_state * x + D_state * u
    C_mat = Eigen::MatrixXd::Identity(outputs, state_vars);
    D_mat = Eigen::MatrixXd::Zero(outputs, input_vars);

    std::cout << "formState:State-space matrices extracted successfully.\n";

    // Print results from member variables
    std::cout << "A =\n" << A_mat << "\n";
    std::cout << "B =\n" << B_mat << "\n";
    std::cout << "C =\n" << C_mat << "\n";
    std::cout << "D =\n" << D_mat << "\n";
}
