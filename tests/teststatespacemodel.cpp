#include <gtest/gtest.h>
#include "network.h"
#include "Bus.h"
#include "Include_components.h"

class TestStateSpaceModel : public testing::Test {};

TEST_F(TestStateSpaceModel, TestCutsets) {
	// Cutset
	Bus* bus0 = new Bus("0", 1);
	Bus* bus1 = new Bus("1", 1);
	Bus* bus2 = new Bus("2", 1);

	std::vector<Bus*> buses = { bus0, bus1, bus2 };
	// Create Elements (assumes ACSource and Resistor constructors take two buses and a value)
	AC_source* ac = new AC_source("ac", 1, DenseMatrix(1, 1, { integer(10) }));
	Resistor* r1 = new Resistor("R1", 2, 2.0);
	Resistor* r2 = new Resistor("R2", 2, 2.0);
	Resistor* r3 = new Resistor("R3", 2, 2.0);

	// Manually connect elements to buses
	ac->attachBus(bus1, 1);
	ac->attachBus(bus0, 2);

	r1->attachBus(bus1, 1);
	r1->attachBus(bus0, 2);

	r2->attachBus(bus1, 1);
	r2->attachBus(bus2, 2);

	r3->attachBus(bus2, 1);
	r3->attachBus(bus0, 2);

	// Collect elements
	std::vector<Element*> elements = { ac, r1, r2, r3 };

	auto busToElementsMap = StateSpaceModel::generateBusToElementsMap(elements);

	// Create the StateSpaceModel object
	StateSpaceModel model;

	//Generate all bus cutsets
	std::vector<std::vector<Bus*>> cutset_nodes = model.from_cutset_nodes(buses, {});

	std::cout << "Cutset Nodes:" << std::endl;
	for (const auto& group : cutset_nodes) {
		std::cout << "[ ";
		for (Bus* b : group) {
			std::cout << b->getBusName() << " ";
		}
		std::cout << "]" << std::endl;
	}

	// Generate element cutsets from the bus cutsets
	std::vector<std::vector<Element*>> cutset_elements = model.from_cutsets(cutset_nodes, busToElementsMap);

	std::cout << "\nCutset Elements:" << std::endl;
	for (const auto& group : cutset_elements) {
		std::cout << "[ ";
		for (Element* e : group) {
			std::cout << e->getElementSymbol() << " ";
		}
		std::cout << "]" << std::endl;
	}

	// Check if the cutset nodes and cutsets are included correctly
	EXPECT_EQ(cutset_nodes.size(), 7);
	EXPECT_EQ(cutset_elements.size(), 7);

	delete bus0;
	delete bus1;
	delete bus2;
	delete ac;
	delete r1;
	delete r2;
	delete r3;
}

TEST_F(TestStateSpaceModel, TestLoops) {
	// Cutset
	Bus* bus0 = new Bus("0", 1);
	Bus* bus1 = new Bus("1", 1);
	Bus* bus2 = new Bus("2", 1);

	std::vector<Bus*> buses = { bus0, bus1, bus2 };
	// Create Elements (assumes ACSource and Resistor constructors take two buses and a value)
	AC_source* ac = new AC_source("ac", 1, DenseMatrix(1, 1, { integer(10) }));
	Resistor* r1 = new Resistor("R1", 2, 2.0);
	Resistor* r2 = new Resistor("R2", 2, 2.0);
	Resistor* r3 = new Resistor("R3", 2, 2.0);

	// Manually connect elements to buses
	ac->attachBus(bus1, 1);
	ac->attachBus(bus0, 2);

	r1->attachBus(bus1, 1);
	r1->attachBus(bus0, 2);

	r2->attachBus(bus1, 1);
	r2->attachBus(bus2, 2);

	r3->attachBus(bus2, 1);
	r3->attachBus(bus0, 2);

	// Collect elements
	std::vector<Element*> elements = { ac, r1, r2, r3 };

	auto busToElementsMap = StateSpaceModel::generateBusToElementsMap(elements);

	// Create the StateSpaceModel object
	StateSpaceModel model;

	// Find loops
	auto loops = StateSpaceModel::findLoops(buses, busToElementsMap);

	// Print loop results
	std::cout << "\nLoops found:" << std::endl;
	for (size_t i = 0; i < loops.size(); ++i) {
		std::cout << "Loop " << i + 1 << ": ";
		for (Element* e : loops[i]) {
			std::cout << e->getElementSymbol() << " ";
		}
		std::cout << std::endl;
	}

	EXPECT_EQ(loops.size(), 4); // Expecting 4 loops based on the connections
	
	delete bus0;
	delete bus1;
	delete bus2;
	delete ac;
	delete r1;
	delete r2;
	delete r3;
}

