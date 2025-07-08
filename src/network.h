#ifndef NETWORK_H
#define NETWORK_H

#include <unordered_map>
#include <vector>
#include <tuple>
#include <string>
#include <stdexcept>
#include <iostream>
#include <map>
#include "State_Space_Model.h"

class Bus;
class Element;

// Type alias for a pair of element designator and pin name, used to map buses to connected elements in the network
using Net = std::unordered_map<Bus*, std::vector<Element*>>;

class Network {
private:
    //StateSpaceModel state_space_model; 
    std::unordered_map<std::string, Bus*> buses; // Map of bus names to buses
    std::unordered_map<std::string, Element*> elements;  // Map of designators to elements
    std::unordered_map<Bus*, std::vector<Element*>> connections; // Connections between buses and elements

    int pins; // Total number of pins/phases in the network, used for equivalent admittance/impedance calculation
    //state_space_model
    int number_voltage_sources;
    int number_current_sources;
    int number_switches;
    int number_inductors;
    int number_capacitors;
    int number_resistors;
    int number_independent_sources;
    int number_elements;
    int number_nodes;
    int total_number_equations;
    int state_variables;
    std::vector<Element*> element_list;
    std::vector<std::string> symbols_bank;
    std::vector<Bus*> nodes;
    std::vector<Element*> switches;
    std::vector<Element*> inductors;
    std::vector<Element*> capacitors;
    std::vector<RCP<const Symbol>> initial_values;
    std::vector<RCP<const Symbol>> state_variables_symbols;
    std::vector<RCP<const Symbol>> sources_symbols;
    std::vector<RCP<const Symbol>> independent_sources;
    std::vector<RCP<const Symbol>> sources_values;

    //int pins; // Total number of pins/phases in the network, used for equivalent admittance/impedance calculation 

        //store Dictionary OPF
    std::map<std::string, std::map<std::string, std::map<std::string, double>>> data;

    std::unordered_map<std::string, int> busName2Id_;


public:

    // Constructor to initialize the network with zero pins
    Network() :pins(0) {}

    // Destructor to handle resource cleanup
    ~Network();

    // Function to connect two elements in the network
    //void connect(const std::string& source, const std::string& destination);

    // Overloaded function to connect multiple elements to a single destination
    //void connect(const std::vector<std::string>& sources, const std::string& destination);

    // Function to add a bus to the network using a bus object
    void addBus(Bus* bus);

    // Function to add a bus to the network by providing a bus name and bus object
    void addBus(const std::string& busName, Bus* bus);

    // Function to add an element (with an Element object) and get a unique designator
    void addElement(Element* elem);

    // Function to add an element to the network using a specific designator
    void addElement(const std::string& designator, Element* elem);

    // Function to connect an element to a specific bus at a particular terminal
    void connectElementToBus(Element* elem, int terminal, Bus* bus);

    // Function to delete an element from the network based on its designator
    void deleteElement(const std::string& designator);

    // Function to delete a bus from the network based on its name
    void deleteBus(const std::string& busName);

    // Print all the current connections between buses and elements in the network
    void printConnections();

    // Getters for private members variables for buses, elements, and connections
    const std::unordered_map<std::string, Bus*>& getBuses() const { return buses; }
    const std::unordered_map<std::string, Element*>& getElements() const { return elements; }
    const std::unordered_map<Bus*, std::vector<Element*>>& getConnections() const { return connections; }

    // Function to compute the equivalent impedance between buses, excluding certain elements  //network_stability.cpp
    void compute_equivalent_impedance(std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements);

    void compute_equivalent_impedance_nums(std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements, double omega_num);

    // System analysis
    void checkStability(const Eigen::MatrixXd& A_matrix);

    //Power flow computation //network_powerflow.cpp
    std::map<std::string, double> PowerFlow();

    // Power flow construction helpers
   /* void addBusAC(std::vector<std::vector<std::string>>& dict_ac,
        const std::vector<std::string>& bus_info);

    void addBusDC(std::vector<std::vector<std::string>>& dict_dc,
        const std::vector<std::string>& bus_info);

    void make_BranchAC(Element* element,
        std::map<std::string, std::map<std::string, std::map<std::string, double>>>& data,
        std::map<std::string, double>& global_params);

    void make_BranchDC(Element* element,
        std::map<std::string, std::map<std::string, std::map<std::string, double>>>& data,
        std::map<std::string, double>& global_params);

    void make_Generator(Element* element,
        std::map<std::string, std::map<std::string, std::map<std::string, double>>>& data);

    void make_Converter(Element* element,
        std::vector<std::vector<std::string>>& dict_dc,
        std::vector<std::vector<std::string>>& dict_ac,
        std::vector<std::string> new_i,
        std::vector<std::string> new_o,
        std::map<std::string, std::map<std::string, std::map<std::string, double>>>& data,
        std::map<std::string, double>& global_params);*/

    void addBusAC(std::vector<std::vector<std::string>>& dict_ac,
        const std::vector<std::string>& bus_info,
        bool print_info = false);

    void addBusDC(std::vector<std::vector<std::string>>& dict_dc,
        const std::vector<std::string>& bus_info,
        bool print_info = false);

    void make_BranchAC(Element* element,
        std::map<std::string, double>& global_params,
        const std::vector<std::string>& br_info,
        bool print_info = false);

    void make_BranchDC(Element* element,
        std::map<std::string, double>& global_params,
        const std::vector<std::string>& br_info,
        bool print_info = false);

    void make_Generator(Element* element,
        const std::vector<std::string>& gen_info,
        bool print_info = false);

    void make_Load(Element* element,
        const std::vector<std::string>& load_info,
        bool print_info = false);

    //void make_Converter(Element* element,
    //    std::vector<std::vector<std::string>>& dict_dc,
    //    std::vector<std::vector<std::string>>& dict_ac,
    //    std::vector<std::string> new_i,
    //    std::vector<std::string> new_o,
    //    std::map<std::string, std::map<std::string, std::map<std::string, double>>>& data,
    //    std::map<std::string, double>& global_params);

    void make_Converter(Element* element,
        std::map<std::string, double>& global_params,
        const std::vector<std::string>& conv_info,
        bool print_info = false);

    void make_OPF(const Network& net,
        bool vscControl = true,
        bool writeTxt = false,
        bool plotResult = false);

    const auto& getNetData() const { return data; }
    auto& getNetData() { return data; }


    //getters for state_space_model
    int getNumberStateVariables() const; //number of state variables
    int getStateVariablePosition() const; //MNA matrix column positions
    const std::vector<SymEngine::RCP<const SymEngine::Symbol>>& getStateVariableSymbols() const; //return symengine symbols 
    int getNumberIndependentSource() const; //independent sources
    const std::vector<SymEngine::RCP<const SymEngine::Symbol>>& getSourceSymbols() const;

    int  getNumberOutputs() const;     // y dimension
    const std::vector<int>& getOutputIndexes() const;
    int  getNumberEquations() const;     // total MNA rows    
};

#endif // NETWORK_H
