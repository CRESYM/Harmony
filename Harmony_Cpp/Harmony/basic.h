#ifndef BASIC_H
#define BASIC_H

#include <cstddef>
#include <iostream> // For std::cout (for debugging)
#include <functional> // Include for std::hash

// Declaration of Basic struct
struct Basic {
	void* ptr; // Using void* for compatibility with C pointers
	//double value;
	size_t size; // Size of the data (assuming z represents some form of array)

	// Default constructor
	Basic();

	// Constructor to create a Basic object from a pointer
	Basic(void* v, size_t sz);

	// Constructor to create a Basic object from a pointer
	Basic(void* v);

	// Constructor to create a Basic object from a double value
	Basic(double value);

	// Constructor to create a Basic object from a string
	Basic(const std::string& expr) : ptr(nullptr) {
		// Allocate memory for the expression string and store it
		std::string* exprPtr = new std::string(expr);
		// Assign the allocated memory to ptr
		ptr = static_cast<void*>(exprPtr);
	}

	// Destructor to free the resource when Basic object goes out of scope
	~Basic();

	// Define the += operator for Basic
	Basic& operator+=(const double rhs);

	// Define the += operator for adding two Basic objects
	Basic& operator+=(const Basic& rhs);

	// Define the -= operator for adding two Basic objects
	Basic& operator-=(const Basic& rhs);

	// Debugging function to print the value
	void printValue() const;

	// Method to check if Basic object is zero
	bool isZero() const;

	// Define the equality operator (operator==)
};

// Implementation of Basic constructor that accepts a double value
Basic::Basic(double value) : ptr(nullptr) {
	// Allocate memory for a double and store the value
	double* doublePtr = new double(value);
	// Assign the allocated memory to ptr
	ptr = static_cast<void*>(doublePtr);
}

// Implementation of the += operator for adding a double to Basic
Basic& Basic::operator+=(const double rhs) {
	double* valuePtr = static_cast<double*>(ptr);
	*valuePtr += rhs;
	return *this;
}

// Implementation of the += operator for adding two Basic objects
Basic& Basic::operator+=(const Basic& rhs) {
	// Add the values represented by rhs to this object
	double* valuePtr = static_cast<double*>(ptr);
	double* rhsPtr = static_cast<double*>(rhs.ptr);
	*valuePtr += *rhsPtr;
	return *this;
}

// Implementation of the -= operator for subtracting two Basic objects
Basic& Basic::operator-=(const Basic& rhs) {
	// Subtract the value represented by rhs from this object
	double* valuePtr = static_cast<double*>(ptr);
	double* rhsPtr = static_cast<double*>(rhs.ptr);
	*valuePtr -= *rhsPtr;
	return *this;
}

// Debugging function to print the value
void Basic::printValue() const {
	if (ptr != nullptr) {
		double* valuePtr = static_cast<double*>(ptr);
		std::cout << "Value: " << *valuePtr << std::endl;
	}
	else {
		std::cout << "Value: [nullptr]" << std::endl;
	}
}

// Implementation of isZero() method
bool Basic::isZero() const {
	return ptr == nullptr;
}

// Declaration of basic_new_stack function
/*extern "C" {
	void* basic_new_stack();
	void basic_free(void* z);
}*/

/*
// Define the hash function for Basic
struct BasicHash {
	size_t operator()(const Basic& basic) const {
		// Implement hash calculation logic here
		// For example, if Basic represents a double value:
		// return std::hash<double>{}(*(static_cast<double*>(basic.ptr)));
		//return std::hash<double>{}(basic.value);
		return std::hash<void*>{}(basic.ptr);
	}
};*/

// Custom hash function for Basic
namespace std {
	template <>
	struct hash<Basic> {
		size_t operator()(const Basic& b) const {
			// Define your hash calculation based on the properties of Basic
			// For example, you could hash based on the pointer value
			return hash<void*>{}(b.ptr);
		}
	};
}

/*bool operator==(const Basic& other) const {
	// Implement equality comparison logic here
	// For example, if Basic represents a double value:
	// return *(static_cast<double*>(ptr)) == *(static_cast<double*>(other.ptr));
	//return value == other.value;
	return ptr == other.ptr;
}*/

bool operator==(const Basic& lhs, const Basic& rhs) {
	// Define your equality comparison based on the properties of Basic
	// For example, you could compare the pointer values
	return lhs.ptr == rhs.ptr;
}

#endif // BASIC_H