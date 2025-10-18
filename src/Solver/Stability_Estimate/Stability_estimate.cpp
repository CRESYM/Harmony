#include "Stability_estimate.h"

#include "../../network.h"      // For access to the Network class and its members
#include "../../Include_components.h"
#include "../../Elements/Element.h"      // For Element* operations (e.g., compute_y_parameters, getConnections)
#include "../../Bus.h"          // For Bus* methods like getBusName, getPinNumber, etc.

void StabilityEstimate::add_areas(Network* net) {
    // This function can be implemented to categorize and add AC and DC grids to the system
    // For now, it is left empty as a placeholder

    if (!net) {
        std::cerr << "Error: Null Network pointer passed to StabilityEstimate::add_areas().\n";
        return;
    }

    // Iterate over all buses in the network
    for (const auto& bus_pair : net->getBuses()) {
        Bus* bus = bus_pair.second;
        if (!bus) continue;

        std::string area = bus->getBusLocation(); // Assume Bus has getBusLocation()

        // Skip if location string is invalid
        if (area.empty()) continue;

        bool is_ac = (area[0] == 'A' || area[0] == 'a') && (area[1] == 'C' || area[1] == 'c');
        bool is_dc = (area[0] == 'D' || area[0] == 'd') && (area[1] == 'C' || area[1] == 'c');

        // Pointer to selected map
        auto& grid_map = is_ac ? ac_grids : dc_grids;
        auto& grid_names = is_ac ? ac_grid_names : dc_grid_names;

        if (!is_ac && !is_dc)
            continue; // Ignore buses not belonging to AC/DC areas

        // Create subnetwork if not already existing
        if (grid_map.find(area) == grid_map.end()) {
            grid_map[area] = new SubNetwork(area);
            grid_names.push_back(area);
        }

        // Retrieve the SubNetwork
        SubNetwork* sub = grid_map[area];

        // Add bus to the SubNetwork
        sub->addBus(bus);

        // Add connected elements
        auto conn_it = net->getConnections().find(bus);
        if (conn_it != net->getConnections().end()) {
            for (auto& elem : conn_it->second) {
                if (!elem) continue;

                // --- Check if element is an MMC dynamically ---
                MMC* mmc = dynamic_cast<MMC*>(elem);
                if (mmc) {
                    std::string conv_name = elem->getElementSymbol();
                    if (converters.find(conv_name) == converters.end()) {
                        converters[conv_name] = elem; // std::make_unique<Element>(elem);
                        std::cout << "Detected MMC converter: " << conv_name << "\n";
                    }

                    std::string loc = mmc->getElementLocation();
                    if (loc.empty()) continue;

                    // Normalize and parse format "ACi_DCj" (case-insensitive)
                    std::string loc_upper = loc;
                    std::transform(loc_upper.begin(), loc_upper.end(), loc_upper.begin(), ::toupper);

                    std::regex pattern(R"(AC(\d+)_DC(\d+))", std::regex::icase);
                    std::smatch match;
                    if (std::regex_search(loc_upper, match, pattern) && match.size() == 3) {
                        std::string ac_area = "AC" + match[1].str();
                        std::string dc_area = "DC" + match[2].str();

                        // Ensure both grids exist
                        if (ac_grids.find(ac_area) == ac_grids.end()) {
                            ac_grids[ac_area] = new SubNetwork(ac_area);
                        }
                        if (dc_grids.find(dc_area) == dc_grids.end()) {
                            dc_grids[dc_area] = new SubNetwork(dc_area);
                        }

                        // Identify converter terminal buses (AC and DC)
                        auto conns = mmc->getConnections();
                        Bus* ac_bus = nullptr;
                        Bus* dc_bus = nullptr;

                        for (const auto& [bus, terminal] : conns) {
                            if (!bus) continue;
                            std::string bname = bus->getBusLocation();
                            std::string bname_lower = bname;
                            std::transform(bname_lower.begin(), bname_lower.end(), bname_lower.begin(), ::tolower);

                            if (bname_lower.rfind("ac", 0) == 0)
                                ac_bus = bus;
                            else if (bname_lower.rfind("dc", 0) == 0)
                                dc_bus = bus;
                        }

                         // Add buses as outputs for their corresponding grids
                        if (ac_bus) {
                            ac_grids[ac_area]->addOutput(ac_bus->getBusName(), ac_bus);
                        }
                        if (dc_bus) {
                            dc_grids[dc_area]->addOutput(dc_bus->getBusName(), dc_bus);
                        }
                    }
                    else {
                        std::cerr << "[WARN] Converter " << mmc->getElementSymbol()
                            << " has invalid location format: " << loc << "\n";
                    }
                }
                else {
                    sub->addElement(elem);
					// sub->connectElementToBus(elem, elem->getBusIndex(bus), bus);
                }
         
            }
        }
    }

    // Optional summary output
    std::cout << "\n=== StabilityEstimate: Area Summary ===\n";
    std::cout << "AC Grids Detected: " << ac_grids.size() << "\n";
    for (const auto& [name, sub] : ac_grids) {
        std::cout << "  - " << name << " (" << sub->getBuses().size() << " buses, "
            << sub->getElements().size() << " elements, " << sub->getOutputs().size() << " outputs)\n";
    }

    std::cout << "DC Grids Detected: " << dc_grids.size() << "\n";
    for (const auto& [name, sub] : dc_grids) {
        std::cout << "  - " << name << " (" << sub->getBuses().size() << " buses, "
            << sub->getElements().size() << " elements, " << sub->getOutputs().size() << " outputs)\n";
    }
}

void StabilityEstimate::compute_equivalent_impedance(Network* net, std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements) {
    if (start_buses.empty())
    {
        throw std::invalid_argument("There is no start buses.");
        exit(1);
    }

    // Erase duplicates in start and end buses, remove gnd from the list of buses
    sort(start_buses.begin(), start_buses.end()); start_buses.erase(unique(start_buses.begin(), start_buses.end()), start_buses.end());
    sort(end_buses.begin(), end_buses.end()); end_buses.erase(unique(end_buses.begin(), end_buses.end()), end_buses.end());
    // Remove the element using erase function and iterators
    for (int i = 0; i < start_buses.capacity(); i++) {  // Ensure 'gnd' is not in start buses
        if (start_buses[i]->getBusName() == "gnd") { // leave the function
            throw std::invalid_argument("Ground cannot be a start bus.");
            exit(1);
        }
    }

    // check bus positions in matrix, add only buses which are not gnd
    int pos = 0;
    std::vector<int> positions_currents;
    std::unordered_map<Bus*, int> all_buses;
    // Assign positions to start buses
    for (auto& bus : start_buses) {
        all_buses[bus] = pos; pos += bus->getPinNumber();
        positions_currents.push_back(pos);
        pos += bus->getPinNumber();
    }
    int equivalent_impedance_size = pos / 2;

    // Assign positions to end buses
    for (auto& bus : end_buses) {
        if (bus->getBusName() != "gnd") {
            all_buses[bus] = pos; pos += bus->getPinNumber();
            positions_currents.push_back(pos);
            pos += bus->getPinNumber();
        }
    }

	auto& buses = net->getBuses(); // Get all buses from the network
    // Assign positions to other buses not in start or end buses
    for (const auto& bus : buses) {
        if (std::find(start_buses.begin(), start_buses.end(), bus.second) == start_buses.end()
            && std::find(end_buses.begin(), end_buses.end(), bus.second) == end_buses.end()) {
            if (bus.second->getBusName() != "gnd") {
                all_buses[bus.second] = pos;
                pos += bus.second->getPinNumber();
            }
        }
    }


    // Initialize the admittance matrix Y as a zero matrix
    DenseMatrix Y = createZeroMatrix(pos, pos + 1);

    // Make MNA with excuded elements
    // Go through buses (connections) and add element Y parameters the element should not be skipped.
    // Order for writing equations input voltage and current, then output voltage and current, and then other voltages.
    std::unordered_map<Bus*, std::vector<Element*>> connections = net->getConnections(); // Get connections from the network
    pos = 0; // position in positions_currents
    for (auto& bus : all_buses) {
        for (auto& element : connections[bus.first]) {
            // Skip elements that are in the skip_elements list
            if (std::find(skip_elements.begin(), skip_elements.end(), element) == skip_elements.end()) {
                // Get the Y-parameter matrix of the element
                DenseMatrix element_Y_matrix = element->get_y_parameters();

                // add element_Y_matrix
                std::map<Bus*, int> bus_map = element->getConnections();
                int terminal = bus_map[bus.first] - 1; int pins = bus.first->getPinNumber();
                int position = bus.second;
                // get the other bus and add -element_Y_matrix
                Bus* other_bus = element->getOtherBus(bus.first);
                int terminal_other = bus_map[other_bus] - 1; int pins_other = other_bus->getPinNumber();
                int position_other = all_buses[other_bus];
                for (int i = 0; i < pins; i++) {
                    for (int j = 0; j < pins; j++) {
                        Y.set(position + i, position + j, add(Y.get(position + i, position + j),
                            element_Y_matrix.get(terminal * pins + i, terminal * pins + j)));
                    }
                    for (int j = 0; j < pins_other; j++) {
                        if (other_bus->getBusName() != "gnd") {
                            Y.set(position + i, position_other + j, add(Y.get(position_other + i, position_other + j),
                                element_Y_matrix.get(terminal * pins + i, terminal_other * pins_other + j)));
                        }
                    }
                }

            }

        }
        // check if there is current associated to bus
        int position = bus.second; int pins = bus.first->getPinNumber();
        if (pos < positions_currents.capacity()) {
            // check it if belongs to start_buses
            if (pos < start_buses.capacity()) {
                for (int i = 0; i < pins; i++) {
                    Y.set(position + i, positions_currents[pos] + i, integer(-1));
                    Y.set(positions_currents[pos] + i, position + i, integer(1));
                    Y.set(positions_currents[pos] + i, Y.ncols() - 1, symbol("V" + std::to_string(pos + i)));
                }
            }
            else { // it belongs to end buses
                for (int i = 0; i < pins; i++) {
                    Y.set(position + i, positions_currents[pos] + i, integer(1));
                    Y.set(positions_currents[pos] + i, position + i, integer(1));
                    Y.set(positions_currents[pos] + i, Y.ncols() - 1, symbol("V" + std::to_string(pos + i)));
                }
            }
        }
        pos++;
    }

	cout << Y.__str__() << endl;

    // reduced_row_echelon_form
    vec_uint pivot_cols;
    reduced_row_echelon_form(Y, Y, pivot_cols);

    //std::cout << "Reduced row echelon form gives: " << std::endl;
    //for (int i = 0; i < Y.nrows(); i++) {
    //    for (int j = 0; j < Y.ncols(); j++)
    //        std::cout << Y.get(i, j)->__str__() << " ";
    //    std::cout << endl;
    //}


    //Compute the equivalent impedance
    DenseMatrix equivalent_impedance = createZeroMatrix(equivalent_impedance_size, 1);
    pos = 0;
    for (auto& bus : start_buses) {
        int pos_voltage = all_buses[bus]; int position_current = positions_currents[pos];
        int pins = bus->getPinNumber();
        for (int i = 0; i < pins; i++) {
            equivalent_impedance.set(pos_voltage + i, 0, div(Y.get(pos_voltage + i, Y.ncols() - 1), Y.get(position_current + i, Y.ncols() - 1)));
        }
        pos++;
    }
    // Print the equivalent impedance
    std::cout << "Equivalent impedance symbolic is: " << std::endl;
    for (int i = 0; i < equivalent_impedance.nrows(); i++) {
        std::cout << equivalent_impedance.get(i, 0)->__str__() << " ";
        std::cout << endl;
    }
    //cout << equivalent_impedance_size << endl;
}

MatrixXcd StabilityEstimate::compute_equivalent_impedance_num(Network* net, std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements, double frequency)
{
    if (start_buses.empty())
    {
        throw std::invalid_argument("There is no start buses.");
        exit(1);
    }

    // Erase duplicates in start and end buses, remove 'gnd' from the list of buses
    sort(start_buses.begin(), start_buses.end());
    start_buses.erase(unique(start_buses.begin(), start_buses.end()), start_buses.end());
    sort(end_buses.begin(), end_buses.end());
    end_buses.erase(unique(end_buses.begin(), end_buses.end()), end_buses.end());

    // Remove the element using erase function and iterators
    for (int i = 0; i < start_buses.capacity(); i++) {  // Ensure 'gnd' is not in start buses
        if (start_buses[i]->getBusName() == "gnd") { // leave the function
            throw std::invalid_argument("Ground cannot be a start bus.");
            exit(1);
        }
    }

    // Check bus positions in the matrix, add only buses that are not 'gnd'
    int pos = 0;
    std::unordered_map<Bus*, int> all_buses;
    std::vector<int> positions_currents;

    // Assign positions to start buses
    for (auto& bus : start_buses) {
        all_buses[bus] = pos;
        pos += bus->getPinNumber();
        positions_currents.push_back(pos);
        pos += bus->getPinNumber();
    }
    int equivalent_impedance_size = pos / 2;

    // Assign positions to end buses
    for (const auto& bus : end_buses) {
        if (bus->getBusName() != "gnd") {
            all_buses[bus] = pos;
            pos += bus->getPinNumber();
            positions_currents.push_back(pos);
            pos += bus->getPinNumber();
        }
    }

	auto& buses = net->getBuses(); // Get all buses from the network
    //Assign positions to other buses that are not in start or end buses
    for (const auto& bus : buses) {
        if (std::find(start_buses.begin(), start_buses.end(), bus.second) == start_buses.end() &&
            std::find(end_buses.begin(), end_buses.end(), bus.second) == end_buses.end()) {
            if (bus.second->getBusName() != "gnd") {
                all_buses[bus.second] = pos;
                pos += bus.second->getPinNumber();
            }
        }
    }

    // Initialize the admittance matrix
    Eigen::MatrixXcd Y = Eigen::MatrixXcd::Zero(pos, pos + 1);

	std::unordered_map<Bus*, std::vector<Element*>> connections = net->getConnections(); // Get connections from the network
    // Populate the admittance matrix
    pos = 0;
    for (auto& bus : all_buses) {
        for (auto& element : connections[bus.first]) {
            // Skip elements that are in the skip_elements list
            if (std::find(skip_elements.begin(), skip_elements.end(), element) == skip_elements.end()) {
                // Get the Y-parameter matrix of the element
                std::vector<std::vector<complex<double>>> element_Y_matrix = element->compute_y_parameters(frequency);  // Use numerical Y

                // Add element_Y_matrix
                std::map<Bus*, int> bus_map = element->getConnections();
                int terminal = bus_map[bus.first] - 1;
                int pins = bus.first->getPinNumber();
                int position = bus.second;
                // Get the other bus and add -element_Y_matrix
                Bus* other_bus = element->getOtherBus(bus.first);
                int terminal_other = bus_map[other_bus] - 1;
                int pins_other = other_bus->getPinNumber();
                int position_other = all_buses[other_bus];

                // Update the admittance matrix with element's Y parameters
                for (int i = 0; i < pins; i++) {
                    for (int j = 0; j < pins; j++) {
                        Y(position + i, position + j) += element_Y_matrix[terminal * pins + i][terminal* pins + j];
                    }
                    for (int j = 0; j < pins_other; j++) {
                        if (other_bus->getBusName() != "gnd") {
                            Y(position + i, position_other + j) += element_Y_matrix[terminal * pins + i][terminal_other * pins_other + j];
                        }
                    }
                }
            }
        }

        // Check if there is current associated with bus
        int position = bus.second;
        int pins = bus.first->getPinNumber();
        if (pos < positions_currents.capacity()) {
            // Check if it belongs to start_buses
            if (pos < start_buses.capacity()) {
                for (int i = 0; i < pins; i++) {
                    Y(position + i, positions_currents[pos] + i) = -1;
                    Y(positions_currents[pos] + i, position + i) = 1;
                    Y(positions_currents[pos] + i, Y.cols() - 1) = std::complex<double>(1, 0);  // Symbol
                }
            }
            else { // It belongs to end buses
                for (int i = 0; i < pins; i++) {
                    Y(position + i, positions_currents[pos] + i) = 1;
                    Y(positions_currents[pos] + i, position + i) = 1;
                    Y(positions_currents[pos] + i, Y.cols() - 1) = std::complex<double>(1, 0);  // Symbol
                }
            }
        }
        pos++;
    }

    // Extract the reduced admittance matrix (block)
    Eigen::MatrixXcd reduced_Y = Y.block(0, 0, pos, pos);  // Extract the block
    Eigen::VectorXcd b = Y.block(0, pos, pos, 1);          // Extract the RHS vector


    // Solve the linear system Y * x = b
    Eigen::VectorXcd solution = reduced_Y.partialPivLu().solve(b);

    Eigen::MatrixXcd equivalent_impedance = Eigen::MatrixXcd::Zero(equivalent_impedance_size, 1);
    pos = 0;
    for (auto& bus : start_buses) {
        int pos_voltage = all_buses[bus];
        int position_current = positions_currents[pos];
        int pins = bus->getPinNumber();
        for (int i = 0; i < pins; i++) {
            equivalent_impedance(pos_voltage + i, 0) = solution(pos_voltage + i) / solution(position_current + i);
        }
        pos++;
    }

    // Print the equivalent impedance
    std::cout << "Equivalent impedance numeric is: " << std::endl;
    for (int i = 0; i < equivalent_impedance.rows(); i++) {
        std::cout << equivalent_impedance(i, 0) << " ";
    }
    std::cout << std::endl;

	return equivalent_impedance;
}

MatrixXcd StabilityEstimate::compute_equivalent_admittance_parameters_num(SubNetwork* subnet, double frequency)
{
    if (!subnet)
        throw std::invalid_argument("Null SubNetwork pointer passed to compute_equivalent_impedance_num().");

    std::unordered_map<std::string, Bus*> start_buses = subnet->getOutputs();

    // --- Assign matrix positions ---
    int pos = 0;
    std::unordered_map<Bus*, int> bus_positions;
    std::vector<int> current_positions;
    for (auto& [name_bus, bus] : start_buses) {
        bus_positions[bus] = pos;
        pos += bus->getPinNumber();
        current_positions.push_back(pos);
    }
    for (int i = 0; i < pos; i++) {
        current_positions[i] += pos - 1;
	}
    int equivalent_impedance_size = pos;

    // Add remaining buses from subnet
    for (const auto& [busName, bus] : subnet->getBuses()) {
        if (bus->getBusName() == "gnd") continue;
        if (bus_positions.find(bus) == bus_positions.end()) {
            bus_positions[bus] = pos;
            pos += bus->getPinNumber();
        }
    }

    // --- Initialize admittance matrix ---
    Eigen::MatrixXcd Y = Eigen::MatrixXcd::Zero(2*pos, 2*pos);
	VectorXcd z = VectorXcd::Zero(2*pos,1);

    const auto& buses = subnet->getBuses();

    // --- Build Y-matrix ---
    for (const auto& [bus_name, bus] : buses) {
		auto& elements = bus->getConnectedElements();
		//cout << "Processing bus: " << bus->getBusName() << " with " << elements.size() << " elements.\n";
        for (Element* element : elements) {
			// Skip if elemenet if is null, MMC, or not connected to the bus
            if (!element) continue;
            MMC* mmc = dynamic_cast<MMC*>(element);
			if (mmc) continue; // Skip MMCs             
            const auto& elem_conns = element->getConnections();
            if (elem_conns.find(bus) == elem_conns.end())
                continue;
            Bus* other_bus = element->getOtherBus(bus);
            if (!other_bus || bus->getBusName() == "gnd")
                continue;

            // Compute numerical Y-parameters
            std::vector<std::vector<complex<double>>> Ye = element->compute_y_parameters(frequency);

            int bus_pos = bus_positions[bus];
            int other_pos = bus_positions[other_bus];
            int pins = bus->getPinNumber();
            int other_pins = other_bus->getPinNumber();

            int terminal = elem_conns.at(bus) - 1;
            int terminal_other = elem_conns.at(other_bus) - 1;

            // Fill admittance terms
            for (int i = 0; i < pins; ++i) {
                for (int j = 0; j < pins; ++j)
                    Y(bus_pos + i, bus_pos + j) += Ye[terminal * pins + i][terminal * pins + j];

                for (int j = 0; j < other_pins; ++j)
                    Y(bus_pos + i, other_pos + j) += Ye[terminal * pins + i][terminal_other * other_pins + j];
            }
        }
    }

    // --- Apply current sources for start and end buses ---
    int idx = 0;
	int num_start_buses = start_buses.size();
    for (auto& [name_bus, bus] : start_buses) {
        int bus_pos = bus_positions[bus];
        int curr_pos = current_positions[idx];
        int pins = bus->getPinNumber();

        for (int i = 0; i < pins; ++i) {
            Y(bus_pos + i, curr_pos + i) = -1;
            Y(curr_pos + i, bus_pos + i) = 1;
        }
        idx++;
    }

    MatrixXcd Y_params = MatrixXcd::Zero(num_start_buses, num_start_buses);

    // Iterate through the combinations of start buses to get admittance parameters
    // e.g. start for bus 1 = 1V, bus2-n = 0V, then bus1 = 0V, bus2 = 1V, etc.
    // and estimate Y11, Y12-n, etc.
	int bus_vector_pos = 0;
    for (auto& [name_bus, bus] : start_buses) {
        int bus_pos = bus_positions[bus];
        int pins = bus->getPinNumber();
        // Set voltage source for this bus
        for (int i = 0; i < pins; ++i) {
            z(current_positions[bus_vector_pos] + i, 0) = std::complex<double>(1, 0); // 1V source
        }
        // Solve Y * x = z
        Eigen::VectorXcd solution = Y.partialPivLu().solve(z);

		// Store results in Y_params
		idx = 0;
        for (idx = 0; idx < num_start_buses; idx++) {
            for (int j = 0; j < pins; j++) {
				Y_params(idx + j, bus_vector_pos + j) = solution(current_positions[idx] + j, 0);
			}	
        }
        
        // Reset voltage source for this bus to 0V for next iteration
		z.setZero();
		// Move to next bus
		bus_vector_pos++;
	}
 
   	return Y_params;
}

void StabilityEstimate::print_summary() const {
    std::cout << "\n================= STABILITY ESTIMATE SUMMARY =================\n";

    // --- AC Grids ---
    std::cout << "\n--- AC Grids (" << ac_grids.size() << ") ---\n";
    if (ac_grids.empty()) {
        std::cout << "No AC subnetworks found.\n";
    }
    else {
        for (const auto& [name, sub] : ac_grids) {
            std::cout << "AC Grid: " << name << "\n";
            std::cout << "  Buses (" << sub->getBuses().size() << "): ";
            for (const auto& [busName, busPtr] : sub->getBuses())
                std::cout << busName << " ";
            std::cout << "\n  Elements (" << sub->getElements().size() << "): ";
            for (const auto& [elName, elPtr] : sub->getElements())
                std::cout << elName << " ";
			std::cout << "\n  Outputs (" << sub->getOutputs().size() << "): ";
			for (auto& [outName, outBus] : sub->getOutputs())
				std::cout << outName << " ";
            std::cout << "\n\n";
        }
    }

    // --- DC Grids ---
    std::cout << "\n--- DC Grids (" << dc_grids.size() << ") ---\n";
    if (dc_grids.empty()) {
        std::cout << "No DC subnetworks found.\n";
    }
    else {
        for (const auto& [name, sub] : dc_grids) {
            std::cout << "DC Grid: " << name << "\n";
            std::cout << "  Buses (" << sub->getBuses().size() << "): ";
            for (const auto& [busName, busPtr] : sub->getBuses())
                std::cout << busName << " ";
            std::cout << "\n  Elements (" << sub->getElements().size() << "): ";
            for (const auto& [elName, elPtr] : sub->getElements())
                std::cout << elName << " ";
			std::cout << "\n  Outputs (" << sub->getOutputs().size() << "): ";
            for (auto& [outName, outBus] : sub->getOutputs())
				std::cout << outName << " ";
            std::cout << "\n\n";
        }
    }

    // --- Converters ---
    std::cout << "\n--- Converters (" << converters.size() << ") ---\n";
    if (converters.empty()) {
        std::cout << "No MMC converters detected.\n";
    }
    else {
        for (const auto& [name, elem] : converters) {
            std::cout << "Converter: " << name << "\n";
        }
    }

    std::cout << "\n===============================================================\n";
}

MatrixXcd StabilityEstimate::compute_transfer_function(string converter_name, string location, double frequency) {
    // Check if converter exists
    if (converters.find(converter_name) == converters.end()) {
        std::cerr << "Error: Converter " << converter_name << " not found.\n";
        return MatrixXcd::Zero(1, 1);
    }
    Element* converter = converters[converter_name];
    MMC* mmc = dynamic_cast<MMC*>(converter);
    if (!mmc) {
        std::cerr << "Error: Element " << converter_name << " is not an MMC.\n";
        return MatrixXcd::Zero(1,1);
    }

    std::string ac_area = mmc->getACarea();
    std::string dc_area = mmc->getDCarea();

    // Compute equivalent Y parameters first for AC grids and then for DC grids
    // DC grids
    int dc_side_pins = 0;
    unordered_map<string, MatrixXcd> Y_dc_matrices;
    for (auto& [name, sub] : dc_grids) {
        std::cout << "Computing equivalent admittance for DC grid: " << name << "\n";
        MatrixXcd Y_dc = compute_equivalent_admittance_parameters_num(sub, frequency);
        Y_dc_matrices[name] = Y_dc;
        dc_side_pins = Y_dc.rows(); // Assuming single bus for DC grid
        std::cout << "Equivalent admittance matrix for DC grid " << name << ":\n" << Y_dc << "\n";
    }
    // AC grids
    unordered_map<string, MatrixXcd> Y_ac_matrices;
    for (auto& [name, sub] : ac_grids) {
        std::cout << "Computing equivalent admittance for AC grid: " << name << "\n";
        MatrixXcd Y_ac = compute_equivalent_admittance_parameters_num(sub, frequency);
        Y_ac_matrices[name] = Y_ac;
        std::cout << "Equivalent admittance matrix for AC grid " << name << ":\n" << Y_ac << "\n";
    }

    // Cross coupling for the admittance of each converter
    // Depending on the side of the converter, the admittance matrix will be different
    // If it is not main converter, then the converter is always considered from DC side.
    // If it is main converter, then if the location is on DC side, then the same applies.
    int converter_number = converters.size();
    MatrixXcd Z_closing = MatrixXcd::Zero((converter_number - 1) * dc_side_pins, (converter_number - 1) * dc_side_pins);
    unordered_map<string, MatrixXcd> Y_conv_matrices;
    int index = 0;
    for (auto& [name, elem] : converters) {
        MMC* mmc_elem = dynamic_cast<MMC*>(elem);
        if (!mmc_elem) continue;
        // Identify converter terminal buses (AC and DC)
        auto conns_elem = mmc_elem->getConnections();
        Bus* ac_bus = nullptr;
        Bus* dc_bus = nullptr;
        for (const auto& [bus, terminal] : conns_elem) {
            if (!bus) continue;
            std::string bname = bus->getBusLocation();
            std::string bname_lower = bname;
            std::transform(bname_lower.begin(), bname_lower.end(), bname_lower.begin(), ::tolower);
            if (bname_lower.rfind("ac", 0) == 0)
                ac_bus = bus;
            else if (bname_lower.rfind("dc", 0) == 0)
                dc_bus = bus;
        }
        if (!ac_bus || !dc_bus) continue;
        // Get admittance matrices for the grids

        // Depending on the side of the converter, the admittance matrix looking inside converter will be different
        if ((converter_name != name) || (location == dc_area)) {
            MatrixXcd Y_ac = Y_ac_matrices[ac_area];
            MatrixXcd Ymmc = vectorToMatrix(mmc_elem->compute_y_parameters(frequency));
            // The overall transfer function considering the converter's own admittance and the grid admittances.
            MatrixXcd Ydc(1, 1);
            Ydc(0, 0) = Ymmc(0, 0);
            MatrixXcd b = Ymmc.block(0, 1, 0, 2);
            MatrixXcd a = Ymmc.block(1, 0, 2, 0);
            MatrixXcd Ydq = Ymmc.block(1, 1, 2, 2);
            Y_conv_matrices[name] = b * (Y_ac - Ydq).inverse() * a + Ydc;

            Z_closing.block(index, index, dc_side_pins, dc_side_pins) = Y_conv_matrices[name].inverse();
        }
        index++;
    }


    // CROSS-COUPLING OF THE DC SIDE OF THE CONVERTER
    // with the DC grid admittance
    MatrixXcd Y_dc = compute_closing_impedance(dc_grids[dc_area], dc_area, Y_dc_matrices[dc_area], Z_closing);

    // Final transfer function computation
    // For DC cut 
    if (location == dc_area) {
        MatrixXcd Y_ac = Y_ac_matrices[ac_area];
        MatrixXcd Ymmc = vectorToMatrix(mmc->compute_y_parameters(frequency));
        // The overall transfer function considering the converter's own admittance and the grid admittances.
        MatrixXcd Ydc(1, 1);
        Ydc(0, 0) = Ymmc(0, 0);
        MatrixXcd b = Ymmc.block(0, 1, 0, 2);
        MatrixXcd a = Ymmc.block(1, 0, 2, 0);
        MatrixXcd Ydq = Ymmc.block(1, 1, 2, 2);
        Y_conv_matrices[converter_name] = b * (Y_ac - Ydq).inverse() * a + Ydc;
		MatrixXcd TF = Y_conv_matrices[converter_name] * Y_dc;

        return TF;
    }
    else {
        // For AC cut - compute the overall admittance looking into the converter from AC side
        // i.e. repeat the same procedure as above but now from AC side
        MatrixXcd Ymmc = vectorToMatrix(mmc->compute_y_parameters(frequency));
        // The overall transfer function considering the converter's own admittance and the grid admittances.
        MatrixXcd Ydq(2, 2);
        Ydq = Ymmc.block(1, 1, 2, 2);
        MatrixXcd b = Ymmc.block(1, 0, 2, 0);
        MatrixXcd a = Ymmc.block(0, 1, 0, 2);
        MatrixXcd Ydc = Ymmc.block(0, 0, 1, 1);
        Y_conv_matrices[converter_name] = (Y_dc - Ydc).inverse() * a * b + Ydq;
		MatrixXcd TF = Y_conv_matrices[converter_name] * Y_ac_matrices[ac_area];

		return TF;
    }
}

MatrixXcd StabilityEstimate::compute_closing_impedance(SubNetwork* sub, string& connection_name, MatrixXcd& Y_parameters, MatrixXcd& Z_closing) {
	// Get the list of output ports from the subnetwork
    auto outputs = sub->getOutputs();
    int total_ports = outputs.size();

    // Find the index of the input port
    int input_idx = -1;
    std::vector<std::string> output_names;
    output_names.reserve(total_ports);
    int i = 0;
    for (const auto& [name, bus] : outputs) {
        output_names.push_back(name);
        if (name == connection_name) {
            input_idx = i;
        }
        i++;
    }

    if (input_idx == -1) {
        throw std::runtime_error("Input connection name not found in subnetwork outputs.");
    }

    // Determine the size of the output block
    int N = total_ports - 1;
    if (N == 0) {
        // If there are no other outputs, the closing impedance is simply Z_closing
        return Z_closing;
    }

    // Partition the Y_parameters matrix
    MatrixXcd Y11(1, 1);
    MatrixXcd Y12(1, N);
    MatrixXcd Y21(N, 1);
    MatrixXcd Y22(N, N);

    Y11(0, 0) = Y_parameters(input_idx, input_idx);

    int out_row = 0;
    for (int row = 0; row < total_ports; ++row) {
        if (row == input_idx) continue;

        int out_col = 0;
        for (int col = 0; col < total_ports; ++col) {
            if (col == input_idx) continue;
            Y22(out_row, out_col) = Y_parameters(row, col);
            out_col++;
        }

        Y21(out_row, 0) = Y_parameters(row, input_idx);
        out_row++;
    }

    int out_col = 0;
    for (int col = 0; col < total_ports; ++col) {
        if (col == input_idx) continue;
        Y12(0, out_col) = Y_parameters(input_idx, col);
        out_col++;
    }

    // Compute the closing impedance using the partitioned matrices
    // Z_eq = (Y11 - Y12 * (Y22 + Y_load)^-1 * Y21)^-1
    // Assuming Y_load is represented by Z_closing.inverse()
    // MatrixXcd Y_load = Z_closing.inverse();
	MatrixXcd I = MatrixXcd::Identity(Y22.rows(), Y22.cols());
    MatrixXcd Z_eq = (Y11 - Y12 * (I + Y22 * Z_closing).inverse() * Y21).inverse();

    return Z_eq;
}
