#include <gtest/gtest.h>
#include "Load.h"
#include "Bus.h"
#include "utils.h"

class TestElement : public testing::Test {};

// Test cases for getInputPins
TEST_F(TestElement, TestInputPins) {
    // Optional: Let GTest capture Harmony's output to std::cerr so
    // it is not displayed in the test log
    testing::internal::CaptureStderr();

    // Case 1
    Load l1("l1", 1, { 1, 1, 7 });
    EXPECT_EQ(l1.getInputPins(), 1);

    // Case 2
    Load l2("l2", 3, { 7, 8, 9 });
    EXPECT_EQ(l2.getInputPins(), 3);

    // Stop GTest capturing Harmony's output to std::cerr
    testing::internal::GetCapturedStderr();
}

// Test cases for getOutputPins
TEST_F(TestElement, TestOutputPins) {
    // Optional: Let GTest capture Harmony's output to std::cerr so
    // it is not displayed in the test log
    testing::internal::CaptureStderr();

    // Case 1
    Load l1("l1", 1, { 1, 1, 7 });
    EXPECT_EQ(l1.getOutputPins(), 1);

    // Case 2
    Load l2("l2", 3, { 7, 8, 9 });
    EXPECT_EQ(l2.getOutputPins(), 3);

    // Stop GTest capturing Harmony's output to std::cerr
    testing::internal::GetCapturedStderr();
}

// Test cases for getElementSymbol
TEST_F(TestElement, TestElementSymbol) {
    // Optional: Let GTest capture Harmony's output to std::cerr so
    // it is not displayed in the test log
    testing::internal::CaptureStderr();

    // Case 1
    Load l1("l1", 1, { 1, 1, 7 });
    EXPECT_EQ(l1.getElementSymbol(), "l1");

    // Case 2
    Load l2("l2", 3, { 7, 8, 9 });
    EXPECT_EQ(l2.getElementSymbol(), "l2");

    // Stop GTest capturing Harmony's output to std::cerr
    testing::internal::GetCapturedStderr();
}

// Test cases for printElementInfo
TEST_F(TestElement, TestPrintElementInfo) {
    // Optional: Let GTest capture Harmony's output to std::cerr so
    // it is not displayed in the test log
    testing::internal::CaptureStderr();
    // Capture the output of std::cout to check for expected msgs
    testing::internal::CaptureStdout();

    // Case 1
    Load l1("l1", 1, { 1, 1, 7 });
    l1.printElementInfo();
    std::string expected1 = "Element Symbol: l1, Input Pins: 1, Output Pins: 1";
    
    // Case 2
    Load l2("l2", 3, { 7, 8, 9 });
    l2.printElementInfo();
    std::string expected2 = "Element Symbol: l2, Input Pins: 3, Output Pins: 3";
   
    // Stop GTest capturing Harmony's output to std::cout; save output to str    
    std::string coutoutput = testing::internal::GetCapturedStdout();

    // Check coutoutput contains the expected msgs
    EXPECT_FALSE(coutoutput.find(expected1) == std::string::npos);
    EXPECT_FALSE(coutoutput.find(expected2) == std::string::npos);

    // Stop GTest capturing Harmony's output to std::cerr
    testing::internal::GetCapturedStderr();
}

// Test cases for printElementValues
TEST_F(TestElement, TestPrintElementValues) {
    // Optional: Let GTest capture Harmony's output to std::cerr so
    // it is not displayed in the test log
    testing::internal::CaptureStderr();
    // Capture the output of std::cout to check for expected msgs
    testing::internal::CaptureStdout();

    // Case 1
    Load l1("l1", 1, { 1, 1, 1 });
    l1.printElementValues();
    std::string expected = "Element : l1\n";
    expected += "Y matrix symbolic entries: \n";
    expected += "1.0/(1.0 + (0.0 + 1.0*I)*w + (-0.0 - 1.0*I)*w**(-1)) -1.0/(1.0 + (0.0 + 1.0*I)*w + (-0.0 - 1.0*I)*w**(-1)) \n";
    expected += "-1.0/(1.0 + (0.0 + 1.0*I)*w + (-0.0 - 1.0*I)*w**(-1)) 1.0/(1.0 + (0.0 + 1.0*I)*w + (-0.0 - 1.0*I)*w**(-1)) \n";

    // Stop GTest capturing Harmony's output to std::cout; save output to str    
    std::string coutoutput = testing::internal::GetCapturedStdout();

    // Check coutoutput contains the expected msgs
    EXPECT_FALSE(coutoutput.find(expected) == std::string::npos);
    
    // Stop GTest capturing Harmony's output to std::cerr
    testing::internal::GetCapturedStderr();
}

// Test cases for writeFile
TEST_F(TestElement, TestWriteFile) {
    // Optional: Let GTest capture Harmony's output to std::cerr so
    // it is not displayed in the test log
    testing::internal::CaptureStderr();

    // Case 1
    Load l1("l1", 1, { 1, 1, 1 });
    l1.writeFile(1000, 2000, 100);

    // Read data
    std::string expected = readFile("../data/l1.csv");

    // Check file was created in the exepcted path and with the expected name
    std::string actual;
    try {
        actual = readFile("files/l1.csv");
        if (actual.empty()) {
            throw std::runtime_error("File is empty");
        }
    }
    catch (std::runtime_error err) {
        FAIL() << "Element::writeFile did not produce the expected file";
    }

    // Check the file contains the expected content
    EXPECT_EQ(actual, expected);


    // Stop GTest capturing Harmony's output to std::cerr
    testing::internal::GetCapturedStderr();
}

// Test cases for compute_y_parameters
TEST_F(TestElement, TestComputeYParameters) {
    // Optional: Let GTest capture Harmony's output to std::cerr so
 // it is not displayed in the test log
    testing::internal::CaptureStderr();

    // Case 1
    Load l1("l1", 1, { 1, 1, 1 });
    std::vector<std::vector<complex<double>>> actual = l1.compute_y_parameters(1000);

    // Expected value
    std::vector<std::vector<complex<double>>> expected = {
        { {2.5330296552224018e-08, -0.0001591549430918953}, {-2.5330296552224018e-08, 0.0001591549430918953} },
        { {-2.5330296552224018e-08, 0.0001591549430918953}, {2.5330296552224018e-08, -0.0001591549430918953} }
    };

    // Check values match
    for (unsigned int i = 0; i < actual.size(); i++) {
        for (unsigned int j = 0; j < actual[i].size(); j++) {
            EXPECT_DOUBLE_EQ(actual[i][j].real(), expected[i][j].real());
            EXPECT_DOUBLE_EQ(actual[i][j].imag(), expected[i][j].imag());
        }
    }

    // Stop GTest capturing Harmony's output to std::cerr
    testing::internal::GetCapturedStderr();
}

// Test cases for attachBus and getConnections
TEST_F(TestElement, TestBusConnections) {
    // Optional: Let GTest capture Harmony's output to std::cerr so
    // it is not displayed in the test log
    testing::internal::CaptureStderr();
    testing::internal::CaptureStdout();

    // Create Bus objects
    Bus* bus1 = new Bus("bus1", 1);
    Bus* bus2 = new Bus("bus2", 7);
    
    Load l1("l1", 1, { 1, 1, 1 });
    l1.attachBus(bus1, 1);
    l1.attachBus(bus2, 7);
   
    std::map<Bus*, int> connections = l1.getConnections();

    // connections map has 2 elements
    EXPECT_EQ(connections.size(), 2);
    
    // look for bus1 in connections
    EXPECT_NE(connections.find(bus1), connections.end());
    EXPECT_EQ(connections.find(bus1)->second, 1);
    
    // look for bus2 in connections
    EXPECT_NE(connections.find(bus2), connections.end());
    EXPECT_EQ(connections.find(bus2)->second, 7);

    // Cleanup
    delete bus1;
    delete bus2;

    // Stop GTest capturing Harmony's output to std::cerr
    testing::internal::GetCapturedStderr();
    testing::internal::GetCapturedStdout();
}

// Test cases for getBuses
TEST_F(TestElement, TestGetBuses) {
    // Optional: Let GTest capture Harmony's output to std::cerr so
    // it is not displayed in the test log
    testing::internal::CaptureStderr();
    testing::internal::CaptureStdout();

    // Create Bus objects
    Bus* bus1 = new Bus("bus1", 1);
    Bus* bus2 = new Bus("bus2", 7);

    Load l1("l1", 1, { 1, 1, 1 });
    l1.attachBus(bus1, 1);
    l1.attachBus(bus2, 7);
    std::vector<Bus*> actual = l1.getBuses();

    EXPECT_EQ(actual.size(), 2);
    // In C++ std::map sorts its elements based on the key.
    // Since Bus* is a pointer type, the sorting is done based on the memory
    // addresses of the Bus objects. This means the order of the elements of the
    // 'actual' vector is not fixed.
    if (actual[0] == bus1) {
        EXPECT_EQ(actual[1], bus2);
    }
    else if (actual[0] == bus2) {
        EXPECT_EQ(actual[1], bus1);
    }
    else {
        FAIL() << "Element::getBuses did not return the expected value";
    }

    delete bus1;
    delete bus2;

    // Stop GTest capturing Harmony's output to std::cerr
    testing::internal::GetCapturedStderr();
    testing::internal::GetCapturedStdout();
}

// Test cases for getOtherBus
TEST_F(TestElement, TestGetOtherBuses) {
    // Optional: Let GTest capture Harmony's output to std::cerr so
    // it is not displayed in the test log
    testing::internal::CaptureStderr();
    testing::internal::CaptureStdout();

    // Create Bus objects
    Bus* bus1 = new Bus("bus1", 1);
    Bus* bus2 = new Bus("bus2", 7);

    Load l1("l1", 1, { 1, 1, 1 });
    l1.attachBus(bus1, 1);
    l1.attachBus(bus2, 7);

    EXPECT_EQ(l1.getOtherBus(bus1), bus2);
    EXPECT_EQ(l1.getOtherBus(bus2), bus1);

    delete bus1;
    delete bus2;

    // Stop GTest capturing Harmony's output to std::cerr
    testing::internal::GetCapturedStderr();
    testing::internal::GetCapturedStdout();
}
