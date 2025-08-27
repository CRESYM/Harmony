#include "DQsym.h"

vector<complex<double>> DQsym::add(vector<complex<double>> a, vector<complex<double>> b)
{
	int n = std::max(a.size(), b.size());
	vector<complex<double>> c(n);
	for (int i = 0; i < n; i++)
	{
		if (i >= a.size()) a.push_back(0);
		if (i >= b.size()) b.push_back(0);
		c[i] = a[i] + b[i];
	}

	return c;
}

vector<complex<double>> DQsym::subtract(vector<complex<double>> a, vector<complex<double>> b)
{
	int n = std::max(a.size(), b.size());
	vector<complex<double>> c(n);
	for (int i = 0; i < n; i++)
	{
		if (i >= a.size()) a.push_back(0);
		if (i >= b.size()) b.push_back(0);
		c[i] = a[i] - b[i];
	}
	return c;
}