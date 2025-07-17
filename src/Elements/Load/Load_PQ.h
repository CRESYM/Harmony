#ifndef _LOADPQ_H_  
#define _LOADPQ_H_  
  
#include "../Element.h"  
  
class LoadPQ : public Element {  
public:  
    LoadPQ(const std::string& symbol, int pins);  
  
    // Destructor  
    ~LoadPQ() {}  
  
    // Power flow calculations (AC and DC)  
    void computePowerFlowAC(std::map<std::string, std::map<std::string, double>>& branchData,  
        std::map<std::string, double>& globalParams) const override;  
  
    void computePowerFlowDC(std::map<std::string, std::map<std::string, double>>& branchDCData,  
        std::map<std::string, double>& globalParams) const override;  
  
private:  
  
};  
  
#endif
