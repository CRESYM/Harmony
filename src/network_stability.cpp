#include "network.h"      // For access to the Network class and its members
#include "Element.h"      // For Element* operations (e.g., compute_y_parameters, getConnections)
#include "Bus.h"          // For Bus* methods like getBusName, getPinNumber, etc.

#include <algorithm>      // For std::sort, std::find, std::unique
#include <stdexcept>      // For std::invalid_argument
#include <iostream>       // For debug prints (if kept)

void Network::compute_equivalent_impedance(std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements) {
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
    pos = 0; // position in positions_currents
    for (auto& bus : all_buses) {
        for (auto& element : connections[bus.first]) {
            // Skip elements that are in the skip_elements list
            if (std::find(skip_elements.begin(), skip_elements.end(), element) == skip_elements.end()) {
                // Get the Y-parameter matrix of the element
                DenseMatrix element_Y_matrix = element->compute_y_parameters();

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

void Network::compute_equivalent_impedance_nums(std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements, double omega_num)
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


    // Populate the admittance matrix
    pos = 0;
    for (auto& bus : all_buses) {
        for (auto& element : connections[bus.first]) {
            // Skip elements that are in the skip_elements list
            if (std::find(skip_elements.begin(), skip_elements.end(), element) == skip_elements.end()) {
                // Get the Y-parameter matrix of the element
                Eigen::MatrixXcd element_Y_matrix = element->compute_y_parameters_num(omega_num);  // Use numerical Y

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
                        Y(position + i, position + j) += element_Y_matrix(terminal * pins + i, terminal * pins + j);
                    }
                    for (int j = 0; j < pins_other; j++) {
                        if (other_bus->getBusName() != "gnd") {
                            Y(position + i, position_other + j) += element_Y_matrix(terminal * pins + i, terminal_other * pins_other + j);
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
    //std::cout << "Admittance matrix:\n" << Y << std::endl;

    // Extract the reduced admittance matrix (block)
    Eigen::MatrixXcd reduced_Y = Y.block(0, 0, pos, pos);  // Extract the block
    Eigen::VectorXcd b = Y.block(0, pos, pos, 1);          // Extract the RHS vector


    // Solve the linear system Y * x = b
    Eigen::VectorXcd solution = reduced_Y.partialPivLu().solve(b);

    //// Debugging output
    //std::cout << "Solution to the system:\n" << solution << std::endl;

    // Compute the equivalent impedance (numerical)
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
    //cout << equivalent_impedance_size << endl;
}


