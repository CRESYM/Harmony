#include "MMC.h"
#include "Controller.h"
#include "Filter.h"
#include <memory>
#include <stdexcept>
#include <vector>
#include <Eigen/Dense>
#include <cmath>
#include <iostream>

// Static method to initialize the MMC with converter parameters
MMC MMC::init_MMC(const std::vector<double>& converter_params) {
    return MMC(
        "MMC", 100, 10,                            // symbol, inputPins, outputPins
        converter_params[0],                       // omega
        converter_params[1],                       // activePower
        converter_params[2],                       // reactivePower
        converter_params[3],                       // dcPower
        converter_params[4],                       // minActivePower
        converter_params[5],                       // maxActivePower
        converter_params[6],                       // minReactivePower
        converter_params[7],                       // maxReactivePower
        converter_params[8],                       // angle
        converter_params[9],                       // acVoltage
        converter_params[10],                      // dcVoltage
        converter_params[11],                      // armInductance
        converter_params[12],                      // armResistance
        converter_params[13],                      // armCapacitance
        static_cast<int>(converter_params[14]),    // numSubmodules
        converter_params[15],                      // reactorInductance
        converter_params[16],                      // reactorResistance
        converter_params[17]                       // timeDelay
    );  // Return the fully initialized MMC instance
}

// Initialize the controller(s) in MMC using provided parameters
void MMC::init_Controller(const std::vector<double>& converter_params) {
    // Ensure that there are enough parameters for Controller
    if (converter_params.size() < 27) {
        throw std::invalid_argument("Insufficient converter parameters for Controller initialization.");
    }
    //Add a default controller using parameters
    auto controller = std::make_shared<Controller>(
        "Controller1",                         // Symbol for controller
        static_cast<int>(converter_params[18]), // Pins
        converter_params[19],                  // Proportional Gain (Kp)
        converter_params[20],                  // Integral Gain (Ki)
        converter_params[21],                  // Damping Ratio (zeta)
        converter_params[22],                  // Bandwidth
        std::vector<double>(converter_params.begin() + 23, converter_params.end())  // Reference vector (using the rest of the values)
    );
    // Add the controller to the 'controls' map
    //controls["Controller1"] = controller;
    controls[controller->getElementSymbol()] = controller;  // Add filter to the map
}

// Initialize the filter(s) in MMC using provided parameters
void MMC::init_Filter(const std::vector<double>& converter_params) {
    if (converter_params.size() < 27) {
        throw std::invalid_argument("Insufficient converter parameters for Filter initialization.");
    }
    // Add a default filter using parameters
    auto filter = std::make_shared<Filter>(
        "Filter1",                            // Symbol for filter
        static_cast<int>(converter_params[23]), // Pins (example value, adjust accordingly)
        converter_params[24],                  // Time Constant (T)
        converter_params[25],                  // Damping Ratio (zeta)
        converter_params[26]                   // Bandwidth
    );

    //filters["Filter1"] = filter;  // Add filter to the map
    filters[filter->getElementSymbol()] = filter;  // Add filter to the map
}
void MMC::update_MMC(double Vm, double theta, double Pac, double Qac, double Vdc, double Pdc) {
    this->V_m = Vm;
    this->theta = theta;
    this->V_dc = Vdc;
    this->P = Pac;
    this->Q = Qac;
    this->P_dc = Pdc;
}

void MMC::solveEquilibrium() {
    equilibrium_state = Eigen::VectorXd::Zero(13);
}

void MMC::computeJacobian() {
    A_matrix = Eigen::MatrixXd::Zero(13, 13);
    B_matrix = Eigen::MatrixXd::Zero(13, 3);
}

Eigen::MatrixXd MMC::computeStateDerivatives(const Eigen::VectorXd& state, const Eigen::VectorXd& inputs) {
    Eigen::MatrixXd F = Eigen::MatrixXd::Zero(13, 1);
    return F;
}

Eigen::MatrixXd MMC::computeJacobianNumerically(const Eigen::VectorXd& state, const Eigen::VectorXd& inputs) {
    Eigen::MatrixXd J = Eigen::MatrixXd::Zero(13, 13);
    return J;
}