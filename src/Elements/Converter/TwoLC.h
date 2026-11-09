#ifndef TWOLC_H
#define TWOLC_H

/**
 * @file TwoLC.h
 * @brief Two-level converter (2LC) state-space model.
 *
 * Same control stack as the MMC (PLL, P or Vdc, Q or Vac, output-current loop,
 * Padé delay) on a reactor plant: v_M = m V_dc/2, states i_d and i_q.
 * Pac>0 is AC export. Ports of Y are (i_dc, i_d, i_q) from (V_dc, v_d, v_q).
 */

#include "Converter.h"
#include "core/Include_control_blocks.h"

class TwoLC : public Converter {
public:
    TwoLC(const std::string& symbol, const std::string& location,
        const std::vector<double>& converter_params);
    TwoLC(const std::string& symbol, const std::string& location,
        const std::vector<double>& converter_params,
        const std::vector<double>& controller_params);

    ~TwoLC() override = default;

    void solveEquilibrium() override;
    Eigen::MatrixXd computeStateDerivatives(const Eigen::VectorXd& x, const Eigen::VectorXd& u) override;
    void computeABCD() override;
    std::vector<std::vector<std::complex<double>>> compute_y_parameters(double frequency) override;
    void printElementValues() override;
    int getNumberOfInternalStates() const override { return number_of_states; }

private:
    struct Evaluation {
        Eigen::VectorXd F;
        Eigen::Vector3d y;
    };

    void initController(const std::vector<double>& controller_params);
    Evaluation evaluate(const Eigen::VectorXd& x, const Eigen::VectorXd& u);
    Eigen::Vector3d operatingInput() const;

    int number_of_states = 2;
    int vdc_index_ = -1;
    double last_vMd_ = 0.0;
    double last_vMq_ = 0.0;
};

#endif
