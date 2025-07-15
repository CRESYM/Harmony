#include <gtest/gtest.h>
#include "Load.h"


class TestLoad : public testing::Test {};

// Test cases for Load class
/*
std::vector<double> vec = { 10,1e-3,1e-6 };
Load* load = new Load("l1", 1, vec);
load->writeFile(10, 100000, 1000);
*/


// Test cases for Load constructor
TEST_F(TestLoad, TestConstructor) {
    // Optional: Let GTest capture Harmony's output to std::cerr so
    // it is not displayed in the test log
    testing::internal::CaptureStderr();

    // Test excepction for invalid number of pins
    // Pins must be > 0
    EXPECT_THROW(Load("l", 0, { 1, 2, 3 }), std::invalid_argument);
    //EXPECT_THROW(Load("1", -5, { 1, 2, 3 }), std::invalid_argument);

    // Test exception for invalid number of values
    // size(values) = num pins, or size(values) = 3 * num pins
    EXPECT_THROW(Load("l", 1, { 1, 2, 3, 4 }), std::invalid_argument);
    EXPECT_THROW(Load("l", 1, {}), std::invalid_argument);
    EXPECT_NO_THROW(Load("l", 2, { 1, 2, 3, 4, 5, 6 }));
    
    // Test cerr log msgs for initializing load parameters
    Load l1("l", 1, { 0, 0, 0 });
    std::string expected2 = "Load parameters not initialized correctly for phase 1";
    // Stop GTest capturing Harmony's output to std::cerr; save output to str
    std::string cerroutput = testing::internal::GetCapturedStderr();
    // Check cerroutput contains the expected msgs
    EXPECT_FALSE(cerroutput.find(expected2) == std::string::npos);
}

// Test cases for YMatrix
TEST_F(TestLoad, TestYMatrix) {
    // Optional: Let GTest capture Harmony's output to std::cerr so
    // it is not displayed in the test log
    testing::internal::CaptureStderr();

    // Case1
    Load l1("l1", 1, { 0, 0, 1 });
    MatrixXcd y1 = l1.compute_y_parameters_num(10000);
    MatrixXcd y1expected(2, 2);
    y1expected(0, 0) = std::complex<double>(6.12323e-13, 10000);
    y1expected(0, 1) = std::complex<double>(-6.12323e-13, -10000);
    y1expected(1, 0) = std::complex<double>(-6.12323e-13, -10000);
    y1expected(1, 1) = std::complex<double>(6.12323e-13, 10000);
    EXPECT_TRUE(y1.isApprox(y1expected, 1e-9));

    // Case2
    Load l2("l2", 3, { 1, 1, 1 });
    MatrixXcd y2 = l2.compute_y_parameters_num(1500);
    Eigen::MatrixXcd y2expected = Eigen::MatrixXcd::Zero(6, 6);
    std::complex<double> value(4.44445e-07, -0.000666667);
    y2expected.diagonal().setConstant(value);
    y2expected(0, 3) = -value;
    y2expected(1, 4) = -value;
    y2expected(2, 5) = -value;
    y2expected(3, 0) = -value;
    y2expected(4, 1) = -value;
    y2expected(5, 2) = -value;
    EXPECT_TRUE(y2.isApprox(y2expected, 1e-5));

    // Stop GTest capturing Harmony's output to std::cerr
    testing::internal::GetCapturedStderr();
}

// Test cases for getResistance
TEST_F(TestLoad, TestResistance) {
    // Optional: Let GTest capture Harmony's output to std::cerr so
    // it is not displayed in the test log
    testing::internal::CaptureStderr();

    // Case 1
    Load l1("l1", 1, { 7, 0, 1 });
    EXPECT_DOUBLE_EQ(l1.getResistance(0), 7);
    EXPECT_THROW(l1.getResistance(1), std::out_of_range);

    // Case 2
    Load l2("l2", 3, { 7, 8, 9 });
    EXPECT_DOUBLE_EQ(l2.getResistance(0), 7);
    EXPECT_DOUBLE_EQ(l2.getResistance(1), 7);
    EXPECT_DOUBLE_EQ(l2.getResistance(2), 7);
    EXPECT_THROW(l2.getResistance(3), std::out_of_range);

    // Phase cannot be negative
    EXPECT_THROW(l2.getResistance(-1), std::out_of_range);

    // Stop GTest capturing Harmony's output to std::cerr
    testing::internal::GetCapturedStderr();
}

// Test cases for getInductance
TEST_F(TestLoad, TestInductance) {
    // Optional: Let GTest capture Harmony's output to std::cerr so
    // it is not displayed in the test log
    testing::internal::CaptureStderr();

    // Case 1
    Load l1("l1", 1, { 1, 7, 1 });
    EXPECT_DOUBLE_EQ(l1.getInductance(0), 7);
    EXPECT_THROW(l1.getInductance(1), std::out_of_range);

    // Case 2
    Load l2("l2", 3, { 7, 8, 9 });
    EXPECT_DOUBLE_EQ(l2.getInductance(0), 8);
    EXPECT_DOUBLE_EQ(l2.getInductance(1), 8);
    EXPECT_DOUBLE_EQ(l2.getInductance(2), 8);
    EXPECT_THROW(l2.getInductance(3), std::out_of_range);

    // Phase cannot be negative
    EXPECT_THROW(l2.getInductance(-1), std::out_of_range);

    // Stop GTest capturing Harmony's output to std::cerr
    testing::internal::GetCapturedStderr();
}

// Test cases for getCapacitance
TEST_F(TestLoad, TestCapacitance) {
    // Optional: Let GTest capture Harmony's output to std::cerr so
    // it is not displayed in the test log
    testing::internal::CaptureStderr();

    // Case 1
    Load l1("l1", 1, { 1, 1, 7 });
    EXPECT_DOUBLE_EQ(l1.getCapacitance(0), 7);
    EXPECT_THROW(l1.getCapacitance(1), std::out_of_range);

    // Case 2
    Load l2("l2", 3, { 7, 8, 9 });
    EXPECT_DOUBLE_EQ(l2.getCapacitance(0), 9);
    EXPECT_DOUBLE_EQ(l2.getCapacitance(1), 9);
    EXPECT_DOUBLE_EQ(l2.getCapacitance(2), 9);
    EXPECT_THROW(l2.getCapacitance(3), std::out_of_range);

    // Phase cannot be negative
    EXPECT_THROW(l2.getCapacitance(-1), std::out_of_range);

    // Stop GTest capturing Harmony's output to std::cerr
    testing::internal::GetCapturedStderr();
}