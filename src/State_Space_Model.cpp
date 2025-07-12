#include "State_Space_Model.h"
#include "network.h"      
#include "Include_components.h"

void StateSpaceModel::finalizeCounts(Network* net) {
	std::unordered_map<std::string, Element*> elements = net->getElements();
	std::unordered_map<std::string, Bus*> buses = net->getBuses();

    // Clear previous indices
    bus_indices.clear();
    element_indices.clear();

	list_independent_sources.clear();
	list_state_variables.clear();
	list_switches.clear();

    number_switches = 0;
    number_independent_sources = 0;
    number_state_variables = 0;
    if (number_nodes < 0) number_nodes = 0;
    for (const auto& [name, element] : elements) {
        if (dynamic_cast<Inductor*>(element) || dynamic_cast<Capacitor*>(element)) {
            number_state_variables += element->getInputPins();
			list_state_variables.push_back(element);
        }
        else if (dynamic_cast<Switch*>(element)) {
            number_switches += element->getInputPins();
			list_switches.push_back(element);
        }
        else if (dynamic_cast<AC_source*>(element)) {
			number_independent_sources += element->getInputPins();
			list_independent_sources.push_back(element);
        }
        // Add other cases as needed
    }

    number_outputs = 0; // static_cast<int>(output_indexes.size());

	// Update the locations in the model
    number_nodes = 0;
    // Assign indices to buses, excluding the reference bus (assuming "gnd" is ground)
    for (const auto& [name, busPtr] : buses) {
        if (busPtr->getBusName() == "gnd") continue;
        bus_indices[busPtr] = number_nodes; 
        number_nodes += busPtr->getPinNumber();
    }
	// Assign indices to independent sources
    int location = number_nodes;
    for (const auto& element : list_independent_sources) {
	    element_indices[element] = location;
		location += element->getInputPins();
    }
    // Assign indices to switches
    for (const auto& element : list_switches) {
        element_indices[element] = location;
        location += element->getInputPins();
	}
	// Assign indices to state variables
    for (const auto& element : list_state_variables) {
        element_indices[element] = location;
        location += element->getInputPins();
    }

    total_number_equations = number_nodes + number_independent_sources + number_switches + number_state_variables;

    std::cout << "[Network] Nodes: " << number_nodes << std::endl;
    std::cout << "  Switches: " << number_switches << std::endl;
    std::cout << "  Independent sources: " << number_independent_sources << std::endl;
    std::cout << "  State variables: " << number_state_variables << std::endl;
    std::cout << "  Total equations: " << total_number_equations << std::endl;
    std::cout << "  Outputs: " << number_outputs << std::endl;
}


void StateSpaceModel::formState(Network* net)
{
    finalizeCounts(net); // Ensure counts are up-to-date
    std::unordered_map<std::string, Element*> elements = net->getElements();

    // Initialize MNA matrices with appropriate dimensions
    SymEngine::DenseMatrix matrix = createZeroMatrix(total_number_equations, total_number_equations+1);

    // Stamp all elements with full index maps
    int location = number_nodes;
    for (const auto& [name,element] : elements)
    {
		location = (element_indices.count(element) != 0) ? element_indices[element] : 0;
        element->writeMNAmatrix(matrix, bus_indices, location, symbols_bank);
        location += element->getInputPins();

    }

    std::cout << "[StateSpaceModel] MNA matrix populated with equations." << std::endl;
    for (int i = 0; i < total_number_equations; ++i) {
        for (int j = 0; j < total_number_equations+1; ++j) {
            std::cout << matrix.get(i, j)->__str__() << " ";
        }
        std::cout << std::endl;
    }

    // reduced_row_echelon_form
    vec_uint pivot_cols;
    reduced_row_echelon_form(matrix, matrix, pivot_cols);

 //   std::cout << "[StateSpaceModel] MNA after RREF." << std::endl;
 //   for (int i = 0; i < total_number_equations; ++i) {
 //       for (int j = 0; j < total_number_equations + 1; ++j) {
 //           std::cout << matrix.get(i, j)->__str__() << " ";
 //       }
 //       std::cout << std::endl;
 //   }
	//std::cout << "[StateSpaceModel] Pivot columns: ";
 //   for (const auto& col : pivot_cols) {
 //       std::cout << col << " ";
 //   }
 //   std::cout << std::endl;

	// Extract A, B, C, D matrices from the MNA matrix
	A = Eigen::MatrixXd::Zero(number_state_variables, number_state_variables);
	B = Eigen::MatrixXd::Zero(number_state_variables, number_independent_sources);
	C = Eigen::MatrixXd::Zero(number_outputs, number_state_variables);
	D = Eigen::MatrixXd::Zero(number_outputs, number_independent_sources);

	int index_independent_source = 0;
    for (int i = 0; i < list_state_variables.size(); i++) {
        int location1 = element_indices[list_state_variables[i]];
		// Fill A matrix
        for (int j = 0; j < list_state_variables.size(); j++) {
			int location2 = element_indices[list_state_variables[j]];
            for (int k = 0; k < list_state_variables[i]->getInputPins(); ++k) {
				RCP<const Basic> value = matrix.get(location1 + k, total_number_equations);
                for (auto& [element, symbols] : symbols_bank) {
                    if (element != list_state_variables[j]) {
                        value = substitute_symbols(value, symbols, 0.0);
                    }
                    else {
                        value = substitute_symbols(value, symbols, 1.0);
					}
                }
                A(i + k, j + k) = eval_basic(value);
			}
        }

		// Fill B matrix
        for (int j = 0; j < list_independent_sources.size(); j++) {
            int location2 = element_indices[list_independent_sources[j]];
            for (int k = 0; k < list_state_variables[i]->getInputPins(); ++k) {
                RCP<const Basic> value = matrix.get(location1 + k, total_number_equations);
                for (auto& [element, symbols] : symbols_bank) {
                    if (element != list_independent_sources[j]) {
                        value = substitute_symbols(value, symbols, 0.0);
                    }
                    else {
                        value = substitute_symbols(value, symbols, 1.0);
                    }
                }
                B(i + k, j + k) = eval_basic(value);
            }
        }
    }
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
