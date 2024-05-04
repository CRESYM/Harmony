#ifndef TRANSMISSIONLINE_H
#define TRANSMISSIONLINE_H

#include <vector>
#include <complex>
#include <string>
#include <fstream>
#include <tuple>
#include "Element.h"
#include "symengine_n.h"

class TransmissionLine : public Element {
public:
	TransmissionLine() {}
	~TransmissionLine() {}

	std::vector<std::vector<Complex>> eval_y(const Complex& s);
	void make_power_flow_ac(std::vector<std::vector<double>>& dict, std::vector<std::vector<double>>& global_dict);
	void make_power_flow_dc(std::vector<std::vector<double>>& dict, std::vector<std::vector<double>>& global_dict);
	void save_data(const std::string& file_name, const std::vector<double>& omegas);
	void plot_data(const std::vector<double>& omegas);

private:
	//std::vector<std::vector<Complex>> eval_abcd(const Complex& s);
	//std::tuple<std::vector<Complex>, std::vector<Complex>, std::vector<Complex>> eval_parameters(double omega);
	template <typename T> void write_to_file(std::ofstream& file, double omega, const std::vector<T>& data);
};

#endif // TRANSMISSIONLINE_H
