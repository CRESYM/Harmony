#ifndef SIMPLE_MMC_H
#define SIMPLE_MMC_H


#include "../../Solver/DQsym/DQsym.h"

#include "Converter.h"
#include "../../Include_control_blocks.h"

class Simple_MMC : public Converter
{

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

    void setInitialState(const VectorXcd& x0) { initial_state = x0; };

    /*StepResult step(const MatrixXcd& u1,
        const MatrixXcd& u2,
        const VectorXi& brkVec);*/

    const MatrixXcd& getLastVoutUp() const { return VoutUpForMMC_;};
    const MatrixXcd& getLastVoutLow() const { return VoutLowForMMC_; };

    const MatrixXcd& getUpperControlCoeffs() const { return Uup_; };
    const MatrixXcd& getLowerControlCoeffs() const { return Ulow_; };

private:
    
    MatrixXcd makeUpperControlCoeffs(Eigen::Index nCols) const;
    MatrixXcd makeLowerControlCoeffs(Eigen::Index nCols) const;
    MatrixXcd truncateHarmonics(const MatrixXcd& X, Eigen::Index nColsToKeep) const;


private:
	// System matrices already defined in Converter class as: A_matrix, B_matrix, C_matrix, D_matrix

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