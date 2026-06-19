#ifndef _CONTROL_BLOCK_H_
#define _CONTROL_BLOCK_H_

#include "../Constants.h"

class ControlBlock {
public:
	ControlBlock() = default;

	virtual void printValues() {};

	virtual ~ControlBlock() = default;
};

#endif // !_CONTROL_BLOCK_H_
