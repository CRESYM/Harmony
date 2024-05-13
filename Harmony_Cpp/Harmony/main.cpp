#include "Element.h"
#include "Cable.h"
#include "eval_parameter.h"

#include <expression.h>
#include <complex_double.h>
#include <complex.h>
#include <eval_double.h>
#include <symengine_config.h>

#include <iostream>
#include <complex>
#include <vector>

using namespace std;

int main()
{
	using namespace SymEngine;

	// Define a Cable object
	Cable c;

	std::unordered_map<Element, int, ElementHash> myMap;

	// Define some sample values for P and Z matrices
	std::vector<std::vector<double>> P = {
		{1.0, 2.0, 3.0},
		{4.0, 5.0, 6.0},
		{7.0, 8.0, 9.0}
	};

	std::vector<std::vector<double>> Z = {
		{9.0, 8.0, 7.0},
		{6.0, 5.0, 4.0},
		{3.0, 2.0, 1.0}
	};

	// Define some sample kwargs
	std::unordered_map<std::string, std::vector<std::pair<double, double>>> kwargs;
	// Populate kwargs as needed

	// Call the cable function
	bool transformation = true;
	cable(c, P, Z, kwargs, transformation);

	// Insert an element into myMap
	std::unordered_map<std::string, std::string> args = {
	{"symbol", "some_symbol"},
	{"input_pins", "4"},
	{"output_pins", "2"},
	{"transformation", "true"}
	};

	//std::unordered_map<Element, int, ElementHash> myMap;
	Element elem(args);
	myMap.emplace(elem, 42); // Inserting the element with an arbitrary value

	//test the eval_parameters function
	// Define a sample Complex number
	const std::complex<double> s(1.0, 2.0);

	std::cout << "yes!.\n";

	// Call the eval_parameters function

	Eval_parameter evalParam;
	auto result = evalParam.eval_parameters(c, s);

	// Print the result
	std::cout << "Z matrix:" << std::endl;
	for (const auto& row : result.first) {
		for (const auto& elem : row) {
			/// Evaluate the SymEngine expression to get a real number
			std::cout << elem.real() << " + " << elem.imag() << "i ";
		}
		std::cout << std::endl;
	}

	std::cout << "Y matrix:" << std::endl;
	for (const auto& row : result.second) {
		for (const auto& elem : row) {
			//std::cout << elem.getReal() << " + " << elem.getImag() << "i ";
			std::cout << elem.real() << " + " << elem.imag() << "i ";
		}
		std::cout << std::endl;
	}


	std::cout << "hello" << std::endl;

	return 0;

}