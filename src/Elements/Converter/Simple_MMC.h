#ifndef SIMPLE_MMC_H
#define SIMPLE_MMC_H


#include <Eigen/Dense>
#include <complex>
#include "../../Solver/DQsym/DQsym.h"
#include "../../Constants.h"
class Simple_MMC
{
public:
    using MatrixXcd = Eigen::MatrixXcd;
    using MatrixXd = Eigen::MatrixXd;
    using VectorXd = Eigen::VectorXd;
    using VectorXi = Eigen::VectorXi;
    using VectorXcd = Eigen::VectorXcd;
    using Vector3d = Eigen::Vector3d;

    struct StepResult
    {
        MatrixXcd y_mmc;      // raw MMC DSSS output
        MatrixXcd i_up;       // upper-arm current coefficients
        MatrixXcd i_low;      // lower-arm current coefficients
        MatrixXcd vc_up;      // upper capacitor-voltage coefficients
        MatrixXcd vc_low;     // lower capacitor-voltage coefficients
        MatrixXcd vout_up;    // upper-arm feedback to MMC
        MatrixXcd vout_low;   // lower-arm feedback to MMC
    };

public:
    Simple_MMC();

    void reset();

    void setSystemMatrices(const MatrixXcd& Ad,
        const MatrixXcd& Bd,
        const MatrixXcd& Cd,
        const MatrixXcd& Dd);

    void setSwitchData(const VectorXd& swOnRes,
        const VectorXd& swOffRes,
        const VectorXi& swType);

    void setParameters(double dt,
        double f0,
        double C,
        Eigen::Index nKeepMMC = 5,
        Eigen::Index nArm = 9);

    void setInitialState(const VectorXcd& xo);

    StepResult step(const MatrixXcd& u1,
        const MatrixXcd& u2,
        const VectorXi& brkVec);

    const MatrixXcd& getLastVoutUp() const;
    const MatrixXcd& getLastVoutLow() const;

    const MatrixXcd& getUpperControlCoeffs() const;
    const MatrixXcd& getLowerControlCoeffs() const;

private:
    MatrixXcd stack_u_4x_3xN(const MatrixXcd& u1,
        const MatrixXcd& u2,
        const MatrixXcd& u3,
        const MatrixXcd& u4) const;

    MatrixXcd makeUpperControlCoeffs(Eigen::Index nCols) const;
    MatrixXcd makeLowerControlCoeffs(Eigen::Index nCols) const;
    MatrixXcd truncateHarmonics(const MatrixXcd& X, Eigen::Index nColsToKeep) const;

    void validateConfigured() const;

private:
    // System matrices
    MatrixXcd Ad_;
    MatrixXcd Bd_;
    MatrixXcd Cd_;
    MatrixXcd Dd_;

    // Switch data
    VectorXd swOnRes_;
    VectorXd swOffRes_;
    VectorXi swType_;

    // Initial state
    VectorXcd xo_;

    // Solver / model parameters
    double dt_;
    double f0_;
    double w_;
    double C_;

    Eigen::Index nKeepMMC_;
    Eigen::Index nArm_;

    // DQ solver
    DQsym dq_;

    // Internal control coefficient matrices
    MatrixXcd Uup_;
    MatrixXcd Ulow_;

    // Internal arm integrator memory
    MatrixXcd ZupOld_;
    MatrixXcd XupOld_;
    MatrixXcd ZlowOld_;
    MatrixXcd XlowOld_;

    // Feedback signals used as next MMC inputs
    MatrixXcd VoutUpForMMC_;
    MatrixXcd VoutLowForMMC_;

    bool matricesConfigured_;
    bool switchesConfigured_;
    bool paramsConfigured_;
    bool stateConfigured_;
};

#endif // SIMPLE_MMC_H