#include <gtest/gtest.h>
#include "Admittance.h"

class TestAdmittance : public testing::Test {};

// Test constructor 
TEST_F(TestAdmittance, TestConstructor) {
	Admittance* y = new Admittance("y1", "AC1", 1, DenseMatrix(1, 1, { mul(j, omega) }));

    EXPECT_EQ(y->getInputPins(), 1);
    EXPECT_EQ(y->getOutputPins(), 1);
	EXPECT_EQ(y->getElementSymbol(), "y1");
}

// Test cases for YMatrx
TEST_F(TestAdmittance, TestYMatrix) {
    // Optional: Let GTest capture Harmony's output to std::cerr so
    // it is not displayed in the test log
    testing::internal::CaptureStderr();

    // Case 1
    Admittance y("y1", "AC1", 1, DenseMatrix(1, 1, { mul(j, omega) }));
    MatrixXcd y1 = vectorToMatrix(y.compute_y_parameters(1000));
    MatrixXcd y1expected(2, 2);
    y1expected(0, 0) = std::complex<double>(0, 1000);
    y1expected(0, 1) = std::complex<double>(0, -1000);
    y1expected(1, 0) = std::complex<double>(0, -1000);
    y1expected(1, 1) = std::complex<double>(0, 1000);
    EXPECT_TRUE(y1.isApprox(y1expected, 1e-9));

    
    // Stop GTest capturing Harmony's output to std::cerr
    testing::internal::GetCapturedStderr();
}