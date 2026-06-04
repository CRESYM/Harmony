#ifndef _EXAMPLES_H_
#define _EXAMPLES_H_

extern void example_OPF(bool plotting_enabled = true);
extern void example_OPF_csv(bool plotting_enabled = true);
extern void example_OPF_1(bool plotting_enabled = true);
extern void example_OPF_csv_1(bool plotting_enabled = true);
extern void example_OPF_PV(bool plotting_enabled = true);
extern void example_OPF_WT();

extern void example_DQsym_math_operations(bool plotting_enabled = true);
extern void example_DQsym_DSSS2(bool plotting_enabled = true);
extern void example_DQsym_RLC(bool plotting_enabled = true);
extern void example_DQsym_Simple_MMC(bool plotting_enabled = true);
extern void example_DQsym_validation();

extern void example_state_space();

extern void example_generator(bool plotting_enabled = true);

extern void example_MMC(bool plotting_enabled = true);
extern void example_WT_type_3(bool plotting_enabled = true);
extern void example_WT_type_4(bool plotting_enabled = true);
extern void example_PV_plant(bool plotting_enabled = true);
extern void example_OHL(bool plotting_enabled = true);
extern void example_cable(bool plotting_enabled = true);
extern void example_transformer();

extern void example_constructors();
extern void example_visuals(bool plotting_enabled = true);
extern void example_stability_check(bool plotting_enabled = true);
extern void example_admittance_parameters();

extern void example_point2point_case();

#endif
