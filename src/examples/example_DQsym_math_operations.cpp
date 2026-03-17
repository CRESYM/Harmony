#include "Examples.h"

#include "../Solver/DQsym/DQsym.h"
//#include "../Solver/DQsym/dqn2abc.h"
#include "../Solver/Helper_Functions/Visualization.h"

void example_DQsym_math_operations()
{
	// Example usage of DQsym class for mathematical operations
	DQsym dqSym;

	// small test: zeros
	MatrixXcd X = MatrixXcd::Zero(3, 2); // N = 2 -> columns 0..2
	X << complex<double>(0,0), complex<double>(-0.5317,-0.3237), complex<double>(0,0), 
		complex<double>(-0.0657, 0.1971), complex<double>(0,0), complex<double>(0.0576, 0.4674);

	cout << "Input matrix X:" << endl;
	cout << "X size: " << X.rows() << " x " << X.cols() << std::endl;
	cout << X << endl;
	cout << endl;

	cout << "Addition test:" << endl;
	MatrixXcd result = dqSym.add(X, X);
	cout << "Result of addition:" << endl;
	cout << result << endl;
	cout << endl;

	cout << "Subtraction test:" << endl;
	MatrixXcd result2 = dqSym.subtract(X, X);
	cout << "Result of subtraction:" << endl;
	cout << result2 << endl;
	cout << endl;

	cout << "Multiplication test:" << endl;
	MatrixXcd Z = dqSym.multiply(X, X);
	std::cout << "Z size: " << Z.rows() << " x " << Z.cols() << std::endl;
	std::cout << Z << std::endl;
	cout << endl;

	cout << "Integration test:" << endl;
	int N = 1;
	double dt = 0.01, w = 2 * M_PI * 50;
	int nrSig = 1;

	MatrixXcd Xpnz = X;
	MatrixXcd Zpnz_old = MatrixXcd::Zero(nrSig * 3, N + 1);
	MatrixXcd Xpnz_old = MatrixXcd::Zero(nrSig * 3, N + 1);

	MatrixXcd Zpnz = dqSym.integrate(Zpnz_old, Xpnz_old, Xpnz, dt, w);

	std::cout << "Zpnz = \n" << Zpnz << std::endl;

	// DSSS test case
	MatrixXcd Ad(3, 3);
	Ad << complex<double>(-0.9998, 0), complex<double>(0, 0), complex<double>(0, 0), complex<double>(0, 0), complex<double>(-0.9998, 0), complex<double>(0, 0),
		complex<double>(0, 0), complex<double>(0, 0), complex<double>(-0.9998, 0);
	MatrixXcd Bd(3, 9);
	Bd << 9.998900120986692e+04 , 0 ,  0 , 0 , 0 , 0.099989001209867, -0.099989001209867, 0 , 0,
		  0 , 9.998900120986692e+04 , 0 , 0 , 0.099989001209867 , 0 , 0 , -0.099989001209867 , 0,
		  0 , 0 , 9.998900120986692e+04 , 0.099989001209867,  0 , 0 , 0 , 0 , -0.099989001209867;
	MatrixXcd Cd = MatrixXcd(9, 3);
	Cd << 0.001999780024197, 0 , 0,
		  0, 0.001999780024197, 0,
		  0, 0, 0.001999780024197,
		0.000000001999780, 0, 0,
		  0, 0.000000001999780, 0,
		  0, 0, 0.000000001999780,
		0.000000001999780, 0, 0,
		  0, 0.000000001999780, 0,
		0, 0, 0.000000001999780;
	MatrixXcd Dd = MatrixXcd(9, 9);
	Dd << -1.099879013308091e+02, 0, 0, 0, 0, 0.999890012098669, -0.999890012098669, 0, 0,
		0, -1.099879013308091e+02, 0, 0, 0.999890012098669, 0, 0, -0.999890012098669, 0,
		0, 0, -1.099879013308091e+02, 0.999890012098669, 0, 0, 0, 0, -0.999890012098669,
		0.999890012098669, 0, 0, 0, 0, 9.998900120986693e-07, -9.998900120986693e-07, 0, 0,
		0, 0.999890012098669, 0, 0, 9.998900120986693e-07, 0, 0, -9.998900120986693e-07, 0,
		0, 0, 0.999890012098669, 9.998900120986693e-07, 0, 0, 0, 0, -9.998900120986693e-07,
		0.999890012098669, 0, 0, 0, 0, 9.998900120986693e-07, -9.998900120986693e-07, 0, 0,
		0, 0.999890012098669, 0, 0, 9.998900120986693e-07, 0, 0, -9.998900120986693e-07, 0,
		0, 0, 0.999890012098669, 9.998900120986693e-07, 0, 0, 0, 0, -9.998900120986693e-07;
	VectorXd swOnRes(3);
	swOnRes << 1e-2, 1e-2, 1e-2;
	VectorXd swOffRes(3);
	swOffRes << 1e6, 1e6, 1e6;
	VectorXi swType(3);
	swType << 1, 1, 1;
	MatrixXi brkVecs(3, 3);
	brkVecs << 0, 0, 0,  1, 1, 1, 0, 0, 0;
	MatrixXcd u = MatrixXcd::Zero(9, 2);
	u(3, 1) = 1000; u(6, 1) = 1000;
	VectorXcd xo = VectorXcd::Zero(3);

	MatrixXcd y;

	for (int i = 0; i < 3; i++)
	{
		VectorXi brkVec = brkVecs.row(i);
		cout << u << endl;
		cout << brkVec << endl;
		cout << endl;
		y = dqSym.DSSS(Ad, Bd, Cd, Dd, swOnRes, swOffRes, swType, brkVec, u, xo, 2e-5, 50.0);
		cout << y << endl;
		cout << endl;
	}
	

	cout << "\n==========================================\n";
	cout << "dqn2abc test\n";
	cout << "==========================================\n";

	try {
		const double freq_hz = 50.0;
		const double t0 = 0.0;
		const double t1 = 0.04;   // 2 cycles
		const double Ts = 2e-4;

		Eigen::MatrixXcd Xdcpnz_c(3, 6);
		Xdcpnz_c <<
			std::complex<double>(0, 0), std::complex<double>(-0.2455, -0.8802), std::complex<double>(-0.1021, 0.3194), std::complex<double>(0.3739, 0.7338), std::complex<double>(0.2551, -0.8851), std::complex<double>(0.1611, 0.9839),
			std::complex<double>(0, 0), std::complex<double>(-0.2165, 0.5141), std::complex<double>(-0.4002, 0.2114), std::complex<double>(-0.0824, -0.1413), std::complex<double>(-0.5369, 0.2124), std::complex<double>(-0.5156, -0.3999),
			std::complex<double>(0, 0), std::complex<double>(-0.1194, -0.1157), std::complex<double>(-0.3357, -0.1288), std::complex<double>(-0.8896, -0.0929), std::complex<double>(0.0032, -0.2576), std::complex<double>(0.2427, 0.3017);

		cout << "Running simulate_dqn2abc...\n";

		ABCResult res = dqSym.simulate_dqn2abc(Xdcpnz_c, freq_hz, t0, t1, Ts);

		cout << "Simulation done.\n";
		cout << "Samples: " << res.t.size() << "\n";
		cout << "First sample: "
			<< res.Xabc(0, 0) << ", "
			<< res.Xabc(0, 1) << ", "
			<< res.Xabc(0, 2) << "\n";

		cout << "Plotting...\n";

		plot_abc_waveforms(res.t, res.Xabc, "dqn2abc Example");

		cout << "Plot finished.\n";

		// Prevent immediate exit (important!)
		std::cout << "Press Enter to continue...\n";
		std::cin.get();
	}
	catch (const std::exception& e) {
		std::cerr << "ERROR in dqn2abc test: " << e.what() << "\n";
	}


}

