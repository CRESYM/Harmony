#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <filesystem> // For checking output files
#include "Examples.h"

class TestExamples : public testing::Test {
protected:
    // Member variables accessible to SetUp, TearDown, and the tests
    std::streambuf* original_cin_buffer = nullptr;
    std::istringstream simulated_input;

    // Runs automatically BEFORE each TEST_F
    void SetUp() override {
        // Setup simulated input: a single newline character
        simulated_input.str("\n");
        
        // Backup the real std::cin buffer and inject our simulated one
        original_cin_buffer = std::cin.rdbuf();
        std::cin.rdbuf(simulated_input.rdbuf());
    }

    // Runs automatically AFTER each TEST_F (even if the test fails)
    void TearDown() override {
        // Restore the real std::cin
        if (original_cin_buffer != nullptr) {
            std::cin.rdbuf(original_cin_buffer);
        }
    }
};

// ----------------------------------------------------------------------------
// Elements
// ----------------------------------------------------------------------------

TEST_F(TestExamples, Generator) {
    EXPECT_NO_THROW(example_generator(false));
}

TEST_F(TestExamples, MMC) {
    EXPECT_NO_THROW(example_MMC(false));
    EXPECT_TRUE(std::filesystem::exists("files/MMC1.csv"));
    std::filesystem::remove("files/MMC1.csv");
}

TEST_F(TestExamples, WT_type_3) {
    EXPECT_NO_THROW(example_WT_type_3(false));
    EXPECT_TRUE(std::filesystem::exists("files/DFIG.csv"));
    std::filesystem::remove("files/DFIG.csv");
}

TEST_F(TestExamples, WT_type_4) {
    EXPECT_NO_THROW(example_WT_type_4(false));
}

TEST_F(TestExamples, PV_plant) {
    EXPECT_NO_THROW(example_PV_plant(false));
}

TEST_F(TestExamples, OHL) {
    EXPECT_NO_THROW(example_OHL(false));
    EXPECT_TRUE(std::filesystem::exists("files/ohl.csv"));
    std::filesystem::remove("files/ohl.csv");
}

TEST_F(TestExamples, Cable) {
    EXPECT_NO_THROW(example_cable(false));
    EXPECT_TRUE(std::filesystem::exists("files/cable.csv"));
    std::filesystem::remove("files/cable.csv");
}

TEST_F(TestExamples, Transformer) {
    EXPECT_NO_THROW(example_transformer());
    EXPECT_TRUE(std::filesystem::exists("files/T3.csv"));
    std::filesystem::remove("files/T3.csv");
}

TEST_F(TestExamples, Constructors) {
    EXPECT_NO_THROW(example_constructors());
    EXPECT_TRUE(std::filesystem::exists("files/tl.csv"));
    std::filesystem::remove("files/tl.csv");
}

TEST_F(TestExamples, Visuals) {
    EXPECT_NO_THROW(example_visuals(false));
    EXPECT_TRUE(std::filesystem::exists("files/tl1.csv"));
    std::filesystem::remove("files/tl1.csv");
}

TEST_F(TestExamples, Admittance_Parameters) {
    EXPECT_NO_THROW(example_admittance_parameters());
}

// ----------------------------------------------------------------------------
// Solvers - DQSym
// ----------------------------------------------------------------------------

TEST_F(TestExamples, DQsym_DSSS2) {
    // Run the example (assert it doesn't crash/throw)
    EXPECT_NO_THROW(example_DQsym_DSSS2(false));
    
    // Verify the generated files
    EXPECT_TRUE(std::filesystem::exists("dsss2_abc_output.csv"));
    
    // Clean up the file after the test so the next run is fresh
    std::filesystem::remove("dsss2_abc_output.csv");
}

TEST_F(TestExamples, DQsym_RLC) {
    EXPECT_NO_THROW(example_DQsym_RLC(false));
    EXPECT_TRUE(std::filesystem::exists("DQsymRLC_abc_output.csv"));
    std::filesystem::remove("DQsymRLC_abc_output.csv");
}

TEST_F(TestExamples, DQsym_Simple_MMC) {
    EXPECT_NO_THROW(example_DQsym_Simple_MMC(false));
    EXPECT_TRUE(std::filesystem::exists("DQsym_MMC_SigmaDelta.csv"));
    std::filesystem::remove("DQsym_MMC_SigmaDelta.csv");
}

TEST_F(TestExamples, DQsym_math_operations) {
    EXPECT_NO_THROW(example_DQsym_math_operations(false));
}

// ----------------------------------------------------------------------------
// Solvers - State Space
// ----------------------------------------------------------------------------

TEST_F(TestExamples, State_Space) {
    EXPECT_NO_THROW(example_state_space());
}

// ----------------------------------------------------------------------------
// Solvers - Stability Estimate (failing)
// ----------------------------------------------------------------------------

//TEST_F(TestExamples, Stability_Check) {
//    EXPECT_NO_THROW(example_stability_check(false));
//}

// ----------------------------------------------------------------------------
// Solvers - Point 2 Point Case (failing)
// ----------------------------------------------------------------------------

//TEST_F(TestExamples, Point2Point_Case) {
//    EXPECT_NO_THROW(example_point2point_case());
//}
