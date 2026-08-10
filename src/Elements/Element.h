/**
 * @file Element.h
 * @brief Abstract base class for all electrical network components.
 */

#ifndef ELEMENT_H
#define ELEMENT_H

#include "../Constants.h"
#include "../Solver/Helper_Functions/Helper_Functions.h"

class Bus; // Forward declaration of Bus class

/**
 * @defgroup elements Network elements
 * @brief Component models (R, L, C, sources, converters, etc.) and their stamping interfaces.
 * @{
 */

/**
 * @brief Base class for multi-phase network elements with Y-parameters and MNA stamping hooks.
 *
 * Derived classes implement frequency-domain admittance, symbolic MNA assembly, power-flow
 * contributions, and optional state-space behavior. Bus connections are stored as non-owning
 * terminal-to-bus mappings.
 *
 * @ingroup elements
 */
class Element {
public:
    /**
     * @brief Construct an element with symbol, location, and pin counts.
     *
     * Initializes the internal Y-parameter matrix to zeros with dimensions
     * (2 * inputPins) x (2 * outputPins).
     *
     * @param symbol Component type symbol (e.g. "R", "L", "C").
     * @param location Grid or partition label (e.g. "AC1", "DC2", "PEC1").
     * @param inputPins Number of input pins/phases.
     * @param outputPins Number of output pins/phases.
     */
    Element(const std::string& symbol, const std::string& location, int inputPins, int outputPins)
        : element_symbol(symbol), element_location(location), input_pins(inputPins), output_pins(outputPins) {
        Y_matrix = createZeroMatrix(2*inputPins, 2*outputPins);
    }

    /**
     * @brief Virtual destructor for derived-class cleanup.
     */
    virtual ~Element();

    /**
     * @brief Get the number of input pins/phases.
     * @return Input pin count.
     */
    int getInputPins() const { return input_pins; }

    /**
     * @brief Get the number of output pins/phases.
     * @return Output pin count.
     */
    int getOutputPins() const { return output_pins; }

    /**
     * @brief Set the number of input pins/phases.
     * @param pins New input pin count.
     */
    void setInputPins(int pins) { input_pins = pins; }

    /**
     * @brief Set the number of output pins/phases.
     * @param pins New output pin count.
     */
    void setOutputPins(int pins) { output_pins = pins; }

    /**
     * @brief Enable or disable coordinate transformation for this element.
     * @param flag True to apply transformation (e.g. three-phase to dq).
     */
	void setTransformation(bool flag) { transformation = flag; }

    /**
     * @brief Get the element type symbol.
     * @return Symbol string (e.g. "R", "L", "C").
     */
    std::string getElementSymbol() const { return element_symbol; }

    /**
     * @brief Collect all buses connected to this element.
     * @return Vector of non-owning bus pointers from the connections map.
     */
    std::vector<Bus*> getBuses();

    /**
     * @brief Find the terminal index associated with a given bus.
     * @param bus Bus pointer to look up.
     * @return Terminal index if connected; -1 if not found.
     */
    int getBusIndex(Bus* bus) {
        auto it = connections.find(bus);
        if (it != connections.end()) {
            return it->second; // Return the index of the bus in the connections map
        }
        return -1; // Return -1 if the bus is not found
	}

    /**
     * @brief Get the bus connected at the opposite terminal from a given bus.
     * @param bus One connected bus.
     * @return The other connected bus, or nullptr if not applicable.
     */
    Bus* getOtherBus(Bus*);

    /**
     * @brief Const access to the bus-to-terminal connection map.
     * @return Map from bus pointer to terminal index.
     */
    const std::map<Bus*, int>& getConnections() const { return connections; }

    /**
     * @brief Copy of the bus-to-terminal connection map.
     * @return Map from bus pointer to terminal index.
     */
    std::map<Bus*, int> getConnections() { return connections; }

    /**
     * @brief Get the element's grid or partition location label.
     * @return Location string.
     */
	string getElementLocation() const { return element_location; }
    

    /**
     * @brief Attach a bus to a specific element terminal.
     * @param bus Non-owning bus pointer.
     * @param terminal Terminal index (0-based).
     */
    void attachBus(Bus*, int);

    /**
     * @brief Print basic element metadata (symbol, input/output pin counts).
     */
    void printElementInfo() const {
        std::cout << "Element Symbol: " << element_symbol
            << ", Input Pins: " << input_pins
            << ", Output Pins: " << output_pins << std::endl;
    }

    /**
     * @brief Print extended element-specific values (override in derived classes).
     */
    virtual void printElementValues();

    /**
     * @brief Write Y-parameter matrix samples over a frequency sweep to a file.
     * @param start_frequency Lower bound of frequency range (Hz).
     * @param end_frequency Upper bound of frequency range (Hz).
     * @param number_of_points Number of frequency samples.
     */
    void writeFile(double start_frequency, double end_frequency, int number_of_points);

    /**
     * @brief Plot Y-parameter matrix entries versus frequency.
     * @param start_frequency Lower bound of frequency range (Hz).
     * @param end_frequency Upper bound of frequency range (Hz).
     * @param number_of_points Number of frequency samples.
     */
    void plotYParameters(double start_frequency, double end_frequency, int number_of_points);

    /**
     * @brief Plot eigenvalues of the element model (override in state-space elements).
     */
    virtual void plotEigenvalues() { 
		cout << "Eigenvalue plotting is not available for the linear element." << endl;
    };

    /**
     * @brief Plot participation factors (override in state-space elements).
     */
    virtual void plotParticipationFactors() {
        cout << "Participation factors plotting is not available for the linear element." << endl;
    };

    /**
     * @brief Compute complex Y-parameters at a single frequency.
     *
     * Default implementation returns an empty matrix; derived classes provide the
     * frequency-dependent admittance.
     *
     * @param frequency Evaluation frequency in Hz.
     * @return Complex Y-parameter matrix as nested vectors.
     */
    virtual std::vector<std::vector<complex<double>>> compute_y_parameters(double frequency);

    /**
     * @brief Get the symbolic or stored Y-parameter matrix.
     * @return SymEngine dense matrix representing Y-parameters.
     */
    virtual DenseMatrix get_y_parameters() { return Y_matrix; }; 

    /**
     * @brief Apply a coordinate or coupling transformation to Y-parameter blocks.
     * @param Y1 First Y-parameter block.
     * @param Y2 Second Y-parameter block.
     * @return Transformed Y-parameter matrix.
     */
    virtual std::vector<std::vector<complex<double>>> apply_transformation(std::vector<std::vector<complex<double>>>& Y1, std::vector<std::vector<complex<double>>>& Y2);

    /**
     * @brief Stamp this element into the symbolic MNA matrix (override in derived classes).
     * @param mna Global MNA matrix being assembled.
     * @param busIndexMap Maps buses to global node indices.
     * @param offset Current matrix offset for stamping.
     * @param elementSymbols Per-element symbolic substitution table.
     */
    virtual void writeMNAmatrix(SymEngine::DenseMatrix&, std::unordered_map<Bus*, int>&, int, std::map<Element*, std::vector<RCP<const Basic>>>&) {};

    /**
     * @brief Number of internal state variables for dynamic elements.
     * @return State count; zero for static linear elements.
     */
    virtual int getNumberOfInternalStates() const { return 0; }

    /**
     * @brief Number of plant states exposed for simulation (may differ from internal states).
     * @return Plant state count; defaults to getNumberOfInternalStates().
     */
    virtual int getNumberOfPlantStates() const {
        return getNumberOfInternalStates();   // default: full state count
    }

    /**
     * @brief Simulate a step response given input states (override in dynamic elements).
     * @param states Input state trajectories per channel.
     * @param nKeep Number of states to retain in the output.
     * @return Simulated state matrices; empty for static elements.
     */
    virtual std::vector<MatrixXcd> simulateInputStep(
        const std::vector<MatrixXcd>& states, int nKeep) const {
        return {};
    }

    /**
     * @brief Return symbolic names for virtual inputs used in stamping.
     * @return List of SymEngine symbol RCPs; empty if none.
     */
    virtual std::vector<RCP<const Basic>> getVirtualInputSymbols() const { return {}; }

    /**
     * @brief Return parameter substitutions for symbolic evaluation.
     * @return Map from symbols to replacement expressions.
     */
    virtual map_basic_basic getParameterSubstitutions() const {return {}; }

    /**
     * @brief Compute branch power-flow quantities (override in derived classes).
     * @param branchData Output map of branch OPF results.
     * @param globalParams Shared global OPF parameters.
     */
    virtual void computePowerFlow(std::map<std::string, double>& branchData,
		std::map<std::string, double>& globalParams) const {
	}

    /**
     * @brief Get stored OPF-related metadata for this element.
     * @return Map of OPF parameter names to values.
     */
    std::map<std::string, double> getOPFInfo() const {
        return element_OPF_info;
    }

    /**
     * @brief Store OPF-related metadata for this element.
     * @param info Map of OPF parameter names to values.
     */
    void setOPFInfo(std::map<std::string, double>& info) { element_OPF_info = info; }

protected:
	bool transformation = false; // Flag to indicate if a transformation is applied (e.g. three-phase to dq-frame)

    std::string element_symbol; // Element symbol (e.g., R, L, C)
	std::string element_location; // Element location (it can be AC1,2,... or DC1,2,... or PEC1,2,...)
    int input_pins; // Number of input pins/phases
    int output_pins;  // Number of output pins/phases
    std::map<Bus* , int> connections; // Map of bus connections to the element's terminal
    std::map<std::string, double> element_OPF_info = {};

    DenseMatrix Y_matrix; // Y-parameter matrix representing the admittance of the element
};

/** @} */ // end of elements group

#endif // ELEMENT_H
