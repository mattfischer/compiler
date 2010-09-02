#ifndef IR_H
#define IR_H

#include <string>

#include "Type.h"

struct IRLine {
	enum Type {
		TypeLoad,
		TypeLoadImm,
		TypeAdd,
		TypeMult,
		TypePrint
	};

	Type type;
	long lhs;
	long rhs1;
	long rhs2;

	IRLine(Type _type, long _lhs, long _rhs1, long _rhs2) : type(_type), lhs(_lhs), rhs1(_rhs1), rhs2(_rhs2) {}

	void print() const;
};

struct IRSymbol {
	std::string name;
	Type *type;

	IRSymbol(const std::string &_name, Type *_type) : name(_name), type(_type) {}
};

#endif