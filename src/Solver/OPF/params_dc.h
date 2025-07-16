#ifndef PARAMS_DC_H
#define PARAMS_DC_H

#include <string>
#include <unordered_map>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <complex>

struct DCNetworkParams {
	std::unordered_map<std::string, Eigen::MatrixXd> network_dc;
	double baseMW_dc, pol_dc;
	int nbuses_dc, nbranches_dc, nconvs_dc;
	Eigen::MatrixXd bus_dc, branch_dc, conv_dc;
	Eigen::SparseMatrix<std::complex<double>> Y_dc;
	Eigen::SparseMatrix<double> y_dc;
	Eigen::VectorXcd ztfc_dc;
	Eigen::VectorXd rtf_dc, xtf_dc, bf_dc, rc_dc, xc_dc;
	Eigen::VectorXd gtfc_dc, btfc_dc, aloss_dc, bloss_dc, closs_dc;
	Eigen::VectorXd basekV_dc;
	Eigen::VectorXi convState_dc;
	Eigen::VectorXi fbus_dc, tbus_dc;
};

DCNetworkParams params_dc(const std::string& case_name);

#endif // PARAMS_DC_H
