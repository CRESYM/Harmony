#include "Admittance.h"
#include <stdexcept>

using namespace std;
using namespace SymEngine;

Admittance::Admittance(const std::string& symbol, int pins, vector<RCP<Symbol>> values)
    : Element(symbol, pins, pins) { // Call the Element constructor
    // Initialize the admittance 2D array
    admittance = new RCP<Symbol>*[pins]; // Allocate array of pointers
    for (int i = 0; i < pins; i++) {
        admittance[i] = new RCP<Symbol>[pins]; // Allocate each row
    }

    if (values.size() != 0) {  // If there are entries
        if (pins > 0) { // Check for valid number of pins
            if (values.size() == 1) {
                for (int i = 0; i < pins; i++)
                    admittance[i][i] = values[0];
            }
            else if (values.size() == pins) {
                for (int i = 0; i < pins; i++)
                    admittance[i][i] = values[i];
            }
            else if (values.size() == pins * pins) {
                for (int i = 0; i < pins; i++)
                    for (int j = 0; j < pins; j++)
                        admittance[i][j] = values[i * pins + j];
            }
            else {
                throw invalid_argument("Invalid number of admittance vector entries: " + to_string(values.size()));
            }
        }
        else {
            throw invalid_argument("Invalid number of pins, must be greater than 0!");
        }
    }
}

// Destructor to free allocated memory
Admittance::~Admittance() {
    //for (int i = 0; i < input_pins; i++) {
    for (int i = 0; i < getInputPins(); i++) { // Use getter to access input_pins
        delete[] admittance[i];
    }
    delete[] admittance;
}
