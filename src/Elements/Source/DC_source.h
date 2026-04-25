#ifndef _DC_SOURCE_H_
#define _DC_SOURCE_H_

#include "Source_base.h"

class DC_source : public Source_base {
public:
	DC_source(const std::string& symbol, const std::string& location, int pins, double V, double R);
	DC_source(const std::string& symbol, const std::string& location, int pins, const vector<double>& V, double R);
	DC_source(const std::string& symbol, const std::string& location, int pins, double V, const vector<double>& Z);
	DC_source(const std::string& symbol, const std::string& location, int pins, const vector<double>& V, const vector<double>& Z);

	~DC_source() {}

	/// MNA stamp — identical to AC_source (ideal voltage source with symbol in last column)
	void writeMNAmatrix(SymEngine::DenseMatrix&, std::unordered_map<Bus*, int>&, int,
		std::map<Element*, std::vector<RCP<const Basic>>>&) override;

	std::vector<MatrixXcd> simulateInputStep(
		const std::vector<MatrixXcd>& states, int nKeep) const override;
};

#endif