#ifndef _CONVERTER_H_
#define _CONVERTER_H_

/**
 * @file Converter.h
 * @brief Base class for power electronic converters with state-space models.
 */

#include "Elements/Element.h"
#include "core/Include_control_blocks.h"

/**
 * @class Converter
 * @brief Abstract power converter with ABCD matrices, controllers, and filters.
 * @ingroup converter
 */
class Converter : public Element {
public:

	/**
	 * @brief Construct a three-phase, two-terminal converter element.
	 * @param symbol Element identifier.
	 * @param location Network area string encoding AC and DC areas (e.g. AC1_DC1).
	 */
	Converter(const std::string& symbol, const std::string& location)
		: Element(symbol, location, 3, 2) {}

	virtual ~Converter();

	// Continuous-time matrix getters  
	Eigen::MatrixXd getA() const { return A_matrix; }
	Eigen::MatrixXd getB() const { return B_matrix; }
	Eigen::MatrixXd getC() const { return C_matrix; }
	Eigen::MatrixXd getD() const { return D_matrix; }

	Eigen::VectorXd getEquilibriumState() const { return equilibrium_state; }

	double getP() const { return P; }
	double getQ() const { return Q; }
	double getPdc() const { return P_dc; }
	double getVm() const { return V_m; }
	double getTheta() const { return theta; }
	double getVdc() const { return V_dc; }

	/** @brief Print the voltages and powers used to linearize Y(s). */
	void dumpLinearizationOp(const std::string& id) const;

	/** @brief Use a DQsym (or other) state as the linearization point, skipping Newton. */
	void setEquilibriumState(const Eigen::VectorXd& x,
		const Eigen::VectorXd& u = Eigen::VectorXd())
	{
		equilibrium_state = x;
		operating_input_ = u;
		if (u.size() < 3)
			return;
		V_dc = u(0);
		V_m = std::hypot(u(1), u(2));
		theta = std::atan2(-u(2), u(1));
		const int p = static_cast<int>(x.size()) - 12;
		if (p >= 0 && static_cast<int>(x.size()) >= p + 2) {
			P = 1.5 * (u(1) * x(p) + u(2) * x(p + 1));
			Q = 1.5 * (u(2) * x(p) - u(1) * x(p + 1));
		}
	}

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
	virtual vector<MatrixXcd> simulateTimeStep(const vector<MatrixXcd>& input, double Ts, int nKeep1, int nKeep2) { return vector<MatrixXcd>(1, MatrixXcd::Zero(1, 1)); }

	// System analysis
	void checkStability() const;
	void printEigenvalues() const;

	// Plotting
	virtual void plotEigenvalues() override;
	virtual void plotParticipationFactors() override;


protected:
	double omega_0;  // Nominal frequency
	double P;        // Active power [W]; >0 = AC export. P = 1.5(Vd Id + Vq Iq)
	double Q;        // Reactive power [VAr]; report Park Q = 1.5(Vq Id - Vd Iq)
	double P_dc;     // DC power [W]; >0 = DC import. Pdc = 3 Vdc iΣz
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
	MatrixXd A_matrix, B_matrix, C_matrix, D_matrix; // Continuous-time system matrices
	MatrixXd Adelay, Bdelay, Cdelay, Ddelay; // Delay system matrices

	int pade_order = 2; // Order of Padé approximation for delays
	VectorXd equilibrium_state;
	VectorXd operating_input_; // non-empty → computeABCD uses this u0 (DQsym snapshot)

	VectorXcd initial_state; // Initial state for time-domain simulations


	std::map<std::string, std::unique_ptr<Controller>> controls;
	std::map<std::string, std::unique_ptr<Filter>> filters; 

	// List of controller and filter names, it can be changed only by developers
	const std::vector<std::string> controller_list = {
		"pll",  "dc_voltage", "active_power", "ac_voltage", "reactive_power", "energy", "zcc", "occ", "ccc",
		"droop", "gfm"
	}; // Trailing slot (gfm) may be omitted in legacy packs.
	const std::vector<std::string> filter_list = {
		"ac_voltage_dq", "ac_voltage", "active_power", "reactive_power", "dc_voltage"
	}; // List of filter names



	

};

#endif // _CONVERTER_H_
