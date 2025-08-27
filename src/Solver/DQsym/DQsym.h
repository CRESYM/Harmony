#ifndef _DQSYM_H_	
#define _DQSYM_H_

#include "../../Constants.h"

class DQsym
{
public:
	DQsym() = default;
	~DQsym() = default;

	vector<complex<double>> add(vector<complex<double>>, vector<complex<double>>);
	vector<complex<double>> subtract(vector<complex<double>>, vector<complex<double>>);
	// vector<complex<double>> multiply(vector<complex<double>>, vector<complex<double>>);
	// vector<complex<double>> integrate(vector<complex<double>>);
};


#endif // _DQSYM_H_