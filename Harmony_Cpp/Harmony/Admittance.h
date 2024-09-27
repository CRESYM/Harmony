#ifndef _ADMITTANCE_H_
#define _ADMITTANCE_H_

#include "Element.h"
#include <vector> // Include vector header
#include <SymEngine/Matrix.h> // Ensure you include the necessary SymEngine headers

using namespace SymEngine; // Use the SymEngine namespace

class Admittance : public Element {
public:
    //Admittance(const std::string& symbol, int pins, const RCP<const Basic>& admittanceValue);
    //Admittance(const std::string& symbol, int pins, const DenseMatrix& admittanceMatrix);
    //Admittance(const std::string& symbol, int pins, const std::vector<RCP<Symbol>>& admittanceValues);

   // Unified constructor for single-phase and three-phase systems
    Admittance(const std::string& symbol, int pins,
        const DenseMatrix& admittanceMatrix = createZeroMatrix(1))
        : Element(symbol, pins, pins), Y_matrix(admittanceMatrix) {

        is_three_phase = (admittanceMatrix.nrows() == 3 && admittanceMatrix.ncols() == 3);

        // Print information about the Admittance type
        if (is_three_phase) {
            std::cout << "Three-phase Admittance created for element: " << symbol << std::endl;
        }
        else {
            std::cout << "Single-phase Admittance created for element: " << symbol << std::endl;
        }
    }

    ~Admittance() override;

    void compute_y_parameters(double frequency) override;

    // Print Admittance values
    void printElementValues() override {
        std::cout << "Admittance Matrix for element: " << getElementSymbol() << std::endl;
        for (int i = 0; i < Y_matrix.nrows(); i++) {
            for (int j = 0; j < Y_matrix.ncols(); j++) {
                std::cout << Y_matrix.get(i, j)->__str__() << " "; // Use __str__() for output
            }
            std::cout << std::endl;
        }
    }


private:
    DenseMatrix Y_matrix; // Matrix representation of admittance

    // Helper function to create a zero matrix of given size
    static DenseMatrix createZeroMatrix(int size) {
        DenseMatrix zeroMatrix(size, size);
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                zeroMatrix.set(i, j, zero); // Use SymEngine's symbolic `zero`
            }
        }
        return zeroMatrix;
    }
};

#endif // _ADMITTANCE_H_

