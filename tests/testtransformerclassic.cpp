#include <gtest/gtest.h>
#include "Transformer_classic.h"

class TestTransformerClassic : public testing::Test {};

// Test constructor 
TEST_F(TestTransformerClassic, TestConstructor) {
	std::vector<double> transformer_values = { 1.0, 1.0e-3, 2.0, 2.0e-3, 0.5e-3 }; // R_primary, L_primary, R_secondary, L_secondary, Mutual inductance
	Transformer_classic* transformer = new Transformer_classic("T1", 1, transformer_values);

	EXPECT_EQ(transformer->getResistance(0), 1.0);
	EXPECT_EQ(transformer->getInductance(0), 1.0e-3);
	EXPECT_EQ(transformer->getResistance(1), 2.0);
	EXPECT_EQ(transformer->getInductance(1), 2.0e-3);
	EXPECT_EQ(transformer->getMutualInductance(), 0.5e-3);
}

// Test constructor 
TEST_F(TestTransformerClassic, TestYMatrix) {
	std::vector<double> transformer_values = { 1.0, 1.0e-3, 2.0, 2.0e-3, 0.5e-3 }; // R_primary, L_primary, R_secondary, L_secondary, Mutual inductance
	Transformer_classic* transformer = new Transformer_classic("T1", 1, transformer_values);
	
	MatrixXcd y = transformer->compute_y_parameters_num(1500);
	MatrixXcd yexpected(2,2);
	std::complex<double> Zp = 1.0 + std::complex<double>(0, 1.0e-3 * 1500);
	std::complex<double> Zs = 2.0 + std::complex<double>(0, 2.0e-3 * 1500);
	std::complex<double> Zm = std::complex<double>(0, 0.5e-3 * 1500);
	std::complex<double> D = Zp*Zs - pow(M,2);
	yexpected << Zs/D, -Zm/D, -Zm/D, Zp/D;
	EXPECT_TRUE(y.isApprox(yexpected, 1e-9));
}