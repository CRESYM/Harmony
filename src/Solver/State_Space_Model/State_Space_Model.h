#ifndef STATE_SPACE_MODEL
#define STATE_SPACE_MODEL

#include "../../Elements/Element.h"  
#include "../../Bus.h"      
#include "../../network.h"

class Network; // Forward declaration of Network class
class Element; // Forward declaration of Element class
class Bus; // Forward declaration of Bus class


class StateSpaceModel {
public:
    // Nested Tree class for loop formation
    class Tree {
    private:
        Bus* root;                          // Starting node of the tree/loop
        Element* element;                   // Connecting element
        Bus* current_node;                  // Current node in traversal
        std::vector<Element*> subelements;  // Elements in this branch
        std::shared_ptr<Tree> parent;       // Parent node in the tree
    public:
        Tree(Element* element, Bus* current_node, Bus* root, std::shared_ptr<Tree> parent = nullptr)
            : element(element), current_node(current_node), root(root), parent(parent) {}

        void addPathElement(Element* element) {
            subelements.push_back(element);  
        }
        //Getters
        Bus* getRoot() const { return root; }
        Element* getElement() const { return element; }
        Bus* getCurrentNode() const { return current_node; }
        const std::vector<Element*>& getSubelements() const { return subelements; }
        std::shared_ptr<Tree> getParent() const { return parent; }

        // Check if a node already exists in this path
        bool containsNode(Bus* node) const;

        // Get the full path of elements from root to current node
        std::vector<Element*> getPath() const;

        // debugging: print the tree nodes
        std::string toString() const; 
    };

    StateSpaceModel() = default; 
	// StateSpaceModel(Network* net);

    static std::vector<std::vector<Bus*>> form_cutset_nodes(const std::vector<Bus*>& buses,
        const std::vector<std::pair<Bus*, Bus*>>& connections);

    static std::vector<std::vector<Element*>> form_cutsets(
        const std::vector<std::vector<Bus*>>& cutset_nodes,
        const std::map<Bus*, std::vector<Element*>>& busToElementsMap);

    //creates a mapping from each bus to the list of elements connected to it.
    static std::map<Bus*, std::vector<Element*>> generateBusToElementsMap(
        const std::vector<Element*>& elements);

    static std::vector<std::vector<Element*>> findLoops(
        const std::vector<Bus*>& nodes,
        const std::map<Bus*, std::vector<Element*>>& node_collection);

    // build A,B,C,D from a populated Network
    void formState(Network* net, const std::vector<Bus*>& out);

    // Getters for Eigen::MatrixXd
    const Eigen::MatrixXd& getA() const { return A; } 
    const Eigen::MatrixXd& getB() const { return B; }
    const Eigen::MatrixXd& getC() const { return C; }
    const Eigen::MatrixXd& getD() const { return D; }

private:
    static void traverseForLoops(
        Bus* current_node,
        Bus* start_node,
        const std::vector<Bus*>& nodes,
        std::vector<std::vector<Element*>>& loop_collection,
        const std::map<Bus*, std::vector<Element*>>& node_collection,
        std::shared_ptr<Tree> current_branch,
        const std::map<Bus*, int>& busIndices);

    // A, B, C, D;  
    Eigen::MatrixXd A; 
    Eigen::MatrixXd B; 
    Eigen::MatrixXd C; 
    Eigen::MatrixXd D; 

    int number_switches;
    int number_independent_sources;
    int number_nodes;
    int total_number_equations;
    int number_outputs;
    int number_state_variables;

	std::vector<Bus*> output; // Output variable indices
    std::unordered_map<Bus*, int> bus_indices;
    std::unordered_map<Element*, int> element_indices;
    std::map<Element*, std::vector<RCP<const Basic>>> symbols_bank;
	std::vector<Element*> list_state_variables; // Elements that are state variables (e.g., inductors, capacitors)
	std::vector<Element*> list_independent_sources; // Independent sources (e.g., voltage, current sources)
	std::vector<Element*> list_switches; // Switches in the network

    void finalizeCounts(Network*);
    void substituteParameters(DenseMatrix& matrix);
};

#endif //STATE_SPACE_MODEL