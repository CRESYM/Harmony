#/*ifndef MYBASIC_H
#define MYBASIC_H

#include <cstddef>
#include <iostream> // For std::cout (for debugging)
#include <functional> // Include for std::hash
#include <unordered_map>
#include <basic.h>

// Declaration of Basic struct
struct MyBasic {
	void* ptr; // Using void* for compatibility with C pointers
	size_t size; // Size of the data (assuming z represents some form of array)

	// Default constructor
	MyBasic();

	// Constructor to create a Basic object from a pointer
	MyBasic(void* v, size_t sz);

	// Constructor to create a Basic object from a pointer
	MyBasic(void* v);

	// Constructor to create a Basic object from a double value
	MyBasic(double value);

	// Constructor to create a Basic object from a string
	MyBasic(const std::string& expr) : ptr(nullptr) {
		// Allocate memory for the expression string and store it
		std::string* exprPtr = new std::string(expr);
		// Assign the allocated memory to ptr
		ptr = static_cast<void*>(exprPtr);
	}

	// Destructor to free the resource when Basic object goes out of scope
	~MyBasic();

	// Define the += operator for Basic
	SymEngine::Basic& operator+=(const double rhs);

	// Define the += operator for adding two Basic objects
	SymEngine::Basic& operator+=(const SymEngine::Basic& rhs);

	// Define the -= operator for adding two Basic objects
	SymEngine::Basic& operator-=(const SymEngine::Basic& rhs);

	// Debugging function to print the value
	void printValue() const;

	// Method to check if Basic object is zero
	bool isZero() const;
};

/*namespace std {
	template <>
	struct hash<Basic> {
		size_t operator()(const Basic& b) const {
			// Define your hash calculation based on the properties of Basic
			// For example, you could hash based on the pointer value
			//return hash<void*>{}(b.ptr);
			//return hash<decltype(b.ptr)>{}(b.ptr);
			return hash<uintptr_t>{}(reinterpret_cast<uintptr_t>(b.ptr));
		}
	};
}

//Equality operator overload for comparing Basic objects
bool operator==(const Basic& lhs, const Basic& rhs) {
	// Define your equality comparison based on the properties of Basic
	// For example, you could compare the pointer values
	return lhs.ptr == rhs.ptr;
}

// Custom hash function for Basic

struct BasicHash {
	size_t operator()(const SymEngine::Basic& b) const {
		return std::hash<uintptr_t>{}(reinterpret_cast<uintptr_t>(b.ptr));
	}
};

// Custom equality comparison for Basic
struct BasicEqual {
	bool operator()(const SymEngine::Basic& lhs, const SymEngine::Basic& rhs) const {
		return lhs.ptr == rhs.ptr;
	}
};

// Alias for the unordered_map with custom hash and equality
using CMapBasicBasic = std::unordered_map<SymEngine::Basic, SymEngine::Basic, BasicHash, BasicEqual>;

#endif // MYBASIC_H */
