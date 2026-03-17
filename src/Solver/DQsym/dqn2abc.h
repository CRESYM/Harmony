#pragma once

#include <vector>
#include <string>
#include <complex>
#include <Eigen/Dense>

struct ABCResult {
    std::vector<double> t;   // time vector
    Eigen::MatrixXd Xabc;    // N x 3 matrix: columns = xa, xb, xc
};

// Reconstruct abc values at one electrical angle theta
Eigen::Vector3d dqn2abc_at_time(const Eigen::MatrixXcd& Xdcpnz_c, double theta);

// Simulate abc waveform over a time interval
ABCResult simulate_dqn2abc(const Eigen::MatrixXcd& Xdcpnz_c,
    double freq_hz,
    double t0,
    double t1,
    double Ts);

// Optional convenience runner for testing
void run_dqn2abc();