#include <gtest/gtest.h>
#include "Transformer_real.h"

class TestTransformerReal : public testing::Test {};

// Test constructor 
TEST_F(TestTransformerReal, TestConstructor) {
	// Create Transformer object with 1 pin
	std::vector<double> transformer_values = { 4.3218, 0.45856, 0.7938, 0.084225, 1.0804e+06, 2e-3, 2.0, 0.0 }; // R_primary, L_primary, R_secondary, L_secondary, Turns Ratio
	Transformer_real* transformer = new Transformer_real("T1", 1, transformer_values);
	
	EXPECT_EQ(transformer->getTurnsRatio(), 2.0);
	EXPECT_EQ(transformer->getPhaseLag(), 0.0);
	EXPECT_EQ(transformer->getResistance(0), 4.3218);
	EXPECT_EQ(transformer->getInductance(0), 0.45856);
	EXPECT_EQ(transformer->getResistance(1), 0.7938);
	EXPECT_EQ(transformer->getInductance(1), 0.084225);
	EXPECT_EQ(transformer->getResistance(2), 1.0804e+06);
	EXPECT_EQ(transformer->getInductance(2), 2e-3);
}
