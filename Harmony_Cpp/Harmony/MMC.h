#ifndef MMC_H
#define MMC_H

#include "Element.h"
#include "Filter.h"
#include "Controller.h"

class MMC : public Element {
public:
    // Constructor
    MMC(const std::string& symbol, int inputPins, int outputPins,
        double omega, double activePower, double reactivePower, double dcPower,
        double minActivePower, double maxActivePower, double minReactivePower,
        double maxReactivePower, double angle, double acVoltage, double dcVoltage,
        double armInductance, double armResistance, double armCapacitance,
        int numSubmodules, double reactorInductance, double reactorResistance,
        double timeDelay)
        : Element(symbol, inputPins, outputPins),
        omega_0(omega), P(activePower), Q(reactivePower), P_dc(dcPower),
        P_min(minActivePower), P_max(maxActivePower), Q_min(minReactivePower),
        Q_max(maxReactivePower), theta(angle), V_m(acVoltage), V_dc(dcVoltage),
        L_arm(armInductance), R_arm(armResistance), C_arm(armCapacitance),
        N(numSubmodules), L_reactor(reactorInductance),
        R_reactor(reactorResistance), t_delay(timeDelay) {}

    // Destructor
    ~MMC() {}

    // Add a filter or controller
    void addSubElement(const std::string& subElementName, const std::shared_ptr<Element>& subElement) {
        subElements.push_back({ subElementName, subElement });
    }

    // Print all sub-elements (filters/controllers)
    void printSubElements() const override {
        std::cout << "MMC Sub-Elements:" << std::endl;
        for (const auto& pair : subElements) {
            const std::string& subElementName = pair.first; // Key
            const std::shared_ptr<Element>& subElement = pair.second; // Value
            std::cout << "  Sub-Element Name: " << subElementName << std::endl;

            // Check the type of sub-element using dynamic_cast
            if (auto filter = dynamic_cast<Filter*>(subElement.get())) {
                std::cout << "  (Filter)" << std::endl;
                filter->printElementValues();  // If Filter, print its values
            }
            else if (auto controller = dynamic_cast<Controller*>(subElement.get())) {
                std::cout << "  (Controller)" << std::endl;
                controller->printElementValues();  // If Controller, print its values
            }
            else {
                std::cout << "  (Unknown Element Type)" << std::endl;
            }
        }
    }

    // Override to print MMC-specific parameters
    virtual void printElementValues() override {
        Element::printElementInfo();
        std::cout << "MMC Parameters:\n"
            << "  Active Power (P): " << P << " MW\n"
            << "  Reactive Power (Q): " << Q << " MVA\n"
            << "  DC Power (P_dc): " << P_dc << " kW\n"
            << "  AC Voltage Amplitude (V_m): " << V_m << " kV\n"
            << "  DC Voltage (V_dc): " << V_dc << " kV\n"
            << "  Nominal Frequency (omega_0): " << omega_0 << " rad/s\n"
            << "  Arm Inductance (L_arm): " << L_arm << " H\n"
            << "  Arm Resistance (R_arm): " << R_arm << " Ω\n"
            << "  Capacitance per Submodule (C_arm): " << C_arm << " F\n"
            << "  Number of Submodules (N): " << N << "\n"
            << "  Reactor Inductance (L_reactor): " << L_reactor << " H\n"
            << "  Reactor Resistance (R_reactor): " << R_reactor << " Ω\n"
            << "  Time Delay (t_delay): " << t_delay << " s\n";
        printSubElements();
    }

private:
    double omega_0;  // Nominal frequency
    double P;        // Active power [MW]
    double Q;        // Reactive power [MVA]
    double P_dc;     // DC power [kW]
    double P_min;    // Min active power output [MW]
    double P_max;    // Max active power output [MW]
    double Q_min;    // Min reactive power output [MVA]
    double Q_max;    // Max reactive power output [MVA]
    double theta;    // AC voltage angle [rad]
    double V_m;      // AC voltage amplitude [kV]
    double V_dc;     // DC-bus voltage [kV]
    double L_arm;    // Arm inductance [H]
    double R_arm;    // Arm resistance
    double C_arm;    // Capacitance per submodule [F]
    int N;           // Number of submodules per arm
    double L_reactor; // Inductance of the phase reactor [H]
    double R_reactor; // Resistance of the phase reactor [Ω]
    double t_delay;   // Time delay [s]

    // Sub-elements
    std::vector<std::pair<std::string, std::shared_ptr<Element>>> subElements;
};

#endif // MMC_H