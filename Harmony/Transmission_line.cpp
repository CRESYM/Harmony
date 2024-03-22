#include "transmission_line.h"
#include "abcd_parameters.h"

#include <complex>
#include <string>
#include <map>
#include <vector>

// Function to evaluate Y matrix given a Transmission_line
std::vector<std::vector<Complex>> TransmissionLine::eval_y(const Complex& s) {
	// Assuming eval_abcd and abcd_to_y functions are defined elsewhere
	return abcd_to_y(eval_abcd(s));
}
/*
// Function to calculate power flow AC parameters and update a dictionary
void TransmissionLine::make_power_flow_ac(std::vector<std::vector<double>>& dict, std::vector<std::vector<double>>& global_dict) {
	int key = dict["branch"].size();
	dict["branch"][key]["transformer"] = false;
	dict["branch"][key]["tap"] = 1;
	dict["branch"][key]["shift"] = 0;
	dict["branch"][key]["c_rating_a"] = 1;

	std::vector<std::vector<Complex>> abcd = eval_abcd(Complex(0.0, global_dict["omega"] * 1e-6));
	int n = abcd.size() / 2;
	Complex Z = abcd[0][n] / global_dict["Z"];
	Complex Y = (abcd[n][0] * std::conj(abcd[n][n]) - abcd[n][n]) * global_dict["Z"];

	dict["branch"][key]["br_r"] = Z.real();
	dict["branch"][key]["br_x"] = Z.imag();
	dict["branch"][key]["g_fr"] = Y.real();
	dict["branch"][key]["b_fr"] = Y.imag();
	dict["branch"][key]["g_to"] = Y.real();
	dict["branch"][key]["b_to"] = Y.imag();
}

void TransmissionLine::make_power_flow_dc(std::vector<std::vector<double>>& dict, std::vector<std::vector<double>>& global_dict) {
	int key = dict["branchdc"].size();
	dict["branchdc"][key]["l"] = 0;
	dict["branchdc"][key]["c"] = 0;

	std::vector<std::vector<Complex>> abcd = eval_abcd(Complex(0.0, 1e-6));
	int n = abcd.size() / 2;
	Complex Z = abcd[0][n] / global_dict["Z"];
	dict["branchdc"][key]["r"] = Z.real();
}

void TransmissionLine::save_data(const std::string& file_name, const std::vector<double>& omegas) {
	// Implementation of save_data function
}

void TransmissionLine::plot_data(const std::vector<double>& omegas) {
	// Implementation of plot_data function
}

std::vector<std::vector<Complex>> TransmissionLine::eval_abcd(const Complex& s) {
	// Implementation of eval_abcd function
}

std::tuple<std::vector<Complex>, std::vector<Complex>, std::vector<Complex>> TransmissionLine::eval_parameters(double omega) {
	// Implementation of eval_parameters function
}

template <typename T>
void TransmissionLine::write_to_file(std::ofstream& file, double omega, const std::vector<T>& data) {
	// Implementation of write_to_file function
}*/