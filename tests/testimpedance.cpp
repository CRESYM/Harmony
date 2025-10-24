#include <gtest/gtest.h>
#include "./Elements/Impedance/Impedance.h"

class TestImpedance : public testing::Test {};

// Test cases for Impedance constructor
TEST_F(TestImpedance, TestConstructor) {
    // Optional: Let GTest capture Harmony's output to std::cerr so
    // it is not displayed in the test log
    testing::internal::CaptureStderr();

    // Test excepction for invalid number of pins(
    // Pins must be > 0
    //EXPECT_THROW(Impedance("z", 0, DenseMatrix(1, 1, { div(integer(1), mul(j, omega)) })), std::invalid_argument);
    //EXPECT_THROW(Impedance("z", -1, DenseMatrix(1, 1, { div(integer(1), mul(j, omega)) })), std::invalid_argument);

    // Test exception for invalid number of values
    // values.ncols() = 1, or values.ncols() = num pins
    // case: pins = 1 & values.ncols = 2
    EXPECT_THROW(Impedance("z", "AC1", 1, DenseMatrix(1, 2, { div(integer(1), mul(j, omega)), mul(j, omega) })), std::invalid_argument);
    // case: pins = 3 & values.ncols = 2
    EXPECT_THROW(Impedance("z", "AC1", 3, DenseMatrix(1, 2, { div(integer(1), mul(j, omega)), mul(j, omega) })), std::invalid_argument);
    // case: pins = 2 & values.ncols = 2
    EXPECT_NO_THROW(Impedance("z", "AC1", 2, DenseMatrix(1, 2, { div(integer(1), mul(j, omega)), mul(j, omega) })), std::invalid_argument);

    // Stop GTest capturing Harmony's output to std::cerr
    testing::internal::GetCapturedStderr();
}

// Test cases for YMatrx
TEST_F(TestImpedance, TestYMatrix) {
    // Optional: Let GTest capture Harmony's output to std::cerr so
    // it is not displayed in the test log
    testing::internal::CaptureStderr();

    // Case 1
    Impedance z1("z1", "AC1", 1, DenseMatrix(1, 1, { div(integer(1), mul(j, omega)) })); // 1/wi
    MatrixXcd y1 = vectorToMatrix(z1.compute_y_parameters(10000));
    MatrixXcd y1expected(2, 2);
    y1expected(0, 0) = std::complex<double>(0, 10000);
    y1expected(0, 1) = std::complex<double>(0, -10000);
    y1expected(1, 0) = std::complex<double>(0, -10000);
    y1expected(1, 1) = std::complex<double>(0, 10000);
    EXPECT_TRUE(y1.isApprox(y1expected, 1e-9));

    // Case 2
    Impedance z2("z2", "AC1", 2, DenseMatrix(1, 2, { div(integer(1), mul(j, omega)), mul(integer(2), mul(j, omega))})); // 1/wi, 2*(wi)
    MatrixXcd y2 = vectorToMatrix(z2.compute_y_parameters(1500));    
    MatrixXcd y2expected(4, 4);
    std::complex<double> i1500(0, 1500);
    std::complex<double> i0(0, 0);
    std::complex<double> i33(0, 0.000333333);
    y2expected << i1500, i0, -i1500, i0,
        i0, -i33, i0, i33,
        -i1500, i0, i1500, i0,
        i0, i33, i0, -i33;
    EXPECT_TRUE(y2.isApprox(y2expected, 1e-9));
    
 
    // Stop GTest capturing Harmony's output to std::cerr
    testing::internal::GetCapturedStderr();
}

