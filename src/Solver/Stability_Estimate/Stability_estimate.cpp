/**
 * @file Stability_estimate.cpp
 * @brief Implementation of Small-signal stability and impedance analysis for AC/DC areas.
 */
﻿#include "Stability_estimate.h"

#include "../../network.h"      // For access to the Network class and its members
#include "../../Include_components.h"
#include "../../Bus.h"          // For Bus* methods like getBusName, getPinNumber, etc.

void StabilityEstimate::add_areas(Network* net) {
    if (net->is_area_empty())
        net->add_areas();

    ac_grid_names = net->get_ac_grid_names();
    dc_grid_names = net->get_dc_grid_names();
    ac_grids = net->get_ac_grids();
    dc_grids = net->get_dc_grids();
    converters = net->get_converters();
}

/**
 * @brief Computes the symbolic equivalent impedance of a portion of the network.
 * @param net Pointer to the main Network object.
 * @param start_buses Vector of buses where current is injected.
 * @param end_buses Vector of buses where current is extracted.
 * @param skip_elements Vector of elements to exclude from the calculation.
 * @throws std::invalid_argument if start_buses is empty or contains 'gnd'.
 */
void StabilityEstimate::compute_equivalent_impedance(Network* net, std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements) {
    if (start_buses.empty())
    {
        throw std::invalid_argument("There is no start buses.");
    }

    // Erase duplicates in start and end buses, remove gnd from the list of buses
    sort(start_buses.begin(), start_buses.end()); start_buses.erase(unique(start_buses.begin(), start_buses.end()), start_buses.end());
    sort(end_buses.begin(), end_buses.end()); end_buses.erase(unique(end_buses.begin(), end_buses.end()), end_buses.end());
    // Remove the element using erase function and iterators
    for (int i = 0; i < start_buses.size(); i++) {  // Ensure 'gnd' is not in start buses
        if (start_buses[i]->isGround()) { // leave the function
            throw std::invalid_argument("Ground cannot be a start bus.");
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
        if (!bus->isGround()) {
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
            if (!bus.second->isGround()) {
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
                        if (!other_bus->isGround()) {
                            Y.set(position + i, position_other + j, add(Y.get(position_other + i, position_other + j),
                                element_Y_matrix.get(terminal * pins + i, terminal_other * pins_other + j)));
                        }
                    }
                }

            }

        }
        // check if there is current associated to bus
        int position = bus.second; int pins = bus.first->getPinNumber();
        if (pos < positions_currents.size()) {
            // check it if belongs to start_buses
            if (pos < start_buses.size()) {
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

/**
 * @brief Computes the numerical equivalent impedance of a portion of the network.
 * @param net Pointer to the main Network object.
 * @param start_buses Vector of buses where current is injected.
 * @param end_buses Vector of buses where current is extracted.
 * @param skip_elements Vector of elements to exclude from the calculation.
 * @param frequency The frequency in Hz for the numerical calculation.
 * @return An Eigen::MatrixXcd containing the computed equivalent impedance.
 * @throws std::invalid_argument if start_buses is empty or contains 'gnd'.
 */
MatrixXcd StabilityEstimate::compute_equivalent_impedance_num(Network* net, std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements, double frequency)
{
    if (start_buses.empty())
    {
        throw std::invalid_argument("There is no start buses.");
    }

    // Erase duplicates in start and end buses, remove 'gnd' from the list of buses
    sort(start_buses.begin(), start_buses.end());
    start_buses.erase(unique(start_buses.begin(), start_buses.end()), start_buses.end());
    sort(end_buses.begin(), end_buses.end());
    end_buses.erase(unique(end_buses.begin(), end_buses.end()), end_buses.end());

    // Remove the element using erase function and iterators
    for (int i = 0; i < start_buses.size(); i++) {  // Ensure 'gnd' is not in start buses
        if (start_buses[i]->isGround()) { // leave the function
            throw std::invalid_argument("Ground cannot be a start bus.");
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
        if (!bus->isGround()) {
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
            if (!bus.second->isGround()) {
                all_buses[bus.second] = pos;
                pos += bus.second->getPinNumber();
            }
        }
    }

    // Initialize the admittance matrix
    Eigen::MatrixXcd Y = Eigen::MatrixXcd::Zero(pos, pos + 1);

    std::unordered_set<Element*> processed_elements;
	std::unordered_map<Bus*, std::vector<Element*>> connections = net->getConnections(); // Get connections from the network
    // Populate the admittance matrix
    pos = 0;
    for (auto& bus : all_buses) {
        for (auto& element : connections[bus.first]) {
            // Skip elements that are in the skip_elements list
            if (processed_elements.count(element)) continue;
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
                        if (!other_bus->isGround()) {
                            Y(position + i, position_other + j) += element_Y_matrix[terminal * pins + i][terminal_other * pins_other + j];
                        }
                    }
                }
            }
            processed_elements.insert(element);  // ADD at end of element block (before closing brace)
        }

        // Check if there is current associated with bus
        int position = bus.second;
        int pins = bus.first->getPinNumber();
        if (pos < positions_currents.size()) {
            // Check if it belongs to start_buses
            if (pos < start_buses.size()) {
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

/**
 * @brief Computes the numerical equivalent multi-port admittance parameters for a subnetwork.
 * @param subnet Pointer to the SubNetwork object.
 * @param frequency The frequency in Hz for the calculation.
 * @return An Eigen::MatrixXcd containing the computed Y-parameter matrix for the subnetwork's output ports.
 * @throws std::invalid_argument if the subnet pointer is null or the subnetwork name is invalid.
 */
MatrixXcd StabilityEstimate::compute_equivalent_admittance_parameters_num(SubNetwork* subnet, double frequency)
{
    if (!subnet)
        throw std::invalid_argument("Null SubNetwork pointer passed to compute_equivalent_impedance_num().");

    std::unordered_map<std::string, Bus*> start_buses = subnet->getOutputs();
	string subnet_name = subnet->getName();
    int pins = 1;
    if (subnet_name[0] == 'A' || subnet_name[0] == 'a') {
        if (subnet->getTransformation()) {
            pins = 2; // Assume 2 pins for AC networks in dq frame
        }
        else
			pins = 3; // Assume 3 pins for AC networks in abc frame
    }
    else if (subnet_name[0] == 'D' || subnet_name[0] == 'd') {
        pins = 1; // Assume 1 pin for DC networks
    }
    else {
        throw std::invalid_argument("SubNetwork name must start with 'AC' or 'DC' to determine pin count.");
	}

    // --- Assign matrix positions ---
    int pos = 0;
    std::unordered_map<Bus*, int> bus_positions;
	std::unordered_map<Bus*, int> bus_current_positions;
    for (auto& [name_bus, bus] : start_buses) {
        bus_positions[bus] = pos;
        pos += pins;
		bus_current_positions[bus] = pos;
		pos += pins;
    }

    // Add remaining buses from subnet
    for (const auto& [busName, bus] : subnet->getBuses()) {
        if (bus->isGround()) continue;
        if (bus_positions.find(bus) == bus_positions.end()) {
            bus_positions[bus] = pos;
            pos += pins;
        }
    }

	// cout << pos << endl;

    // --- Initialize admittance matrix ---
    Eigen::MatrixXcd Y = Eigen::MatrixXcd::Zero(pos, pos);
	VectorXcd z = VectorXcd::Zero(pos,1);

    const auto& buses = subnet->getBuses();
    std::unordered_set<Element*> processed_elements; // Set to track processed elements

    // --- Build Y-matrix ---
    for (const auto& [bus_name, bus] : buses) {
		auto elements = bus->getConnectedElements();
        for (Element* element : elements) {
			// Skip if element is null, a Converter, already processed, or connected to ground
            if (!element || dynamic_cast<Converter*>(element) || processed_elements.count(element)) {
                continue;
            }
                     
            const auto& elem_conns = element->getConnections();
            if (elem_conns.find(bus) == elem_conns.end() || bus->isGround()) {
                continue;
            }

            Bus* other_bus = element->getOtherBus(bus);

            // Compute numerical Y-parameters
            std::vector<std::vector<complex<double>>> Ye = element->compute_y_parameters(frequency);

			//cout << "Element " << element->getElementSymbol() << " Y-parameters " << setprecision(10) << vectorToMatrix(Ye) << endl;

            int bus_pos = bus_positions[bus];
            int terminal = elem_conns.at(bus) - 1;
			int terminal_other = 1 - terminal;

            // Add all contributions of the element to the Y-matrix at once
            for (int i = 0; i < pins; ++i) {
                for (int j = 0; j < pins; ++j) {
                    // Y_bus,bus
                    Y(bus_pos + i, bus_pos + j) += Ye[terminal * pins + i][terminal * pins + j];
                                        
                    if (other_bus && !other_bus->isGround()) {
                        int other_pos = bus_positions[other_bus];
                        // Y_bus,other_bus
                        Y(bus_pos + i, other_pos + j) += Ye[terminal * pins + i][terminal_other * pins + j];
                        // Y_other_bus,bus
                        Y(other_pos + i, bus_pos + j) += Ye[terminal_other * pins + i][terminal * pins + j];
                        // Y_other_bus,other_bus
                        Y(other_pos + i, other_pos + j) += Ye[terminal_other * pins + i][terminal_other * pins + j];
                    }
                }
            }
            
            // Mark element as processed
            processed_elements.insert(element);
        }
    }
	
    // --- Apply current sources for start and end buses ---
	int num_start_buses = start_buses.size();
    for (auto& [name_bus, bus] : start_buses) {
        int bus_pos = bus_positions[bus];
        int curr_pos = bus_current_positions[bus];

        for (int i = 0; i < pins; ++i) {
            Y(bus_pos + i, curr_pos + i) = -1;
            Y(curr_pos + i, bus_pos + i) = 1;
        }
    }

    //cout << Y << endl;

    MatrixXcd Y_params = MatrixXcd::Zero(num_start_buses*pins, num_start_buses*pins);

    // Iterate through the combinations of start buses to get admittance parameters
    // e.g. start for bus 1 = 1V, bus2-n = 0V, then bus1 = 0V, bus2 = 1V, etc.
    // and estimate Y11, Y12-n, etc.
    int bus_pos = 0;
    for (auto& [name_bus, bus] : start_buses) {
        // int bus_pos = bus_positions[bus];
        // Set voltage source for this bus
        for (int i = 0; i < pins; ++i) {
            z(bus_current_positions[bus] + i, 0) = std::complex<double>(1, 0); // 1V source
        
            // Solve Y * x = z
            Eigen::VectorXcd solution = Y.partialPivLu().solve(z);
		    //cout << "Solution for bus " << name_bus << ":\n" << solution << "\n";

		    // Store results in Y_params
            int idx = 0;
            for (auto& [name_bus_inner, bus_inner] : start_buses) {
			    int bus_pos_inner = bus_positions[bus_inner];
                for (int j = 0; j < pins; j++) {
                    //cout << bus_current_positions[bus_inner] << endl;
				    Y_params(idx + j, bus_pos + i) = solution(bus_current_positions[bus_inner] + j, 0);
			    }	
                idx += pins;
            }
        
            // Reset voltage source for this bus to 0V for next iteration
            z.setZero();
        }
		bus_pos++;
	}
 
   	return Y_params;
}

/**
 * @brief Computes the transfer function for stability analysis of a specific converter.
 * @param converter_name The name of the converter under analysis.
 * @param location The side of the converter from which the analysis is performed (e.g., "AC1" or "DC1").
 * @param frequency The frequency in Hz for the calculation.
 * @return An Eigen::MatrixXcd representing the resulting transfer function matrix.
 */
MatrixXcd StabilityEstimate::compute_transfer_function(string converter_name, string location, double frequency) {
    // Check if converter exists
    if (converters.find(converter_name) == converters.end()) {
        std::cerr << "Error: Converter " << converter_name << " not found.\n";
        return MatrixXcd::Zero(1, 1);
    }
    Element* converter = converters[converter_name];
    Converter* conv = dynamic_cast<Converter*>(converter);
    if (!conv) {
        std::cerr << "Error: Element " << converter_name << " is not a converter.\n";
        return MatrixXcd::Zero(1,1);
    }

    std::string ac_area = conv->getACarea();
    std::string dc_area = conv->getDCarea();

    // Compute equivalent Y parameters first for AC grids and then for DC grids
    // DC grids
    int dc_side_pins = 1;
    unordered_map<string, MatrixXcd> Y_dc_matrices;
    for (auto& [name, sub] : dc_grids) {
        MatrixXcd Y_dc = compute_equivalent_admittance_parameters_num(sub, frequency);
        Y_dc_matrices[name] = Y_dc;
        //std::cout << "Equivalent admittance matrix for DC grid " << name << ":\n" << setprecision(10) << Y_dc << "\n";
    }
    // AC grids
    unordered_map<string, MatrixXcd> Y_ac_matrices;
    for (auto& [name, sub] : ac_grids) {
        MatrixXcd Y_ac = compute_equivalent_admittance_parameters_num(sub, frequency);
        Y_ac_matrices[name] = Y_ac;
        //std::cout << "Equivalent admittance matrix for AC grid " << name << ":\n" << setprecision(10) << Y_ac << "\n";
    }

    // Cross coupling for the admittance of each converter
    // Depending on the side of the converter, the admittance matrix will be different
    // If it is not main converter, then the converter is always considered from DC side.
    // If it is main converter, then if the location is on DC side, then the same applies.
    int converter_number = converters.size();
    MatrixXcd Y_closing = MatrixXcd::Zero((converter_number - 1) * dc_side_pins, (converter_number - 1) * dc_side_pins);
    unordered_map<string, MatrixXcd> Y_conv_matrices;
    int index = 0;
    for (auto& [name, elem] : converters) {
        Converter* conv_elem = dynamic_cast<Converter*>(elem);
        if (!conv_elem) continue;
        // Identify converter terminal buses (AC and DC)
        auto conns_elem = conv_elem->getConnections();
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

		// Get AC and DC areas
		std::string ac_area1 = conv_elem->getACarea();
		std::string dc_area1 = conv_elem->getDCarea();
        
        // Get admittance matrices for the grids
        // Depending on the side of the converter, the admittance matrix looking inside converter will be different
        if ((converter_name != name) || (location == dc_area)) {
            MatrixXcd Y_ac = Y_ac_matrices[ac_area1];
			//cout << "AC grid admittance matrix connected to converter " << name << "AC grid " << ac_area << ":\n" << setprecision(10) << Y_ac << "\n";
			vector<vector<complex<double>>> conv_y_params = conv_elem->compute_y_parameters(frequency);
            MatrixXcd Yconv = vectorToMatrix(conv_elem->compute_y_parameters(frequency));

			//cout << "Converter " << name << " Y-parameters:\n" << Yconv << "\n";

            // The overall transfer function considering the converter's own admittance and the grid admittances.
            MatrixXcd Ydc(1, 1);
            Ydc(0, 0) = Yconv(0, 0);
            MatrixXcd b = Yconv.block(0, 1, 1, 2);
            MatrixXcd a = Yconv.block(1, 0, 2, 1);
            MatrixXcd Ydq = Yconv.block(1, 1, 2, 2);
            Y_conv_matrices[name] = b * (Y_ac - Ydq).inverse() * a + Ydc;

			//cout << "Converter " << name << " admittance matrix from DC side:\n" << Y_conv_matrices[name] << "\n";

            Y_closing.block(index, index, dc_side_pins, dc_side_pins) = Y_conv_matrices[name];
        }
        index++;
    }

	//cout << "Closing impedance matrix Z_closing:\n" << Y_closing << "\n";

    // CROSS-COUPLING OF THE DC SIDE OF THE CONVERTER
    // with the DC grid admittance

	// Identify converter terminal buses (AC and DC)
	auto conns = conv->getConnections();
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

    std::string dc_busname = dc_bus->getBusName();
    MatrixXcd Z_dc = compute_closing_impedance(dc_grids[dc_area], dc_busname, Y_dc_matrices[dc_area], Y_closing);

	//cout << "Equivalent DC admittance looking from converter " << converter_name << ":\n" << Y_dc << "\n";

    // Final transfer function computation
    // For DC cut 
    if (location.rfind("DC", 0) == 0 || location.rfind("dc", 0) == 0) {
        MatrixXcd Y_ac = Y_ac_matrices[ac_area];
        MatrixXcd Yconv = vectorToMatrix(conv->compute_y_parameters(frequency));
        // The overall transfer function considering the converter's own admittance and the grid admittances.
        MatrixXcd Ydc(1, 1);
        Ydc(0, 0) = Yconv(0, 0);
        MatrixXcd b = Yconv.block(0, 1, 1, 2);
        MatrixXcd a = Yconv.block(1, 0, 2, 1);
        MatrixXcd Ydq = Yconv.block(1, 1, 2, 2);
        Y_conv_matrices[converter_name] = b * (Y_ac - Ydq).inverse() * a + Ydc;
		MatrixXcd TF = Y_conv_matrices[converter_name] * Z_dc;

		//cout << "Transfer function matrix for converter " << converter_name << " from DC side:\n" << TF << "\n";

        return TF;
    }
    else {
        // For AC cut - compute the overall admittance looking into the converter from AC side
        // i.e. repeat the same procedure as above but now from AC side
        // For AC cut
        MatrixXcd Yconv = vectorToMatrix(conv->compute_y_parameters(frequency));

        MatrixXcd Ydq = Yconv.block(1, 1, 2, 2);
        MatrixXcd b = Yconv.block(1, 0, 2, 1);      // 2×1
        MatrixXcd a = Yconv.block(0, 1, 1, 2);       // 1×2
        MatrixXcd Ydc = Yconv.block(0, 0, 1, 1);     // 1×1

        // Schur complement: Y_from_ac = Ydq - b × (Ydc + Y_dc_ext)^-1 × a
        // Z_dc is the DC closing impedance (1×1), Y_dc_ext = Z_dc^-1
        MatrixXcd Y_dc_ext = Z_dc.inverse();     // DC grid + other converters
        Y_conv_matrices[converter_name] = Ydq - b * (Ydc + Y_dc_ext).inverse() * a;

        // Transfer function: Y_conv_from_ac × Z_ac
        MatrixXcd TF = Y_conv_matrices[converter_name] * Y_ac_matrices[ac_area].inverse();
        return TF;
    }
}

/**
 * @brief Computes the equivalent closing impedance for a subnetwork port.
 *
 * This function calculates the equivalent impedance seen from a specific output port
 * (`connection_name`) of a subnetwork, considering the loading effects of other
 * connected converters represented by `Z_closing`.
 *
 * @param sub Pointer to the SubNetwork object.
 * @param connection_name The name of the output port for which to compute the impedance.
 * @param Y_parameters The multi-port admittance matrix of the subnetwork.
 * @param Z_closing The closing impedance matrix representing loads at other ports.
 * @return An Eigen::MatrixXcd representing the equivalent closing impedance.
 * @throws std::runtime_error if the connection_name is not found in the subnetwork's outputs.
 */
MatrixXcd StabilityEstimate::compute_closing_impedance(SubNetwork* sub, string& bus_name, MatrixXcd& Y_parameters, MatrixXcd& Y_closing) {
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
        if (name == bus_name) {
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
        return Y_closing;
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
    MatrixXcd Z_eq = (Y11 - Y12 * (Y_closing + Y22).inverse() * Y21).inverse();

    return Z_eq;
}


/**
 * @brief Writes the Y-parameter matrix to a CSV file over a specified frequency range.
 * @param start_frequency The starting frequency for the sweep.
 * @param end_frequency The ending frequency for the sweep.
 * @param number_of_points The number of frequency points to compute and write.
 */
void StabilityEstimate::writeFileTF(string converter_name, string location, double start_frequency, double end_frequency, int number_of_points) {
    std::ofstream myfile;
    myfile.open("./files/" + converter_name + "_" + location + ".csv");

    // Print the Y-parameters in file
    double gap = (end_frequency - start_frequency) * 1.0 / (number_of_points - 1);
    double frequency = start_frequency;
    for (int p = 0; p < number_of_points; p++) {
		MatrixXcd TF = compute_transfer_function(converter_name, location, frequency);

        // write in file
        myfile << frequency << ",";
        
        for (int i = 0; i < TF.rows(); i++) {
            for (int j = 0; j < TF.cols(); ++j) {
                myfile << TF(i, j).real() << "+1i*(" << TF(i, j).imag() << "),";
			}
        }

        myfile << "\n";

        frequency = frequency + gap; // increase frequency
    }

    myfile.close();
}

/**
 * @brief Generates data and triggers a Bode plot for the Y-parameter matrix.
 * @param start_frequency The starting frequency for the plot.
 * @param end_frequency The ending frequency for the plot.
 * @param number_of_points The number of points to plot across the frequency range.
 */
void StabilityEstimate::bodeplotTF(string converter_name, string location, double start_frequency, double end_frequency, int number_of_points) {
    std::vector<double> frequencies;

	// Check if location is on AC or DC side
	bool is_ac = false;
	bool is_dc = false;
	int num_values = 0;
    std::vector<std::string> labels;
	if (location.rfind("AC", 0) == 0 || location.rfind("ac", 0) == 0) {
		is_ac = true;
		num_values = 4; // dq frame
        labels.push_back("TF_{dd}");
		labels.push_back("TF_{dq}");
		labels.push_back("TF_{qd}");
		labels.push_back("TF_{qq}");
	}
	else if (location.rfind("DC", 0) == 0 || location.rfind("dc", 0) == 0) {
		is_dc = true;
		num_values = 1; // DC side
        labels.push_back("TF");
	}
    else {
        std::cerr << "Error: Location " << location << " is neither AC nor DC side.\n";
        return;
	}

    std::vector<std::vector<double>> magnitudes(number_of_points, std::vector<double>(num_values, 0.0));
    std::vector<std::vector<double>> phases(number_of_points, std::vector<double>(num_values, 0.0));
    
    double gap = pow(10.0, (log10(end_frequency) - log10(start_frequency)) / (number_of_points - 1));
    double frequency = start_frequency;
    for (int p = 0; p < number_of_points; p++) {
        frequencies.push_back(frequency);
		MatrixXcd TF = compute_transfer_function(converter_name, location, frequency);

		// Extract magnitude and phase for each element in TF
        for (int i = 0; i < TF.rows(); ++i) {
            for (int j = 0; j < TF.cols(); ++j) {
                double magnitude = 20 * log10(std::abs(TF(i,j)));
                double phase = std::arg(TF(i,j)) * 180.0 / M_PI; // Convert to degrees
                magnitudes[p][TF.cols() * i + j] = magnitude;
                phases[p][TF.cols() * i + j] = phase;
            }
		}

        frequency *= gap; // increase frequency
    }

    bode_plot_implot(frequencies, magnitudes, phases, labels, "TF of power system cut on " + location + " side of " + converter_name);
}

/**
 * @brief Generates data and triggers a Nyquist plot for the Y-parameter matrix.
 * @param start_frequency The starting frequency for the plot.
 * @param end_frequency The ending frequency for the plot.
 * @param number_of_points The number of points to plot across the frequency range.
 */
void StabilityEstimate::nyquistplotTF(string converter_name, string location, double start_frequency, double end_frequency, int number_of_points) {
    // Check if location is on AC or DC side
    bool is_ac = false;
    bool is_dc = false;
    int num_values = 0;
    std::vector<std::string> labels;
    if (location.rfind("AC", 0) == 0 || location.rfind("ac", 0) == 0) {
        is_ac = true;
        num_values = 4; // dq frame
        labels.push_back("TF_{dd}");
        labels.push_back("TF_{dq}");
        labels.push_back("TF_{qd}");
        labels.push_back("TF_{qq}");
    }
    else if (location.rfind("DC", 0) == 0 || location.rfind("dc", 0) == 0) {
        is_dc = true;
        num_values = 1; // DC side
        labels.push_back("TF");
    }
    else {
        std::cerr << "Error: Location " << location << " is neither AC nor DC side.\n";
        return;
    }

    std::vector<vector<complex<double>>> TF(number_of_points, std::vector<complex<double>>(num_values, 0.0));
    double gap = pow(10.0, (log10(end_frequency) - log10(start_frequency)) / (number_of_points - 1));
    double frequency = start_frequency;
    for (int p = 0; p < number_of_points; p++) {
        MatrixXcd TF_freq = compute_transfer_function(converter_name, location, frequency);
        
        // Extract magnitude and phase for each element in TF
        for (int i = 0; i < TF_freq.rows(); ++i) {
            for (int j = 0; j < TF_freq.cols(); ++j) {
				TF[p][TF_freq.cols() * i + j] = TF_freq(i, j);
            }
        }

        frequency *= gap;
    }
    nyquist_plot_implot(TF, labels, "Nyquist Plot of TF on " + location + " side of " + converter_name);
}