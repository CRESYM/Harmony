#ifndef _DQSYM_CONVERSION_FUNCTIONS_H_
#define _DQSYM_CONVERSION_FUNCTIONS_H_

/**
 * @file DQsym_Conversion_Functions.h
 * @brief Dynamic-phasor arithmetic and abc ↔ dqn domain conversions.
 *
 * Provides element-wise phasor operations, harmonic truncation, and
 * reconstruction of three-phase abc waveforms from dqn coefficient blocks
 * used by the DQsym solver.
 */

#include "../../Constants.h"

/**
 * @brief Time-domain abc waveform reconstructed from dqn coefficients.
 */
struct ABCResult
{
    std::vector<double> t;
    MatrixXd Xabc;
};

/**
 * @brief Element-wise addition of two dynamic-phasor coefficient matrices.
 * @param a First operand.
 * @param b Second operand.
 * @return a + b in the phasor domain.
 */
extern MatrixXcd dq_add(const MatrixXcd& a, const MatrixXcd& b);

/**
 * @brief Element-wise subtraction of two dynamic-phasor coefficient matrices.
 * @param a Minuend.
 * @param b Subtrahend.
 * @return a - b in the phasor domain.
 */
extern MatrixXcd dq_subtract(const MatrixXcd& a, const MatrixXcd& b);

/**
 * @brief Integrates a dynamic-phasor signal over one time step.
 * @param Zpnz_old Previous integrated state (updated in place).
 * @param Xpnz_old Previous input sample (updated in place).
 * @param Xpnz Current input sample.
 * @param dt Time step (s).
 * @param w Angular frequency (rad/s).
 * @return Integrated phasor coefficients.
 */
extern MatrixXcd dq_integrate(MatrixXcd& Zpnz_old, MatrixXcd& Xpnz_old, const MatrixXcd& Xpnz,
    double dt, double w);

/**
 * @brief Multiplies two dynamic-phasor coefficient matrices (convolution in harmonic domain).
 * @param x_coef1_in First operand coefficients.
 * @param y_coef1_in Second operand coefficients.
 * @return Product coefficients.
 */
extern MatrixXcd dq_multiply(const MatrixXcd& x_coef1_in, const MatrixXcd& y_coef1_in);

/**
 * @brief Converts abc-stacked state-space matrices to phasor/sequence domain.
 * @param A Continuous-time state matrix (abc layout).
 * @param B Continuous-time input matrix (abc layout).
 * @param C Continuous-time output matrix (abc layout).
 * @param D Continuous-time feed-through matrix (abc layout).
 * @param Adc Output: phasor-domain state matrix.
 * @param Bdc Output: phasor-domain input matrix.
 * @param Cdc Output: phasor-domain output matrix.
 * @param Ddc Output: phasor-domain feed-through matrix.
 */
extern void convertToPhasor(const MatrixXcd& A, const MatrixXcd& B,
    const MatrixXcd& C, const MatrixXcd& D,
    MatrixXcd& Adc, MatrixXcd& Bdc,
    MatrixXcd& Cdc, MatrixXcd& Ddc);

/**
 * @brief Truncates harmonic columns beyond nKeep in a coefficient matrix.
 * @param X Input coefficient matrix (3 rows × H columns).
 * @param nColsToKeep Number of harmonic columns to retain.
 * @return Truncated matrix.
 */
extern MatrixXcd truncateHarmonics(const MatrixXcd& X, int nColsToKeep);

/**
 * @brief Reconstructs abc phase values from a 3×H dqn block at one angle.
 * @param Xdcpnz_c dqn coefficient block (3 rows).
 * @param theta Electrical angle (rad) at which to evaluate.
 * @return Three-phase abc vector [a, b, c].
 */
extern Vector3d dqn2abc_at_time(const MatrixXcd& Xdcpnz_c, double theta);

/**
 * @brief Reconstructs abc values for all 3-row output groups at one angle.
 * @param Y Output coefficient matrix with rows grouped in triplets.
 * @param theta Electrical angle (rad) at which to evaluate.
 * @return One abc vector per output group.
 */
extern std::vector<Vector3d> dqn2abc_groups_at_time(const MatrixXcd& Y, double theta);

/**
 * @brief Simulates abc waveform reconstruction over a time interval.
 * @param Xdcpnz_c dqn coefficient block (3 rows × H columns).
 * @param freq_hz Fundamental frequency in Hz.
 * @param t0 Start time (s).
 * @param t1 End time (s).
 * @param Ts Sample period (s).
 * @return Time vector and abc waveform matrix.
 */
extern ABCResult simulate_dqn2abc(const MatrixXcd& Xdcpnz_c,
    double freq_hz, double t0, double t1, double Ts);

#endif // _DQSYM_CONVERSION_FUNCTIONS_H_
