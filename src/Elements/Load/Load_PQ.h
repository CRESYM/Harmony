#ifndef _LOADPQ_H_  
#define _LOADPQ_H_  
  
#include "Load_base.h"  
  
class LoadPQ : public Load_base {  
public:  
    LoadPQ(const std::string& symbol, const std::string& location, int pins, vector<double> values);
  
    // Destructor  
    ~LoadPQ() {}  

	// Power flow computation for AC networks
    void computePowerFlowAC(std::map<std::string, double>& busAC,
        std::map<std::string, double>& global_params) const override;
  
private:  
	double P;  // Active power 
	double Q;  // Reactive power
  
};  
  
#endif
