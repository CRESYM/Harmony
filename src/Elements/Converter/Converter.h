#ifndef _CONVERTER_H_
#define _CONVERTER_H_


#include "../Element.h"
#include "../../Include_control_blocks.h"


class Converter : public Element {
public:

	// Constructor
	Converter(const std::string& symbol, const std::string& location)
		: Element(symbol, location, 3, 1) {
	}

	//getter
	Eigen::MatrixXd getA() const { return A_matrix; }
	Eigen::MatrixXd getB() const { return B_matrix; }
	Eigen::MatrixXd getC() const { return C_matrix; }
	Eigen::MatrixXd getD() const { return D_matrix; }
	Eigen::VectorXd getEquilibriumState() const { return equilibrium_state; }
	VectorXcd getEigenvalues() { return eigenvalues; }
	VectorXcd getEigenvectors() { return eigenvectors; }
	string getACarea() const {
		auto pos = element_location.find('_');
		return element_location.substr(0, pos);
	} // Get AC area from location string
	string getDCarea() const {
		auto pos = element_location.find('_');
		return element_location.substr(pos + 1);
	} // Get DC area from location string

	// Solvers
	virtual void solveEquilibrium() {};
	virtual void computeABCD() {};
	virtual Eigen::MatrixXd computeStateDerivatives(const Eigen::VectorXd& x, const Eigen::VectorXd& u) {
		return Eigen::MatrixXd::Zero(1, 1);
	};
	void computeEigenvalues() {
		Eigen::EigenSolver<Eigen::MatrixXd> es(A_matrix);
		eigenvalues = es.eigenvalues();
		eigenvectors = es.eigenvectors();
	}
	// Compute participation factors from the state matrix A
	// Returns: MatrixXd (n x n) where P(i,j) is participation of state i in mode j
	Eigen::MatrixXd computeParticipationFactors(const Eigen::MatrixXd& A_matrix) {
		// Step 1: Eigen decomposition
		Eigen::EigenSolver<Eigen::MatrixXd> es(A_matrix);

		Eigen::MatrixXcd V = es.eigenvectors();          // Right eigenvectors
		Eigen::VectorXcd lambda = es.eigenvalues();      // Eigenvalues
		Eigen::MatrixXcd W = V.inverse();                // Left eigenvectors (transpose of inverse if needed)

		const int n = A_matrix.rows();
		Eigen::MatrixXd P(n, n);

		// Step 2: Compute participation factors
		// P_ij = |phi_ij * psi_ji|
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				std::complex<double> val = V(i, j) * W(j, i);
				P(i, j) = std::abs(val);
			}
		}

		// Step 3: Normalize participation factors (optional)
		for (int j = 0; j < n; ++j) {
			double col_sum = P.col(j).sum();
			if (col_sum > 0.0) {
				P.col(j) /= col_sum;
			}
		}

		return P;
	}

	// Time-domain simulation
	virtual VectorXd simulateTimeStep(const Eigen::VectorXd& initial_state, double dt) { VectorXd::Zero(1, 1); }

	// System analysis
	void checkStability() const;
	void printEigenvalues() const;

	// Plotting
	virtual void plotEigenvalues() override;
	virtual void plotParticipationFactors() override;


protected:
	double omega_0;  // Nominal frequency
	double P;        // Active power [W]
	double Q;        // Reactive power [VA]
	double P_dc;     // DC power [W]
	double P_min;    // Min active power output [W]
	double P_max;    // Max active power output [W]
	double Q_min;    // Min reactive power output [VA]
	double Q_max;    // Max reactive power output [VA]
	double theta;    // AC voltage angle [rad]
	double V_m;      // AC voltage amplitude [V]
	double V_dc;     // DC-bus voltage [V]
	double L_reactor; // Inductance of the phase reactor [H]
	double R_reactor; // Resistance of the phase reactor [Omega]
	double t_delay;   // Time delay [s]

	// System matrices
	MatrixXd A_matrix, B_matrix, C_matrix, D_matrix;
	MatrixXd Adelay, Bdelay, Cdelay, Ddelay; // Delay system matrices
	int pade_order = 2; // Order of Padé approximation for delays
	VectorXd equilibrium_state;
	VectorXcd eigenvalues;
	VectorXcd eigenvectors;

	VectorXcd initial_state; // Initial state for time-domain simulations


	std::map<std::string, Controller*> controls; // Map of existing controllers
	std::map<std::string, Filter*> filters;      // Map of existing filters   

	// List of controller and filter names, it can be changed only by developers
	const std::vector<std::string> controller_list = {
		"pll",  "dc_voltage", "active_power", "ac_voltage", "reactive_power", "energy", "zcc", "occ", "ccc",
		"droop"
	}; // List of controller names
	const std::vector<std::string> filter_list = {
		"ac_voltage_dq", "ac_voltage", "active_power", "reactive_power", "dc_voltage"
	}; // List of filter names

};

#endif // _CONVERTER_H_
