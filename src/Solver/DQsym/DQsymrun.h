#pragma once

#include <Eigen/Dense>
#include <complex>
#include <vector>
#include <string>
#include <functional>

class DQsymrun
{
public:
    using cd = std::complex<double>;
    using MatrixXcd = Eigen::Matrix<cd, Eigen::Dynamic, Eigen::Dynamic>;
    using VectorXcd = Eigen::Matrix<cd, Eigen::Dynamic, 1>;

    enum class InternalSignal
    {
        VoutUp,
        VoutLow,
        VcUp,
        VcLow,
        Iup,
        Ilow
    };

    struct FeedbackInjection
    {
        int inputBlockIndex = -1;
        InternalSignal signal = InternalSignal::VoutUp;
        bool invert = false;
    };

    struct Config
    {
        double Ts = 0.0;
        double t0 = 0.0;
        double tEnd = 0.0;

        double f = 0.0;  // system frequency
        double omega = 0.0; // system angular frequency

        double C = 0.0; // delete, inside MMC

        int nKeep = 0; // truncation, used for internal MMC calcultion -> delete, kept inside MMC
		int nArm = 0; // delete, inside MMC
		int nInputBlocks = 0;   // DSSS input blocks, each block must be 3 x nKeep 

        int upGroupIndex = 0; // 
        int lowGroupIndex = 0;
        bool invertUpFeedback = true;
        bool invertLowFeedback = true;

        // Switch parameters
        Eigen::VectorXd swOnRes;
        Eigen::VectorXd swOffRes;
        Eigen::VectorXi swType;

        // Breaker / switching schedule as a function of time step
        std::function<Eigen::VectorXi(int step, double t)> breakerFunction;

        // External input blocks, each block must be 3 x nKeep
        std::function<std::vector<MatrixXcd>(int step, double t)> externalInputFunction;

        std::vector<FeedbackInjection> feedbackInjections;
    };

    struct Result
    {
        std::vector<double> time;
        Eigen::MatrixXi brkHistory;

        std::vector<Eigen::MatrixXd> DSSabcHist;

        Eigen::MatrixXd UupAbcHist;
        Eigen::MatrixXd UlowAbcHist;

        Eigen::MatrixXd IupAbcHist;
        Eigen::MatrixXd IlowAbcHist;

        Eigen::MatrixXd VcUpAbcHist;
        Eigen::MatrixXd VcLowAbcHist;

        Eigen::MatrixXd VoutUpAbcHist;
        Eigen::MatrixXd VoutLowAbcHist;
    };

public:
    DQsymrun(
        const Eigen::MatrixXd& Ad_real,
        const Eigen::MatrixXd& Bd_real,
        const Eigen::MatrixXd& Cd_real,
        const Eigen::MatrixXd& Dd_real,
        const Config& cfg
    );

    Result run();

    void exportCSV(const std::string& filename) const;
    void plot() const;

    const Result& getResult() const;
    bool hasRun() const;

private:
    Eigen::MatrixXd Ad_real_;
    Eigen::MatrixXd Bd_real_;
    Eigen::MatrixXd Cd_real_;
    Eigen::MatrixXd Dd_real_;

    Config cfg_;
    Result result_;
    bool hasRun_ = false;

private:
    void validateInputs() const;

    static MatrixXcd getInternalSignalMatrix(
        InternalSignal signal,
        const MatrixXcd& Iup,
        const MatrixXcd& Ilow,
        const MatrixXcd& VcUp,
        const MatrixXcd& VcLow,
        const MatrixXcd& VoutUp,
        const MatrixXcd& VoutLow
    );
};