#ifndef STATE_SPACE_MODEL
#define STATE_SPACE_MODEL

#include <vector>
#include "Element.h"  
#include "Bus.h"                        

class Bus;
class Element;

class StateSpaceModel {
public:
    StateSpaceModel() = default; 
    
    static std::vector<std::vector<Bus*>> from_cutset_nodes(const std::vector<Bus*>& buses,
        const std::vector<std::pair<Bus*, Bus*>>& connections);

    static std::vector<std::vector<Element*>> from_cutsets(
        const std::vector<std::vector<Bus*>>& cutset_nodes,
        const std::map<Bus*, std::vector<Element*>>& busToElementsMap);

    //creates a mapping from each bus to the list of elements connected to it.
    static std::map<Bus*, std::vector<Element*>> generateBusToElementsMap(
        const std::vector<Element*>& elements);
};

#endif //STATE_SPACE_MODEL