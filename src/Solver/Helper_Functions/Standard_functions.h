#ifndef _STANDARD_FUNCTIONS_H_
#define _STANDARD_FUNCTIONS_H_

#include "../../Constants.h"

// Standard constants
extern const double mu_0; // vacuum permitivity
extern const double epsilon_0; // vacuum permeability
extern const double gamma_num;


// Standard mathematical functions

// Helper sign function
extern int sgn(int v);
extern MatrixXcd vectorToMatrix(const vector<vector<complex<double>>>& vec);
extern vector<vector<complex<double>>> matrixToVector(const MatrixXcd& mat);

// Functions for matrix operations
extern vector<vector<complex<double>>> mat_mul(const vector<vector<complex<double>>>& A, const vector<vector<complex<double>>>& B);
extern vector<vector<complex<double>>> mul_scalar(const vector<vector<complex<double>>>& A, const complex<double>& scalar);
extern vector<vector<complex<double>>> mat_add(const vector<vector<complex<double>>>& A, const vector<vector<complex<double>>>& B);
extern vector<vector<complex<double>>> mat_sub(const vector<vector<complex<double>>>& A, const vector<vector<complex<double>>>& B);
extern vector<vector<complex<double>>> mat_transpose(const vector<vector<complex<double>>>& A);
extern vector<vector<complex<double>>> get_block(const vector<vector<complex<double>>>& Y, int r_off, int c_off, int r_num, int c_num);

#endif // _STANDARD_FUNCTIONS_H_