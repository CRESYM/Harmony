#include <cstddef>

#include <iostream> // For std::cout (for debugging)

// Declaration of Basic struct
struct Basic {
	void* ptr; // Using void* for compatibility with C pointers
	size_t size; // Size of the data (assuming z represents some form of array)

	// Default constructor
	Basic();

	// Constructor to create a Basic object from a pointer
	Basic(void* v, size_t sz);

	// Constructor to create a Basic object from a pointer
	Basic(void* v);

	// Constructor to create a Basic object from a double value
	Basic(double value);

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
extern "C" {
	void* basic_new_stack();
	void basic_free(void* z);
}
