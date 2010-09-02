#ifndef IR_H
#define IR_H

struct IRLine {
	enum Type {
		TypeLoad,
		TypeLoadImm,
		TypeAdd,
		TypeMult,
		TypePrint,
		TypeEqual,
		TypeNequal,
		TypeJump,
		TypeCJump,
		TypeNCJump
	};

	Type type;
	void *lhs;
	void *rhs1;
	void *rhs2;

	IRLine(Type _type, void *_lhs, void *_rhs1, void *_rhs2) : type(_type), lhs(_lhs), rhs1(_rhs1), rhs2(_rhs2) {}
};
#endif