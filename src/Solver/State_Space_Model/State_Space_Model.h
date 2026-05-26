#ifndef STATE_SPACE_MODEL
#define STATE_SPACE_MODEL

#include "../../Elements/Element.h"
#include "../../Bus.h"
#include "../../network.h"

class Network; class Element; class Bus;

enum class SSMMode {
    Standard,   ///< Raw pin-count columns in B (for eigenvalue/impedance analysis)
    DQsym       ///< All B columns grouped in 3s (for DSSS phasor domain)
};

struct InputMapping {
    std::string elementName;
    int pinIndex;            ///< Pin within element (0-based)
    int bColumnIndex;        ///< Column in B (or B_dqsym)
    bool isVirtualInput;
    int groupIndex;          ///< Which 3-column group (DQsym mode)
    int phaseInGroup;        ///< 0,1,2 within the group (DQsym mode)
};

struct StateMapping {
    std::string elementName;
    int stateIndex;
    int aRowIndex;
};

struct OutputMapping {
    std::string busName;
    int pinIndex;
    int cRowIndex;
};


/// add22/5 : moved from private :: Internal info: for each source/virtual provider, how many raw B columns it has
struct InputGroup {
    Element* element;
    int rawCols;          ///< Actual pin count (2 for DC, 3 for AC, 12 for MMC virtual)
    int dqsymCols;        ///< After expansion: ceil to next multiple of 3
    int rawStartCol;      ///< Start column in raw B
    int dqsymStartCol;    ///< Start column in B_dqsym
    bool isVirtual;
};


class StateSpaceModel {
public:
    class Tree {
    private:
        Bus* root; Element* element; Bus* current_node;
        std::vector<Element*> subelements; std::shared_ptr<Tree> parent;
    public:
        Tree(Element* e, Bus* cur, Bus* r, std::shared_ptr<Tree> p = nullptr)
            : element(e), current_node(cur), root(r), parent(p) {
        }
        void addPathElement(Element* e) { subelements.push_back(e); }
        Bus* getRoot() const { return root; }
        Element* getElement() const { return element; }
        Bus* getCurrentNode() const { return current_node; }
        const std::vector<Element*>& getSubelements() const { return subelements; }
        std::shared_ptr<Tree> getParent() const { return parent; }
        bool containsNode(Bus* node) const;
        std::vector<Element*> getPath() const;
        std::string toString() const;
    };

    StateSpaceModel() = default;

    static std::vector<std::vector<Bus*>> form_cutset_nodes(const std::vector<Bus*>&,
        const std::vector<std::pair<Bus*, Bus*>>&);
    static std::vector<std::vector<Element*>> form_cutsets(
        const std::vector<std::vector<Bus*>>&,
        const std::map<Bus*, std::vector<Element*>>&);
    static std::map<Bus*, std::vector<Element*>> generateBusToElementsMap(
        const std::vector<Element*>&);
    static std::vector<std::vector<Element*>> findLoops(
        const std::vector<Bus*>&, const std::map<Bus*, std::vector<Element*>>&);

    void formState(Network* net, const std::vector<Bus*>& out,
        SSMMode mode = SSMMode::Standard);

    /// Raw matrices (Standard mode)
    const Eigen::MatrixXd& getA() const { return A; }
    const Eigen::MatrixXd& getB() const { return mode_ == SSMMode::DQsym ? B_dqsym : B; }
    const Eigen::MatrixXd& getC() const { return C; }
    const Eigen::MatrixXd& getD() const { return mode_ == SSMMode::DQsym ? D_dqsym : D; }

    /// Raw B (always available regardless of mode)
    const Eigen::MatrixXd& getBRaw() const { return B; }

    SSMMode getMode() const { return mode_; }

    const std::vector<InputMapping>& getInputMap() const { return input_map; }
    const std::vector<StateMapping>& getStateMap() const { return state_map; }
    const std::vector<OutputMapping>& getOutputMap() const { return output_map; }
    const std::vector<InputGroup>& getInputGroups() const { return input_groups; }

    int getInputIndex(const std::string& elementName, int pin = 0) const {
        for (const auto& m : input_map)
            if (m.elementName == elementName && m.pinIndex == pin)
                return m.bColumnIndex;
        return -1;
    }

    int getStateIndex(const std::string& elementName, int stateIdx = 0) const {
        for (const auto& m : state_map)
            if (m.elementName == elementName && m.stateIndex == stateIdx)
                return m.aRowIndex;
        return -1;
    }

    /// Build u (nu × nKeep) from all element contributions.
    /// In DQsym mode: u rows are grouped in 3s, DC values broadcast to all 3 phases.
    MatrixXcd buildInputVector(int nKeep,
        const std::map<std::string, std::vector<MatrixXcd>>& elementStates) const;

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
