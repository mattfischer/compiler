#ifndef	TYPE_H
#define TYPE_H

struct Type {
	enum TypeType {
		TypeIntrinsic,
		TypeNone
	};

	TypeType type;
};

struct TypeIntrinsic : public Type {
	enum IntrinsicType {
		Int
	};

	TypeIntrinsic(IntrinsicType _intrinsicType) : intrinsicType(_intrinsicType) {}

	IntrinsicType intrinsicType;
};

extern Type *TypeNone;
extern Type *TypeInt;

#endif