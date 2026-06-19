#ifndef _EXAMPLES_H_
#define _EXAMPLES_H_

/**
 * @file Examples.h
 * @brief Declarations for runnable demonstration programs.
 * @ingroup examples
 *
 * Enable one function via the CLI (`Harmony --cpp <name>`) or register it in `src/cli.cpp`.
 * Most functions accept @p plotting_enabled to suppress GUI output in CI.
 */

/** @brief Basic hybrid AC–DC optimal power flow case. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_OPF(bool plotting_enabled = true);

/** @brief OPF case driven by CSV network data. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_OPF_csv(bool plotting_enabled = true);

/** @brief Extended hybrid OPF demonstration. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_OPF_1(bool plotting_enabled = true);

/** @brief Variant OPF case from CSV data. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_OPF_csv_1(bool plotting_enabled = true);

/** @brief OPF study including a PV plant. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_OPF_PV(bool plotting_enabled = true);

/** @brief OPF study including a wind-turbine plant. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_OPF_WT(bool plotting_enabled = true);

/** @brief Validate core dynamic-phasor (DQsym) math operations. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_DQsym_math_operations(bool plotting_enabled = true);

/** @brief DQsym switching / DSSS demonstration. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_DQsym_DSSS2(bool plotting_enabled = true);

/** @brief Simple RLC circuit in the DQsym framework. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_DQsym_RLC(bool plotting_enabled = true);

/** @brief Simplified MMC model in the DQsym solver. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_DQsym_Simple_MMC(bool plotting_enabled = true);

/** @brief DQsym validation routines (header declaration only). @ingroup examples */
extern void example_DQsym_validation();

/** @brief Build state-space (A, B, C, D) matrices for an RLC network. @ingroup examples */
extern void example_state_space();

/** @brief Generator element demonstration. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_generator(bool plotting_enabled = true);

/** @brief MMC equilibrium, ABCD, Y-matrix, and eigenvalue study. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_MMC(bool plotting_enabled = true);

/** @brief Type-3 wind-turbine model demonstration. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_WT_type_3(bool plotting_enabled = true);

/** @brief Type-4 wind-turbine model demonstration. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_WT_type_4(bool plotting_enabled = true);

/** @brief Photovoltaic plant model demonstration. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_PV_plant(bool plotting_enabled = true);

/** @brief Overhead transmission line geometry and Y-parameter sweep. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_OHL(bool plotting_enabled = true);

/** @brief Underground cable layers and Y-parameter sweep. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_cable(bool plotting_enabled = true);

/** @brief Transformer topologies and Y-parameters. @ingroup examples */
extern void example_transformer();

/** @brief Instantiate basic element types via constructors. @ingroup examples */
extern void example_constructors();

/** @brief Plotting and visualization utilities. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_visuals(bool plotting_enabled = true);

/** @brief Full AC–DC hybrid stability study. @param plotting_enabled Enable result plots. @ingroup examples */
extern void example_stability_check(bool plotting_enabled = true);

/** @brief Equivalent admittance parameter utilities. @ingroup examples */
extern void example_admittance_parameters();

/** @brief Point-to-point HVDC-style OPF case. @ingroup examples */
extern void example_point2point_case();

#endif
