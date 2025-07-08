#ifndef MMC_H
#define MMC_H

#include "Element.h"
#include "Filter.h"
#include "Controller.h"

// Forward declarations
class Controller;
class Filter;

class MMC : public Element {
public:
    // Constructor 
    MMC(const std::string& symbol, 
        double omega, double activePower, double reactivePower, double dcPower,
        double minActivePower, double maxActivePower, double minReactivePower,
        double maxReactivePower, double angle, double acVoltage, double dcVoltage, // move to converter -> element (parent) class as a parent of mmc
        double armInductance, double armResistance, double armCapacitance,
        int numSubmodules, double reactorInductance, double reactorResistance,
        double timeDelay)
		: Element(symbol, 3, 1), // AC side - input pins; DC side - output pins
        omega_0(omega), P(activePower), Q(reactivePower), P_dc(dcPower),
        P_min(minActivePower), P_max(maxActivePower), Q_min(minReactivePower),
        Q_max(maxReactivePower), theta(angle), V_m(acVoltage), V_dc(dcVoltage),
        L_arm(armInductance), R_arm(armResistance), C_arm(armCapacitance),
        N(numSubmodules), L_reactor(reactorInductance),
        R_reactor(reactorResistance), t_delay(timeDelay) 
    {
        A_matrix = Eigen::MatrixXd::Zero(6, 6);
        B_matrix = Eigen::MatrixXd::Zero(6, 8);
        C_matrix = Eigen::MatrixXd::Identity(6, 6);
        D_matrix = Eigen::MatrixXd::Zero(6, 8);
    }

    // Constructor to initialize MMC with the converter_params (from init_MMC)
    MMC(const std::string& symbol, const std::vector<double>& converter_params)
        : Element(symbol, 3, 1), // AC side - input pins; DC side - output pins
        omega_0(converter_params[0]), P(converter_params[1]), Q(converter_params[2]),
        P_dc(converter_params[3]), P_min(converter_params[4]), P_max(converter_params[5]),
        Q_min(converter_params[6]), Q_max(converter_params[7]),
        theta(converter_params[8]), V_m(converter_params[9]), V_dc(converter_params[10]),
        L_arm(converter_params[11]), R_arm(converter_params[12]), C_arm(converter_params[13]),
        N(static_cast<int>(converter_params[14])), L_reactor(converter_params[15]),
        R_reactor(converter_params[16]), t_delay(converter_params[17]) {};
  

    // Destructor
    ~MMC() {};
    
    // Initialization methods
    void init_Controller(const std::vector<double>&converter_params);// Method to initialize controllers and Filters in MMC
    void init_Filter(const std::vector<double>& converter_params);
    void update_MMC(double Vm, double theta, double Pac, double Qac, double Vdc, double Pdc);
   
    
    //getter
    Eigen::MatrixXd getA() const { return A_matrix; }
    Eigen::MatrixXd getB() const { return B_matrix; }
    Eigen::MatrixXd getC() const { return C_matrix; }
    Eigen::MatrixXd getD() const { return D_matrix; }
    Eigen::VectorXd getEquilibriumState() const { return equilibrium_state; }

    Eigen::MatrixXd computeStateDerivatives(const Eigen::VectorXd& x, const Eigen::VectorXd& u);
    Eigen::VectorXd computeStateDerivativesLinear(const Eigen::VectorXd& x, const Eigen::VectorXd& u);

    Eigen::MatrixXd computeJacobianNumerically(const Eigen::VectorXd& x0, const Eigen::VectorXd& u0);
    void computeJacobianLinear();

    // Equilibrium point calculation
    void solveEquilibrium();

    Eigen::MatrixXcd computeAdmittanceMatrix(std::complex<double> s);

    // System analysis
    void checkStability() const;
    void printEigenvalues() const;

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
    
    //Add Harmonic Parameters 
    double A_harm = 0.0;         // Amplitude of upper arm harmonic injection
    double B_harm = 0.0;         // Amplitude of lower arm harmonic injection
    double phi1 = 0.0, phi2 = 0.0, phi3 = 0.0; // Phase shifts

    // Custom arm resistance values (for nonlinear models or harmonic tuning)
    double Rn_custom = 0.0;  // Negative arm resistance
    double Rm_custom = 10.0; // Mutual resistance
    
    // System matrices
    Eigen::MatrixXd A_matrix, B_matrix, C_matrix, D_matrix;
    Eigen::VectorXd equilibrium_state;
    
    // Sub-elements
    std::vector<std::pair<std::string, std::shared_ptr<Element>>> subElements;

    std::map<std::string, std::shared_ptr<Controller>> controls;
    std::map<std::string, std::shared_ptr<Filter>> filters;
};

#endif // MMC_H