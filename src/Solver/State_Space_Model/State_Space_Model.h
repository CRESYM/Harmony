#ifndef STATE_SPACE_MODEL
#define STATE_SPACE_MODEL

/**
 * @file State_Space_Model.h
 * @brief State-space model assembly from network MNA formulation.
 * @ingroup solvers
 *
 * Builds continuous-time state-space matrices (A, B, C, D) from a
 * Network object, with optional DQsym column grouping for phasor-domain
 * simulation. Provides index mappings between element pins/states and
 * matrix rows/columns.
 */

#include "../../Elements/Element.h"
#include "../../Bus.h"
#include "../../network.h"

class Network; class Element; class Bus;

/**
 * @brief Matrix layout mode for input/output column grouping.
 */
enum class SSMMode {
    Standard,   ///< Raw pin-count columns in B (for eigenvalue/impedance analysis)
    DQsym       ///< All B columns grouped in 3s (for DSSS phasor domain)
};

/**
 * @brief Maps an element input pin to a column in the B matrix.
 */
struct InputMapping {
    std::string elementName;
    int pinIndex;            ///< Pin within element (0-based)
    int bColumnIndex;        ///< Column in B (or B_dqsym)
    bool isVirtualInput;
    int groupIndex;          ///< Which 3-column group (DQsym mode)
    int phaseInGroup;        ///< 0,1,2 within the group (DQsym mode)
};

/**
 * @brief Maps an element state variable to a row in the A matrix.
 */
struct StateMapping {
    std::string elementName;
    int stateIndex;
    int aRowIndex;
};

/**
 * @brief Maps a bus output pin to a row in the C matrix.
 */
struct OutputMapping {
    std::string busName;
    int pinIndex;
    int cRowIndex;
};

/**
 * @brief Per-source metadata for raw and DQsym B-column layout.
 *
 * Tracks how many raw and expanded columns each independent source or
 * virtual-input provider contributes to the input matrix.
 */
struct InputGroup {
    Element* element;
    int rawCols;          ///< Actual pin count (2 for DC, 3 for AC, 12 for MMC virtual)
    int dqsymCols;        ///< After expansion: ceil to next multiple of 3
    int rawStartCol;      ///< Start column in raw B
    int dqsymStartCol;    ///< Start column in B_dqsym
    bool isVirtual;
};

/**
 * @class StateSpaceModel
 * @brief Assembles and stores the linear state-space representation of a network.
 *
 * Starting from the modified nodal admittance formulation, this class
 * identifies state variables (inductor currents, capacitor voltages, converter
 * plant states), independent sources, switches, and output buses, then
 * constructs the A, B, C, and D matrices together with human-readable
 * index mappings.
 */
class StateSpaceModel {
public:
    /**
     * @class Tree
     * @brief Spanning-tree node used during loop/cutset detection.
     *
     * Each node stores the element that connects to the current bus, the path
     * of elements traversed so far, and a link back to the parent node in the
     * search tree.
     */
    class Tree {
    private:
        Bus* root; Element* element; Bus* current_node;
        std::vector<Element*> subelements; std::shared_ptr<Tree> parent;
    public:
        /**
         * @brief Constructs a tree node for graph traversal.
         * @param e Element associated with this node.
         * @param cur Current bus being visited.
         * @param r Root bus of the spanning tree.
         * @param p Parent tree node (nullptr for the root).
         */
        Tree(Element* e, Bus* cur, Bus* r, std::shared_ptr<Tree> p = nullptr)
            : element(e), current_node(cur), root(r), parent(p) {
        }

        /** @brief Appends an element to the path from root to this node. */
        void addPathElement(Element* e) { subelements.push_back(e); }

        /** @return Root bus of the spanning tree. */
        Bus* getRoot() const { return root; }

        /** @return Element connected at this tree node. */
        Element* getElement() const { return element; }

        /** @return Bus currently being visited. */
        Bus* getCurrentNode() const { return current_node; }

        /** @return Elements on the path from root to this node (excluding self). */
        const std::vector<Element*>& getSubelements() const { return subelements; }

        /** @return Parent tree node, or nullptr at the root. */
        std::shared_ptr<Tree> getParent() const { return parent; }

        /**
         * @brief Checks whether a bus appears anywhere in this subtree.
         * @param node Bus to search for.
         * @return True if @p node is the current node or a descendant.
         */
        bool containsNode(Bus* node) const;

        /**
         * @brief Returns the full element path from root to this node.
         * @return Ordered list of elements along the path.
         */
        std::vector<Element*> getPath() const;

        /**
         * @brief Returns a human-readable description of this tree node.
         * @return String representation of the node and its path.
         */
        std::string toString() const;
    };

    /** @brief Default constructor; matrices are populated by formState(). */
    StateSpaceModel() = default;

    /**
     * @brief Partitions buses into cutset node groups.
     * @param buses All network buses.
     * @param treeEdges Spanning-tree edges as (parent, child) bus pairs.
     * @return Groups of buses forming cutsets.
     */
    static std::vector<std::vector<Bus*>> form_cutset_nodes(const std::vector<Bus*>&,
        const std::vector<std::pair<Bus*, Bus*>>&);

    /**
     * @brief Maps cutset node groups to the elements that form each cutset.
     * @param cutsetNodes Bus groups produced by form_cutset_nodes().
     * @param busToElements Adjacency map from bus to incident elements.
     * @return Element lists, one per cutset.
     */
    static std::vector<std::vector<Element*>> form_cutsets(
        const std::vector<std::vector<Bus*>>&,
        const std::map<Bus*, std::vector<Element*>>&);

    /**
     * @brief Builds a bus-to-elements adjacency map.
     * @param elements All network elements.
     * @return Map from each bus to the elements connected to it.
     */
    static std::map<Bus*, std::vector<Element*>> generateBusToElementsMap(
        const std::vector<Element*>&);

    /**
     * @brief Finds fundamental loops in the network graph.
     * @param buses All network buses.
     * @param busToElements Adjacency map from bus to incident elements.
     * @return Element lists, one per loop.
     */
    static std::vector<std::vector<Element*>> findLoops(
        const std::vector<Bus*>&, const std::map<Bus*, std::vector<Element*>>&);

    /**
     * @brief Assembles state-space matrices from a network.
     * @param net Network to linearize.
     * @param out Output buses whose voltages form the C/D outputs.
     * @param mode Column layout mode (Standard or DQsym).
     */
    void formState(Network* net, const std::vector<Bus*>& out,
        SSMMode mode = SSMMode::Standard);

    /** @return State matrix A (n_states × n_states). */
    const Eigen::MatrixXd& getA() const { return A; }

    /** @return Input matrix B or B_dqsym depending on current mode. */
    const Eigen::MatrixXd& getB() const { return mode_ == SSMMode::DQsym ? B_dqsym : B; }

    /** @return Output matrix C (n_outputs × n_states). */
    const Eigen::MatrixXd& getC() const { return C; }

    /** @return Feed-through matrix D or D_dqsym depending on current mode. */
    const Eigen::MatrixXd& getD() const { return mode_ == SSMMode::DQsym ? D_dqsym : D; }

    /** @return Raw input matrix B before DQsym column expansion. */
    const Eigen::MatrixXd& getBRaw() const { return B; }

    /** @return Active matrix layout mode. */
    SSMMode getMode() const { return mode_; }

    /** @return Input pin to B-column mappings. */
    const std::vector<InputMapping>& getInputMap() const { return input_map; }

    /** @return State variable to A-row mappings. */
    const std::vector<StateMapping>& getStateMap() const { return state_map; }

    /** @return Output pin to C-row mappings. */
    const std::vector<OutputMapping>& getOutputMap() const { return output_map; }

    /** @return Per-source column group metadata. */
    const std::vector<InputGroup>& getInputGroups() const { return input_groups; }

    /**
     * @brief Looks up the B-matrix column index for an element input pin.
     * @param elementName Element symbol/name.
     * @param pin Pin index within the element (default 0).
     * @return Column index in B, or -1 if not found.
     */
    int getInputIndex(const std::string& elementName, int pin = 0) const {
        for (const auto& m : input_map)
            if (m.elementName == elementName && m.pinIndex == pin)
                return m.bColumnIndex;
        return -1;
    }

    /**
     * @brief Looks up the A-matrix row index for an element state.
     * @param elementName Element symbol/name.
     * @param stateIdx State index within the element (default 0).
     * @return Row index in A, or -1 if not found.
     */
    int getStateIndex(const std::string& elementName, int stateIdx = 0) const {
        for (const auto& m : state_map)
            if (m.elementName == elementName && m.stateIndex == stateIdx)
                return m.aRowIndex;
        return -1;
    }

    /**
     * @brief Builds the input vector u from per-element state contributions.
     *
     * In DQsym mode, rows are grouped in triplets and DC values are broadcast
     * to all three phase rows.
     *
     * @param nKeep Number of harmonic columns to retain per input group.
     * @param elementStates Map from element name to its input coefficient matrices.
     * @return Complex input matrix (nu × nKeep).
     */
    MatrixXcd buildInputVector(int nKeep,
        const std::map<std::string, std::vector<MatrixXcd>>& elementStates) const;

    /** @brief Prints input, state, and output index mappings to stdout. */
    void printMapping() const;

private:
    static void traverseForLoops(Bus*, Bus*, const std::vector<Bus*>&,
        std::vector<std::vector<Element*>>&,
        const std::map<Bus*, std::vector<Element*>>&,
        std::shared_ptr<Tree>, const std::map<Bus*, int>&);

    Eigen::MatrixXd A, B, C, D;
    Eigen::MatrixXd B_dqsym, D_dqsym;   ///< Expanded B/D with all columns in groups of 3

    SSMMode mode_ = SSMMode::Standard;

    int number_switches = 0, number_independent_sources = 0;
    int number_virtual_inputs = 0;
    int number_nodes = 0, total_number_equations = 0;
    int number_outputs = 0, number_state_variables = 0;

    std::vector<Bus*> output;
    std::unordered_map<Bus*, int> bus_indices;
    std::unordered_map<Element*, int> element_indices;
    std::map<Element*, std::vector<RCP<const Basic>>> symbols_bank;
    std::map<Element*, std::vector<RCP<const Basic>>> virtual_input_bank;
    std::vector<Element*> list_state_variables;
    std::vector<Element*> list_independent_sources;
    std::vector<Element*> list_virtual_input_providers;
    std::vector<Element*> list_switches;

    std::vector<InputMapping>  input_map;
    std::vector<StateMapping>  state_map;
    std::vector<OutputMapping> output_map;

   
    std::vector<InputGroup> input_groups;


    void finalizeCounts(Network*);
    void substituteParameters(SymEngine::DenseMatrix& matrix);
    void buildMappings();
    void expandBForDQsym();   ///< Post-process: raw B → B_dqsym (all groups of 3)
};

#endif
