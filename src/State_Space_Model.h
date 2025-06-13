#ifndef STATE_SPACE_MODEL
#define STATE_SPACE_MODEL

#include <vector>
#include "Element.h"  
#include "Bus.h"                        

class Bus;
class Element;

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
    
    static std::vector<std::vector<Bus*>> from_cutset_nodes(const std::vector<Bus*>& buses,
        const std::vector<std::pair<Bus*, Bus*>>& connections);

    static std::vector<std::vector<Element*>> from_cutsets(
        const std::vector<std::vector<Bus*>>& cutset_nodes,
        const std::map<Bus*, std::vector<Element*>>& busToElementsMap);

    //creates a mapping from each bus to the list of elements connected to it.
    static std::map<Bus*, std::vector<Element*>> generateBusToElementsMap(
        const std::vector<Element*>& elements);

    static std::vector<std::vector<Element*>> findLoops(
        const std::vector<Bus*>& nodes,
        const std::map<Bus*, std::vector<Element*>>& node_collection);

private:
    static void traverseForLoops(
        Bus* current_node,
        Bus* start_node,
        const std::vector<Bus*>& nodes,
        std::vector<std::vector<Element*>>& loop_collection,
        const std::map<Bus*, std::vector<Element*>>& node_collection,
        std::shared_ptr<Tree> current_branch,
        const std::map<Bus*, int>& busIndices);
};

#endif //STATE_SPACE_MODEL