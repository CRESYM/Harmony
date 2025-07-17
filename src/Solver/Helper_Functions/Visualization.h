#ifndef _VISUALIZATION_H_
#define _VISUALIZATION_H_

#include "../../Constants.h"

extern void bode_plot(const std::vector<double>& freq,
	const std::vector<std::vector<double>>& mag_dB,
	const std::vector<std::vector<double>>& phase_deg,
	const std::vector<std::string>& labels,
	const std::string& title = "Bode Plot");

extern void nyquist_plot(const std::vector<double>& real_part,
	const std::vector<double>& imag_part,
	const std::string& title = "Nyquist Plot");

#endif // _VISUALIZATION_H