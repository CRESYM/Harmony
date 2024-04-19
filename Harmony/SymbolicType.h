#ifndef SYMBOLICTYPE_H
#define SYMBOLICTYPE_H

#include <string>

class SymbolicType {
private:
	std::string expression; // Example: Store symbolic expressions as strings

public:
	// Constructor
	SymbolicType(const std::string& expr) : expression(expr) {}

	// Getter method for expression
	std::string getExpression() const {
		return expression;
	}

	// Other methods for symbolic manipulation may be added here
};
#endif // SYMBOLICTYPE_H