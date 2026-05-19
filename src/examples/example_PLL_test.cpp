#include "Examples.h"
#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"

void example_PLL_test() {
    std::cout << "=== example_PLL_test ===\n";

    // -------- PI parameters (Simulink PID without D term) --------
    const double Kp = 180.0;
    const double Ki = 3200.0;
    const int    n = 1;
    const double ref = 0.0;
	 // Kp, Ki, n, ref (for Simulink PID block reference)
    ProportionalIntegralController pll("pll", { Kp, Ki }, n, { ref });

    // -------- Grid voltage: synchronous-frame phasor (constant) --------
    const double f = 50.0;
    const double omega_0 = 2.0 * M_PI * f;
    const double Vgd_grid = 1.25* 3.6779e-06;
    const double Vgq_grid = 1* 3.6779e-06;

    // -------- Current input: synchronous-frame phasor (constant) -------- // <-- ADDED
    // Replace these with whatever Idq you want to feed in.
    const double Igd_grid = 1.25* 0.00040784;             // <-- ADDED (placeholder)
    const double Igq_grid = 1* 0.00040784;             // <-- ADDED (placeholder)

    // -------- Simulation settings --------
    const double dt = 2e-5;
    const double Tend = 0.5;
    const int    N = static_cast<int>(Tend / dt);

    // -------- States --------
    double x_pi = 0.0;
    double theta_c = 0.0;                    // = wt_PLL, integrator output
    double delta_omega = 0.0, omega_c = 0.0;

    // Layer 1: first correct_dq (inside the PLL feedback loop)
    double Vd_corr = 0.0, Vq_corr = 0.0;

    // Layer 2: second correct_dq (downstream Vdq output)
    double Vd_out = 0.0, Vq_out = 0.0;

    // Layer 3: third correct_dq (Idq rotated by the PLL angle)            // <-- ADDED
    double Id_out = 0.0, Iq_out = 0.0;                                     // <-- ADDED

    std::ofstream out("pll_output.csv");
    out << "t,Vgd_grid,Vgq_grid,Igd_grid,Igq_grid,"
        "Vd_corr,Vq_corr,"           // layer 1 (inside PLL)
        "Vd_out,Vq_out,"             // layer 2 (downstream Vdq)
        "Id_out,Iq_out,"             // layer 3 (downstream Idq)        // <-- ADDED
        "wt_ref,wt_PLL,wt_dev,"
        "delta_omega,omega_c,Freq_Hz\n";
    out << std::setprecision(10);

    for (int k = 0; k <= N; ++k) {
        double t = k * dt;

        // ===============================================================
        //  Common reference angle: wt_ref = omega_0 * t
        //  Used by ALL THREE correct_dq blocks
        // ===============================================================
        double wt_ref = omega_0 * t;
        double wt_PLL = theta_c;                  // integrator output
        double wt_dev = wt_PLL - wt_ref;          // angle into all correct_dq blocks

        double cw = std::cos(wt_dev);
        double sw = std::sin(wt_dev);

        // ===============================================================
        //  LAYER 1: First correct_dq, inside the PLL feedback
        //  Drives Vq_corr -> 0 via the PI
        // ===============================================================
        Vd_corr = Vgd_grid * cw + Vgq_grid * sw;
        Vq_corr = -Vgd_grid * sw + Vgq_grid * cw;

        Eigen::VectorXd out_pi = pll.define_equations(x_pi, Vq_corr, 0.0);
        double dxpi_dt = out_pi(0);
        delta_omega = out_pi(1);
        omega_c = omega_0 + delta_omega;

        // ===============================================================
        //  LAYER 2: Second correct_dq, downstream Vdq output
        //  Same Vdq input, same rotation angle.
        // ===============================================================
        Vd_out = Vgd_grid * cw + Vgq_grid * sw;
        Vq_out = -Vgd_grid * sw + Vgq_grid * cw;

        // ===============================================================
        //  LAYER 3: Third correct_dq, Idq rotated by the PLL angle        // <-- ADDED
        //  Same rotation matrix, different input (Igd_grid, Igq_grid).
        //  Output (Id_out, Iq_out) is the current expressed in the
        //  PLL-aligned reference frame for downstream controllers.
        // ===============================================================
        Id_out = Igd_grid * cw + Igq_grid * sw;                           // <-- ADDED
        Iq_out = -Igd_grid * sw + Igq_grid * cw;                           // <-- ADDED

        // ===============================================================
        //  Log
        // ===============================================================
        double Freq_Hz = omega_c / (2.0 * M_PI);
        out << t << ','
            << Vgd_grid << ',' << Vgq_grid << ','
            << Igd_grid << ',' << Igq_grid << ','
            << Vd_corr << ',' << Vq_corr << ','
            << Vd_out << ',' << Vq_out << ','
            << Id_out << ',' << Iq_out << ','                            // <-- ADDED
            << wt_ref << ',' << wt_PLL << ',' << wt_dev << ','
            << delta_omega << ',' << omega_c << ',' << Freq_Hz << '\n';

        // ===============================================================
        //  Integrate states (forward Euler)
        // ===============================================================
        x_pi += dt * dxpi_dt;
        theta_c += dt * omega_c;

        while (theta_c >= 2.0 * M_PI) theta_c -= 2.0 * M_PI;
        while (theta_c < 0.0)        theta_c += 2.0 * M_PI;
    }
    out.close();

    // -------- Final values --------
    std::cout << std::setprecision(10);
    std::cout << "Wrote pll_output.csv (" << N + 1 << " rows)\n\n";

    std::cout << "--- Layer 1 (inside PLL feedback) ---\n";
    std::cout << "  Final Vd_corr     = " << Vd_corr << "\n";
    std::cout << "  Final Vq_corr     = " << Vq_corr << "   (should be ~0)\n\n";

    std::cout << "--- Layer 2 (downstream Vdq output) ---\n";
    std::cout << "  Final Vd_out      = " << Vd_out << "\n";
    std::cout << "  Final Vq_out      = " << Vq_out << "\n\n";

    std::cout << "--- Layer 3 (downstream Idq output) ---\n";                 // <-- ADDED
    std::cout << "  Final Id_out      = " << Id_out << "\n";
    std::cout << "  Final Iq_out      = " << Iq_out << "\n\n";

    std::cout << "--- PLL outputs ---\n";
    std::cout << "  Final delta_omega = " << delta_omega << "   (should be ~0)\n";
    std::cout << "  Final omega_c     = " << omega_c << "   (should be ~" << omega_0 << ")\n";
    std::cout << "  Final Freq (Hz)   = " << omega_c / (2.0 * M_PI) << "   (should be ~50)\n";
    std::cout << "  Final wt_PLL      = " << theta_c << "\n";
    std::cout << "  Final wt_dev      = " << theta_c - omega_0 * Tend
        << "   (should be ~ atan2(Vgq,Vgd) = "
        << std::atan2(Vgq_grid, Vgd_grid) << ")\n";

    std::cout << "\nPress Enter to continue...\n";
    std::cin.get();
}