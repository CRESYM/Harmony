#ifndef _RES_BASE_H_
#define _RES_BASE_H_

#include "../Element.h"

class RES_base : public Element {
public:
	RES_base(const string& symbol) : Element(symbol, 3, 3) {}

	~RES_base() = default;
};


#endif
