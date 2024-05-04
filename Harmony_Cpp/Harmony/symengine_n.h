#ifndef SYMENGINE_N_H
#define SYMENGINE_N_H

#pragma once

#include "basic.h"
#include <complex>

template<typename T>
class BasicType {
public:
	BasicType(const T& val) : value(val) {}
	T getValue() const { return value; }
private:
	T value;
};

// Example definition of Rational
class Rational {
public:
	Rational(int num, int denom) : numerator(num), denominator(denom) {}
	int getNumerator() const { return numerator; }
	int getDenominator() const { return denominator; }
private:
	int numerator;
	int denominator;
};

// Example definition of Integer
class Integer {
public:
	Integer(int val) : value(val) {}
	int getValue() const { return value; }
private:
	int value;
};

// Example definition of Complex
class Complex {
public:
	//Complex(double realPart, double imagPart) : real(realPart), imag(imagPart) {}
	Complex(double realPart = 0.0, double imagPart = 0.0) : real(realPart), imag(imagPart) {}

	double getReal() const { return real; }
	double getImag() const { return imag; }

	// Overload the * operator for complex multiplication
	Complex operator*(const Complex& other) const {
		double resultReal = real * other.real - imag * other.imag;
		double resultImag = real * other.imag + imag * other.real;
		return Complex(resultReal, resultImag);
	}

	// Overload the += operator for complex addition and assignment
	Complex& operator+=(const Complex& other) {
		real += other.real;
		imag += other.imag;
		return *this;
	}

	// Overload the -= operator for complex addition and assignment
	Complex& operator-=(const Complex& other) {
		real -= other.real;
		imag -= other.imag;
		return *this;
	}

	// Define the abs function for Complex numbers
	double abs() const {
		return std::sqrt(real * real + imag * imag);
	}

	// Overloaded division operator
	Complex operator/(const Complex& other) const {
		double divisor = other.real * other.real + other.imag * other.imag;
		double realPart = (real * other.real + imag * other.imag) / divisor;
		double imagPart = (imag * other.real - real * other.imag) / divisor;
		return Complex(realPart, imagPart);
	}

private:
	double real;
	double imag;
};


// Define BasicType for Constants
enum class Constant { pi, eulergamma, exp_1, catalan, goldenratio };

// Function declarations for N
int N(const int& a);
Rational N(const Rational& a);
Complex N(const Complex& a);
template<typename T>
T N(const BasicType<T>& b);
Constant N(const BasicType<Constant>& b);
Complex N(const Basic& b); // N(b::SymEngine.Basic)


#endif // SYMENGINE_N_H

