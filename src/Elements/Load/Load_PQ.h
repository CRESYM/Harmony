#ifndef _LOADPQ_H_  
#define _LOADPQ_H_  
  
#include "Load_base.h"  
  
class LoadPQ : public Load_base {  
public:  
    LoadPQ(const std::string& symbol, int pins);  
  
    // Destructor  
    ~LoadPQ() {}  
  
private:  
  
};  
  
#endif
