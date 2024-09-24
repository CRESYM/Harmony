#include "Admittance.h"

#include <cmath>
using namespace std;
using namespace SymEngine;

Admittance::Admittance(int pins, vector<RCP<Symbol>> values) {
    Element::input_pins = pins;
    Element::output_pins = pins;
    admittance = RCP<Symbol>[pins][pins];
    if (values.capacity() != 0)  // if there are entries
    {
        if (pins != 0) // if there are element connections check further
        {
            switch (values.capacity()) {
            case 1: {
                for (int i = 0; i < pins; i++)
                    admittance[i][i] = values[0];
            }
            case pins: {
                for (int i = 0; i < pins; i++)
                    admittance[i][i] = values[i];
            }
            case pow(pins, 2): {
                for (int i = 0; i < pins; i++)
                    for (int j = 0; j < pins; j++)
                        admittance[i][j] = values[i + j];
            }
            default:
                throw invalid_argument("Invalid number of admittance vector entries: " + values.capacity());
            }
        }
        else
            throw invalid_argument("Invalid number of pins, must be greater than 0!");
    }
}


            

           