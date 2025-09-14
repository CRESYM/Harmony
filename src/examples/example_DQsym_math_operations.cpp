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

	MatrixXcd Zpnz = dqSym.integrate(Xpnz, Zpnz_old, Xpnz_old, dt, w);

	std::cout << "Zpnz = \n" << Zpnz << std::endl;
}