#ifndef AC_SOURCE_H
#define AC_SOURCE_H

#include "Source_base.h"

/*
AC source is modeled as ideal AC voltage source with added series impedance. Its parameters are
AC are series impedance value. It also can be created as single, three phase, etc. and thus, 
pins number is added also as its input.
Additionally, after initial creation needs to be added power flow data:
voltage magnitude and phase shift, active and reactive powers, and their max and min values.
*/
class AC_source : public Source_base {
public:
    // Constructor
    AC_source(const std::string& symbol, const std::string& location, int pins, DenseMatrix Z);
	AC_source(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& Z);
	AC_source(const std::string& symbol, const std::string& location, int pins, const double Z);

    // Destructor
    ~AC_source();

    void writeMNAmatrix(SymEngine::DenseMatrix&, std::unordered_map<Bus*, int>&, int, std::map<Element*, std::vector<RCP<const Basic>>>&) override;

    // Function to print AC source values
    void printElementValues();

private:
    // Properties

};

#endif

