#include "impedance.h"
#include "constants.h"
#include <cmath>
#include <iostream>

// Function definition for impedance
Impedance impedance(const std::vector<std::vector<Basic>>& value, const std::vector<std::vector<Basic>>& z, int pins, bool transformation){ //z define the value of the impedance pins define the n of pin -> integer
	transformation = false;
	Impedance imp;

	if (!z.empty()) { //if z has a value -> enter
		if (pins != 0) {
			if (z.size() == 1 && z[0].size() == 1) {
				// Create diagonal matrix from z[0][0]
				std::vector<std::vector<Basic>> diagMatrix(pins, std::vector<Basic>(pins, Basic()));
				for (int i = 0; i < pins; ++i) {
					diagMatrix[i][i] = z[0][0];
				}
				imp.value = diagMatrix;
			}
			else if (z.size() == pins && z[0].size() == pins) {
				imp.value = z;
			}
			else if (z.size() == pins * pins) {
				// Reshape z into a square matrix
				std::vector<std::vector<Basic>> squareMatrix(pins, std::vector<Basic>(pins, Basic()));
				for (int i = 0; i < pins; ++i) {
					for (int j = 0; j < pins; ++j) {
						squareMatrix[i][j] = z[i * pins + j][0];
					}
				}
				imp.value = squareMatrix;
			}
			else {
				throw std::invalid_argument("invalid element specification");
			}
		}
		else {
			pins = static_cast<int>(std::sqrt(z.size()));
			// Check if z is a square matrix
			if (pins * pins != z.size()) {
				throw std::invalid_argument("Invalid element specification");
			}

			// Reshape z into a square matrix
			std::vector<std::vector<Basic>> squareMatrix(pins, std::vector<Basic>(pins, Basic()));
			for (int i = 0; i < pins; ++i) {
				for (int j = 0; j < pins; ++j) {
					squareMatrix[i][j] = z[i * pins + j][0];
				}
			}
			imp.value = squareMatrix;
		}
	}

	// Determine ABCD
	// ...

	return imp;
}

/*Impedance impedance(const Basic& z, int pins, bool transformation) {
	Impedance imp;

	// Check if z is zero according to your definition
	if (!z.isZero()) {
		if (pins != 0) {
			if (pins == 1) {
				// Create a diagonal matrix with all elements as z
				std::vector<std::vector<Basic>> diagMatrix(pins, std::vector<Basic>(pins, Basic()));
				diagMatrix[0][0] = z;
				imp.value = diagMatrix;
			}
			else if (pins * pins == 1) {
				// Create a diagonal matrix with all elements as 1
				std::vector<std::vector<Basic>> diagMatrix(pins, std::vector<Basic>(pins, Basic()));
				diagMatrix[0][0] = 1;
				imp.value = diagMatrix;
			}
			else if (pins * pins == z.size) {
				// Reshape z into a square matrix
				std::vector<std::vector<Basic>> squareMatrix(pins, std::vector<Basic>(pins, Basic()));
				for (int i = 0; i < pins; ++i) {
					for (int j = 0; j < pins; ++j) {
						// Access the value at index i * pins + j
						// Assuming z is a one-dimensional array representing the data
						squareMatrix[i][j] = z.ptr[i * pins + j];  // Accessing the pointer directly
					}
				}
				imp.value = squareMatrix;
			}
			else {
				throw std::invalid_argument("Invalid element specification");
			}
		}
		else {
			pins = static_cast<int>(std::sqrt(z.size()));
			// Check if z is a square matrix
			if (pins * pins != z.size()) {
				throw std::invalid_argument("Invalid element specification");
			}

			// Reshape z into a square matrix
			std::vector<std::vector<Basic>> squareMatrix(pins, std::vector<Basic>(pins, Basic()));
			for (int i = 0; i < pins; ++i) {
				for (int j = 0; j < pins; ++j) {
					squareMatrix[i][j] = z[i * pins + j];
				}
			}
			imp.value = squareMatrix;
		}
	}

	// Determine ABCD
	// ...

	return imp;
}*/