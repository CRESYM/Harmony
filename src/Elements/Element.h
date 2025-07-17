#ifndef ELEMENT_H
#define ELEMENT_H

#include "../Constants.h"
#include "../Solver/Helper_Functions/Helper_Functions.h"

class Bus; // Forward declaration of Bus class

class Element {
public:
     // Unified constructor for universal number of phases
    Element(const std::string& symbol, int inputPins, int outputPins)
        : element_symbol(symbol), input_pins(inputPins), output_pins(outputPins) {
        Y_matrix = createZeroMatrix(2*inputPins, 2*outputPins);
    }

    Element(const std::string& name, int phases) {};

    // Virtual destructor for resource cleanup in derived classes
    virtual ~Element();

    // Getters for the number of input/output pins, element symbol, and connections
    int getInputPins() const { return input_pins; }
    int getOutputPins() const { return output_pins; }
    std::string getElementSymbol() const { return element_symbol; }
    std::vector<Bus*> getBuses();
    Bus* getOtherBus(Bus*);
    std::map<Bus*, int> getConnections() { return connections; }

    // Attach bus pointer to the proper terminal
    void attachBus(Bus*, int);

    // Print element information
    void printElementInfo() const {
        std::cout << "Element Symbol: " << element_symbol
            << ", Input Pins: " << input_pins
            << ", Output Pins: " << output_pins << std::endl;
    }

    // Virtual function to print additional element values, can be overridden by derived classes (addition to printElementInfo)
    virtual void printElementValues();

    // Function to write the Y-parameter matrix to a file over a frequency range 
    void writeFile(double start_frequency, int end_frequency, int number_of_points);

	// Function to plot the Y-parameter matrix entries
    void plotYParameters(double start_frequency, int end_frequency, int number_of_points);

    // Virtual function to compute Y-parameters (to be implemented by derived classes)
    virtual std::vector<std::vector<complex<double>>> compute_y_parameters(double frequency);
    virtual DenseMatrix compute_y_parameters() { return Y_matrix; };
    virtual Eigen::MatrixXcd compute_y_parameters_num(double omega_num) { return substitute_symbol(Y_matrix, omega, omega_num); };
        
    // Function to retrieve the Y-parameter matrix
    DenseMatrix read_y_matrix() { return Y_matrix; }

    // Generic MNA stamping 
    virtual void writeMNAmatrix(SymEngine::DenseMatrix&, std::unordered_map<Bus*, int>&, int, std::map<Element*, std::vector<RCP<const Basic>>>&) {};
    
    virtual bool getPhaseState(int i) const { return true; } // Default to closed


    // Virtual power flow computation methods (override in subclasses)
    virtual void computePowerFlowAC(std::map<std::string, std::map<std::string, double>>& branchData,
        std::map<std::string, double>& globalParams) const {}

    virtual void computePowerFlowDC(std::map<std::string, std::map<std::string, double>>& branchDCData,
        std::map<std::string, double>& globalParams) const {}

protected:
    std::string element_symbol; // Element symbol (e.g., R, L, C)
    int input_pins; // Number of input pins/phases
    int output_pins;  // Number of output pins/phases
    std::map<Bus* , int> connections; // Map of bus connections to the element's terminal

    DenseMatrix Y_matrix; // Y-parameter matrix representing the admittance of the element
};

#endif // ELEMENT_H
