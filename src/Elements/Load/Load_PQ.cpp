#include "Load_PQ.h"

LoadPQ::LoadPQ(const std::string& symbol, int pins) : Load_base(symbol, pins, pins) {
    if (pins <= 0) {
        throw std::invalid_argument("Invalid number of pins, must be greater than 0!");
    }

	// It has only OPF representation, so we do not need to compute Y-parameters
    element_OPF_info = { symbol, "1", "0", "0", "0"};
}


