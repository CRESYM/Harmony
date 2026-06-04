#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"

void example_cable(bool plotting_enabled /*=true*/) {
	// Cable constructor check

	Cable::Conductor* conductor1 = new Cable::Conductor(0, 24.25e-3, 1.72e-8);
	Cable::Conductor* conductor2 = new Cable::Conductor(41.75e-3, 46.25e-3, 22e-8);
	Cable::Conductor* conductor3 = new Cable::Conductor(49.75e-3, 60.55e-3, 18e-8, 10);
	Cable::Insulator* insulator1 = new Cable::Insulator(24.25e-3, 41.75e-3, 2.3);
	Cable::Insulator* insulator2 = new Cable::Insulator(46.25e-3, 49.75e-3, 2.3);
	Cable::Insulator* insulator3 = new Cable::Insulator(60.55e-3, 65.75e-3, 2.3);

	map<string, Cable::Conductor*> conductors = { {"C1", conductor1}, {"C2", conductor2}, {"C3", conductor3}};
	map<string, Cable::Insulator*> insulators = { {"I1", insulator1}, {"I2", insulator2}, {"I3", insulator3}};

	std::tuple<double, double, double> earth_params = std::make_tuple(1.0, 1.0, 1.0);

	std::vector<std::pair<double, double>> positions = { make_pair(0.0, 1.0)};

	Cable* cable = new Cable("cable", "DC1", 1, "aerial", 100e3, earth_params, conductors,
		insulators, positions);

	cable->printElementValues();

	cable->writeFile(1, 1e5, 10000);
	cout << "Y-parameters printed in file." << endl;
	
	if (plotting_enabled) {
		cable->plotYParameters(0.1, 1e6, 1e4);
	}

	cout << "Press Enter to continue...\n";
	cin.get();
}