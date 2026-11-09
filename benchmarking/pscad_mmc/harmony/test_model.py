"""Physical and controller invariants for the corrected benchmark model."""
import unittest
import numpy as np
from model_harmony_mmc import HarmonyMmcGfl


class ModelChecks(unittest.TestCase):
    def test_cell_and_arm_resistance_are_distinct(self):
        model = HarmonyMmcGfl()
        self.assertAlmostEqual(model.Rarm, 53.5)
        self.assertAlmostEqual(model.Reqac, 27.285)
        self.assertAlmostEqual(HarmonyMmcGfl(Rarm=1.07).Rarm, 1.07)

    def test_equilibrium_power_balance_and_stability(self):
        model = HarmonyMmcGfl()
        model.solve_equilibrium()
        model.compute_abcd()
        self.assertLess(model.equilibrium_residual, 1e-7)
        self.assertLess(np.linalg.eigvals(model.A).real.max(), 1e-6)
        dc_mw = -3 * model.x_eq[14] * model.Vdc / 1e6
        self.assertLess(abs(dc_mw - 78.00), 0.1)

    def test_auto_freeze_changes_branch(self):
        model = HarmonyMmcGfl(Pac=90e6)
        x = model.solve_equilibrium().copy()
        x[2] -= 0.01  # Nonzero active-power error.
        x[6] = 0.8
        self.assertGreater(model.derivatives(x, model.u0)[6], 0)
        x[6] = 1.01
        self.assertEqual(model.derivatives(x, model.u0)[6], 0)
        model.outer_mode = "unfrozen"
        self.assertGreater(model.derivatives(x, model.u0)[6], 0)

    def test_inner_integrator_stops_at_output_limit(self):
        model = HarmonyMmcGfl(Pac=90e6)
        x = model.solve_equilibrium().copy()
        x[8] = 2
        self.assertEqual(model.derivatives(x, model.u0)[8], 0)

    def test_frozen_modulation_removes_proportional_feedback(self):
        model = HarmonyMmcGfl(Pac=90e6)
        model.solve_equilibrium()
        model.modulation_mode = "frozen"
        a, _, _, _ = model.compute_abcd()
        self.assertTrue(np.allclose(a[:12], 0))
        self.assertTrue(np.allclose(a[12:, :12], 0))
        # Fully frozen plant must remain dynamically coupled to terminal voltage.
        self.assertGreater(abs(model.compute_y(100)[1, 1]), 0)

    def test_failed_equilibrium_is_rejected(self):
        model = HarmonyMmcGfl()
        model.derivatives = lambda x, u, for_equilibrium=False: np.ones(24)
        with self.assertRaises(RuntimeError):
            model.solve_equilibrium()

    def test_batch_dynamics_match_individual_models(self):
        model = HarmonyMmcGfl(Pac=90e6)
        x = model.solve_equilibrium()
        states = np.repeat(x[:, None], 3, axis=1)
        states[6] = [0.8, 1.01, 2]
        states[8, 2] = 2
        states[2] -= 0.01
        inputs = np.repeat(model.u0[:, None], 3, axis=1)
        batch = model.derivatives(states, inputs)
        single = np.column_stack([model.derivatives(states[:, i], inputs[:, i]) for i in range(3)])
        np.testing.assert_allclose(batch, single, atol=1e-8)

    def test_sampled_transient_preserves_equilibrium(self):
        model = HarmonyMmcGfl(Pac=90e6)
        x = model.solve_equilibrium().copy()
        _, states, _ = model.simulate(.02, lambda t: model.u0)
        np.testing.assert_allclose(states[-1], x, atol=1e-5)

    def test_finer_integration_preserves_controller_sample_period(self):
        model = HarmonyMmcGfl(Pac=90e6)
        model.solve_equilibrium()
        def inputs(t):
            voltage = model.u0.copy()
            voltage[1] += 200 * np.sin(2 * np.pi * 50 * t)
            return voltage
        _, coarse, _ = model.simulate(.01, inputs, timestep=20e-6, output_stride=2)
        _, fine, _ = model.simulate(.01, inputs, timestep=10e-6, output_stride=4)
        np.testing.assert_allclose(coarse[:, 12:17], fine[:, 12:17], atol=1e-5)


if __name__ == "__main__":
    unittest.main()
