#ifndef MYSYMBOLICTYPE_H
#define MYSYMBOLICTYPE_H

#include <string>

class MySymbolicType {
private:
	std::string expression; // Example: Store symbolic expressions as strings

public:
	// Constructor
	MySymbolicType(const std::string& expr) : expression(expr) {}

	// Getter method for expression
	std::string getExpression() const {
		return expression;
	}
};

// Custom hash function for SymbolicType
namespace std {
	template <>
	struct hash<MySymbolicType> {
		size_t operator()(const MySymbolicType& st) const {
			// Define your hash calculation based on the properties of SymbolicType
			// For example:
			return hash<string>{}(st.getExpression()); // Assuming getExpression returns a string
		}
	};
}
#endif // MYSYMBOLICTYPE_H
