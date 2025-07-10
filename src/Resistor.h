#ifndef _RESISTOR_H_
#define _RESISTOR_H_

#include "Element.h"

class Resistor : public Element {
public:
    Resistor(const std::string& symbol, int pins, double R);

    // Unified constructor supports both single-phase and multi-phase
    Resistor(const std::string& symbol, int pins, const std::vector<double>& R);

    ~Resistor();

    const std::vector<std::vector<double>>& getYMatrix() const { return Y_matrix; }

    
    // Override writeMNAmatrix for resistor stamping(nodal admittance)
        void writeMNAmatrix(DenseMatrix & A,
            int num_equations,
            int index,
            const RCP<const Basic>&value,
            const std::unordered_map<Bus*, int>&busIndex) override;
    
     //void writeMatrixSymbolic(SymEngine::DenseMatrix& Y,
     //    const std::unordered_map<Bus*, int>& busIndex);

     // Numeric MNA stamping for multi-phase values
     void writeMNAmatrixNumeric(Eigen::MatrixXd& A,
         int num_equations,
         int index,
         const std::unordered_map<Bus*, int>& busIndex) override;
        
    // Override to print resistor-specific data
    void printElementValues() override;

private:
    std::vector<double> R_values;
    std::vector<std::vector<double>> Y_matrix;
    void initializeYMatrix(int pins);
};

#endif // _RESISTOR_H_
