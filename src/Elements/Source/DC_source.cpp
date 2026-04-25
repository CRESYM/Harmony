#include "DC_source.h"

DC_source::DC_source(const std::string& symbol, const std::string& location, int pins, double Vi, double Z)
	: Source_base(symbol, location, pins){
	// For a DC source, the Y-parameter matrix is not frequency-dependent and can have resistive conductance entries based on the internal resistance
    if (pins > 0) { // Check for valid number of pins
        for (int i = 0; i < pins; i++) {
            Y_matrix.set(i, i, div(integer(1), real_double(Z)));
			V.push_back(Vi); // Assuming the same voltage for all pins, can be modified for different voltages per pin
			Zsrc.push_back(Z); // Assuming the same internal resistance for all pins, can be modified for different resistances per pin
        }
    }
    else
        throw invalid_argument("Invalid number of pins, must be greater than 0!");

    // Fill in the complete Y parameters
    for (int i = 0; i < pins; i++)
        for (int j = 0; j < pins; j++) {
            Y_matrix.set(pins + i, j, sub(zero, Y_matrix.get(i, j)));
            Y_matrix.set(pins + i, pins + j, Y_matrix.get(i, j));
            Y_matrix.set(i, pins + j, sub(zero, Y_matrix.get(i, j)));
        }
}


DC_source::DC_source(const std::string& symbol, const std::string& location, int pins, const vector<double>& Vi, double Z)
    : Source_base(symbol, location, pins) {
    // For a DC source, the Y-parameter matrix is not frequency-dependent and can be set to zero
    if (Vi.size() != 0)  // if there are entries
    {
        if (pins > 0) { // Check for valid number of pins
            if (Vi.size() == pins) {
                for (int i = 0; i < pins; i++) {
                    Y_matrix.set(i, i, div(integer(1), real_double(Z)));
                    Zsrc.push_back(Z);
                    V.push_back(Vi[i]); // Assuming the same voltage for all pins, can be modified for different voltages per pin
                }
            }
            else
                throw invalid_argument("Invalid number of voltage vector entries: " + std::to_string(Vi.size()));
        }
    }
    else
        throw invalid_argument("Invalid number of pins, must be greater than 0!");

    // Fill in the complete Y parameters
    for (int i = 0; i < pins; i++)
        for (int j = 0; j < pins; j++) {
            Y_matrix.set(pins + i, j, sub(zero, Y_matrix.get(i, j)));
            Y_matrix.set(pins + i, pins + j, Y_matrix.get(i, j));
            Y_matrix.set(i, pins + j, sub(zero, Y_matrix.get(i, j)));
        }
}

DC_source::DC_source(const std::string& symbol, const std::string& location, int pins, double Vi, const vector<double>& Z)
    : Source_base(symbol, location, pins) {
    // For a DC source, the Y-parameter matrix is not frequency-dependent and can be set to zero
    if (Z.size() != 0)  // if there are entries
    {
        if (pins > 0) { // Check for valid number of pins
            if (Z.size() == 1) {
                for (int i = 0; i < pins; i++) {
                    Y_matrix.set(i, i, div(integer(1), real_double(Z[0])));
                    Zsrc.push_back(Z[0]);
                    V.push_back(Vi); // Assuming the same voltage for all pins, can be modified for different voltages per pin
                }
            }
            else if (Z.size() == pins) {
                for (int i = 0; i < pins; i++) {
                    Y_matrix.set(i, i, div(integer(1), real_double(Z[i])));
                    Zsrc.push_back(Z[i]);
                    V.push_back(Vi); // Assuming the same voltage for all pins, can be modified for different voltages per pin
                }
            }
            else
                throw invalid_argument("Invalid number of series impedance vector entries: " + std::to_string(Z.size()));
        }
    }
    else
        throw invalid_argument("Invalid number of pins, must be greater than 0!");

    // Fill in the complete Y parameters
    for (int i = 0; i < pins; i++)
        for (int j = 0; j < pins; j++) {
            Y_matrix.set(pins + i, j, sub(zero, Y_matrix.get(i, j)));
            Y_matrix.set(pins + i, pins + j, Y_matrix.get(i, j));
            Y_matrix.set(i, pins + j, sub(zero, Y_matrix.get(i, j)));
        }
}

DC_source::DC_source(const std::string& symbol, const std::string& location, int pins, const vector<double>& Vi, const vector<double>& Z)
    : Source_base(symbol, location, pins) {
    // For a DC source, the Y-parameter matrix is not frequency-dependent and can be set to zero
    if (Vi.size() != 0 && Z.size() != 0)  // if there are entries
    {
        if (pins > 0) { // Check for valid number of pins
            if (Vi.size() == pins && Z.size() == pins) {
                for (int i = 0; i < pins; i++) {
                    Y_matrix.set(i, i, div(integer(1), real_double(Z[i])));
                    Zsrc.push_back(Z[i]);
                    V.push_back(Vi[i]); // Assuming the same voltage for all pins, can be modified for different voltages per pin
                }
            }
            else
                throw invalid_argument("Invalid number of voltage or series impedance vector entries: " + std::to_string(Vi.size()) + " or " + std::to_string(Z.size()));
        }
    }
    else
        throw invalid_argument("Invalid number of pins, must be greater than 0!");
    // Fill in the complete Y parameters
    for (int i = 0; i < pins; i++)
        for (int j = 0; j < pins; j++) {
            Y_matrix.set(pins + i, j, sub(zero, Y_matrix.get(i, j)));
            Y_matrix.set(pins + i, pins + j, Y_matrix.get(i, j));
            Y_matrix.set(i, pins + j, sub(zero, Y_matrix.get(i, j)));
        }
}