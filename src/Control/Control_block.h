#ifndef _CONTROL_BLOCK_H_
#define _CONTROL_BLOCK_H_

#include "../Constants.h"

class ControlBlock {
public:
	ControlBlock() = default; // Default constructor

	virtual void printValues() {}; // Pure virtual function to print element-specific values

	~ControlBlock() = default; // Default destructor
};

#endif // !_CONTROL_BLOCK_H_
#pragma once
