#include "Examples.h"

#include "../Solver/DQsym/DQsym.h"

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
		complex<double>(0, 0), complex<double>(0, 0), complex<double>(0.9998, 0);
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
		cout << brkVec << endl;
		cout << endl;
		y = dqSym.DSSS(Ad, Bd, Cd, Dd, swOnRes, swOffRes, swType, brkVec, u, xo, 2e-5, 50.0);
		cout << y << endl;
		cout << endl;
	}
	
}