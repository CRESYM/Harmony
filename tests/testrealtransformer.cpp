#include <gtest/gtest.h>
#include "Transformer_real.h"
#include "Transformer_Y_Y_real.h"

#include "Constants.h"

class TestTransformerReal : public testing::Test {};

// Test constructor 
TEST_F(TestTransformerReal, TestConstructor) {
	// Create Transformer object with 1 pin
	std::vector<double> transformer_values = { 4.3218, 0.45856, 0.7938, 0.084225, 1.0804e+06, 2e-3, 2.0, 0.0 }; // R_primary, L_primary, R_secondary, L_secondary, Turns Ratio
	Transformer_real* transformer = new Transformer_real("T1", "AC1", 1, transformer_values);
	
	EXPECT_EQ(transformer->getTurnsRatio(), 2.0);
	EXPECT_EQ(transformer->getPhaseLag(), 0.0);
	EXPECT_EQ(transformer->getResistance(0), 4.3218);
	EXPECT_EQ(transformer->getInductance(0), 0.45856);
	EXPECT_EQ(transformer->getResistance(1), 0.7938);
	EXPECT_EQ(transformer->getInductance(1), 0.084225);
	EXPECT_EQ(transformer->getResistance(2), 1.0804e+06);
	EXPECT_EQ(transformer->getInductance(2), 2e-3);
}

//std::vector<double> transformer_values = { 4.3218, 0.0, 0.7938, 0.084225,  2.0, 0.0 };
//TransformerYY_real* transformerYY = new TransformerYY_real("T3", 3, transformer_values);
TEST_F(TestTransformerReal, TestTransformerYYConstructor) {
	std::vector<double> transformer_values = { 4.3218, 0.0, 0.7938, 0.084225, 2.0, 0.0 };
	TransformerYY_real* transformerYY = new TransformerYY_real("T3", "AC1", 3, transformer_values);
	
	EXPECT_EQ(transformerYY->getTurnsRatio(), 2.0);
	EXPECT_EQ(transformerYY->getPhaseLag(), 0.0);
	EXPECT_EQ(transformerYY->getResistance(0), 4.3218);
	EXPECT_EQ(transformerYY->getInductance(0), 0.0);
	EXPECT_EQ(transformerYY->getResistance(1), 0.7938);
	EXPECT_EQ(transformerYY->getInductance(1), 0.084225);

	//DenseMatrix Y = transformerYY->compute_y_parameters();

	//RCP<const Basic> Z_p = real_double(4.3218);
	//RCP<const Basic> Z_s = add(real_double(0.7938), mul(j, mul(omega, real_double(0.084225))));
	//RCP<const Basic> a_val = real_double(2.0); // Turns ratio symbol

	//RCP<const Basic> Y_11 = div(one, add(Z_p, mul(a_val, mul(a_val, Z_s))));
	//RCP<const Basic> Y_12 = div(neg(a_val), add(Z_p, mul(a_val, mul(a_val, Z_s))));
	//RCP<const Basic> Y_21 = Y_12;
	//RCP<const Basic> Y_22 = div(one, add(Z_s, div(Z_p, mul(a_val, a_val))));

	//EXPECT_EQ(Y.get(0, 0), Y_11);
	//EXPECT_EQ(Y.get(0, 1), 0);	
	//EXPECT_EQ(Y.get(0, 2), 0);
	//EXPECT_EQ(Y.get(1, 0), 0);
	//EXPECT_EQ(Y.get(1, 1), Y_11);
	//EXPECT_EQ(Y.get(1, 2), 0);
	//EXPECT_EQ(Y.get(2, 0), 0);
	//EXPECT_EQ(Y.get(2, 1), 0);
	//EXPECT_EQ(Y.get(2, 2), Y_11);

	//EXPECT_EQ(Y.get(0, 3), Y_12);
	//EXPECT_EQ(Y.get(0, 4), 0);
	//EXPECT_EQ(Y.get(0, 5), 0);
	//EXPECT_EQ(Y.get(1, 3), 0);
	//EXPECT_EQ(Y.get(1, 4), Y_12);
	//EXPECT_EQ(Y.get(1, 5), 0);
	//EXPECT_EQ(Y.get(2, 3), 0);
	//EXPECT_EQ(Y.get(2, 4), 0);
	//EXPECT_EQ(Y.get(2, 5), Y_12);

	//EXPECT_EQ(Y.get(3, 0), Y_21);
	//EXPECT_EQ(Y.get(3, 1), 0);
	//EXPECT_EQ(Y.get(3, 2), 0);
	//EXPECT_EQ(Y.get(4, 0), 0);
	//EXPECT_EQ(Y.get(4, 1), Y_21);
	//EXPECT_EQ(Y.get(4, 2), 0);
	//EXPECT_EQ(Y.get(5, 0), 0);
	//EXPECT_EQ(Y.get(5, 1), 0);
	//EXPECT_EQ(Y.get(5, 2), Y_21);

	//EXPECT_EQ(Y.get(3, 3), Y_22);
	//EXPECT_EQ(Y.get(3, 4), 0);
	//EXPECT_EQ(Y.get(3, 5), 0);
	//EXPECT_EQ(Y.get(4, 3), 0);
	//EXPECT_EQ(Y.get(4, 4), Y_22);
	//EXPECT_EQ(Y.get(4, 5), 0);
	//EXPECT_EQ(Y.get(5, 3), 0);
	//EXPECT_EQ(Y.get(5, 4), 0);
	//EXPECT_EQ(Y.get(5, 5), Y_22);
}