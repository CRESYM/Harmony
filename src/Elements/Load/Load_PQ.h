#ifndef _LOADPQ_H_  
#define _LOADPQ_H_  

/**
 * @file Load_PQ.h
 * @brief Constant active and reactive power (PQ) load model.
 */
  
#include "Load_base.h"  
  
/**
 * @class LoadPQ
 * @brief Constant P-Q load for AC power-flow studies.
 * @ingroup source
 */
class LoadPQ : public Load_base {  
public:
    /**
     * @brief Construct a constant PQ load.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases).
     * @param values Vector containing active power P and reactive power Q.
     */
    LoadPQ(const std::string& symbol, const std::string& location, int pins, vector<double> values);
  
    // Destructor  
    ~LoadPQ() {}  

	// Power flow computation for AC networks
    void computePowerFlow(std::map<std::string, double>& busAC,
		std::map<std::string, double>& global_params) const override;
 
private:  
	double P;  // Active power 
	double Q;  // Reactive power
  
};  
  
#endif
