#ifndef PARAMS_AC_H
#define PARAMS_AC_H

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <unordered_map>
#include <vector>
#include <string>

struct ACNetworkParams {
	std::unordered_map<std::string, Eigen::MatrixXd> network_ac;
	double baseMVA_ac;
	Eigen::MatrixXd bus_entire_ac, branch_entire_ac, gen_entire_ac, gencost_entire_ac;
	int ngrids;
	std::vector<Eigen::MatrixXd> bus_ac, branch_ac, generator_ac, gencost_ac;
	std::vector<std::vector<int>> recRef;
	std::vector<Eigen::VectorXd> pd_ac, qd_ac;
	std::vector<int> nbuses_ac, nbranches_ac, ngens_ac;
	std::vector<Eigen::SparseMatrix<double>> GG_ac, BB_ac, GG_ft_ac, BB_ft_ac, GG_tf_ac, BB_tf_ac;
	std::vector<Eigen::VectorXi> fbus_ac, tbus_ac;
	std::vector<Eigen::MatrixXd> anglelim_rad;
	std::vector<Eigen::VectorXi> IDtoCountmap;
	std::vector<int> refbuscount_ac;
};
ACNetworkParams params_ac(const std::string& acgrid_name);

#endif // PARAMS_AC_H

