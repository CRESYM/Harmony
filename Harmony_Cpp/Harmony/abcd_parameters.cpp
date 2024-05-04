#include "abcd_parameters.h" // Include the header file with function prototypes
#include <cmath> // Include cmath for mathematical functions
#include <complex>
#include <vector>

/*
// Define shorthand for complex type
using Complex = std::complex<double>;


// Function to connect series components
std::vector<std::vector<Complex>> connect_series(const std::vector<std::vector<Complex>>& a, const std::vector<std::vector<Complex>>& b) {
	// Perform element-wise multiplication of corresponding elements
	std::vector<std::vector<Complex>> result(a.size(), std::vector<Complex>(a[0].size()));
	for (size_t i = 0; i < a.size(); ++i) {
		for (size_t j = 0; j < a[0].size(); ++j) {
			result[i][j] = a[i][j] * b[i][j];
		}
	}
	return result;
}

// Function to connect parallel components
std::vector<std::vector<Complex>> connect_parallel(const std::vector<std::vector<Complex>>& ABCD1, const std::vector<std::vector<Complex>>& ABCD2) {
	int n = ABCD1.size() / 2;
	auto a1 = std::vector<std::vector<Complex>>(ABCD1.begin(), ABCD1.begin() + n);
	auto b1 = std::vector<std::vector<Complex>>(ABCD1.begin(), ABCD1.begin() + n);
	auto c1 = std::vector<std::vector<Complex>>(ABCD1.begin() + n, ABCD1.end());
	auto d1 = std::vector<std::vector<Complex>>(ABCD1.begin() + n, ABCD1.end());
	auto a2 = std::vector<std::vector<Complex>>(ABCD2.begin(), ABCD2.begin() + n);
	auto b2 = std::vector<std::vector<Complex>>(ABCD2.begin(), ABCD2.begin() + n);
	auto c2 = std::vector<std::vector<Complex>>(ABCD2.begin() + n, ABCD2.end());
	auto d2 = std::vector<std::vector<Complex>>(ABCD2.begin() + n, ABCD2.end());

	std::vector<std::vector<Complex>> a, b, c, d;
	if (n == 1) {
		a = { {(b2[0][0] * a1[0][0] + b1[0][0] * a2[0][0]) / (b1[0][0] + b2[0][0])} };
		b = { {b1[0][0] * b2[0][0] / (b1[0][0] + b2[0][0])} };
		c = { {c1[0][0] + c2[0][0] + (d2[0][0] - d1[0][0]) * (a1[0][0] - a2[0][0]) / (b1[0][0] + b2[0][0])} };
		d = { {d1[0][0] + (d2[0][0] - d1[0][0]) * b1[0][0] / (b1[0][0] + b2[0][0])} };
	}
	else {
		// To be continued ...
		// ...
	}

	return {{a[0], b[0]}, {c[0], d[0]}};
}*/