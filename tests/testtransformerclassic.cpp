#include <gtest/gtest.h>
#include "Transformer_classic.h"

class TestTransformerClassic : public testing::Test {};

// Test constructor 
TEST_F(TestTransformerClassic, TestConstructor) {
	std::vector<double> transformer_values = { 1.0, 1.0e-3, 2.0, 2.0e-3, 0.5e-3 }; // R_primary, L_primary, R_secondary, L_secondary, Mutual inductance
	Transformer_classic transformer = Transformer_classic("T1", "AC1", 1, transformer_values);

	EXPECT_EQ(transformer.getResistance(0), 1.0);
	EXPECT_EQ(transformer.getInductance(0), 1.0e-3);
	EXPECT_EQ(transformer.getResistance(1), 2.0);
	EXPECT_EQ(transformer.getInductance(1), 2.0e-3);
	EXPECT_EQ(transformer.getMutualInductance(), 0.5e-3);
}

// Test constructor 
TEST_F(TestTransformerClassic, TestYMatrix) {
	std::vector<double> transformer_values = { 1.0, 1.0e-3, 2.0, 2.0e-3, 0.5e-3 }; // R_primary, L_primary, R_secondary, L_secondary, Mutual inductance
	Transformer_classic transformer = Transformer_classic("T1", "AC1", 1, transformer_values);
	
	auto ytmp = transformer.compute_y_parameters(1500);
	MatrixXcd y(2,2);
	y << ytmp[0][0], ytmp[0][1], ytmp[1][0], ytmp[1][1];
	MatrixXcd yexpected(2,2);
	std::complex<double> w = 2 * M_PI * 1500 * std::complex<double>(0, 1);
	std::complex<double> Zp = 1.0 + 1.0e-3 * w;
	std::complex<double> Zs = 2.0 + 2.0e-3 * w;
	std::complex<double> Zm = 0.5e-3 * w;
	std::complex<double> D = Zp*Zs - pow(Zm,2);
	yexpected << Zs/D, -Zm/D, -Zm/D, Zp/D;
	EXPECT_TRUE(y.isApprox(yexpected, 1e-9));
}