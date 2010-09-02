#include "IR.h"

static char* names[] = {
	/* TypeLoad		*/	"load",
	/* TypeLoadImm	*/	"loadimm",
	/* TypeAdd		*/	"add",
	/* TypeMult		*/	"mult",
	/* TypePrint	*/	"print"
};

void IRLine::print() const
{
	printf("%s ", names[type]);

	switch(type) {
		case TypePrint:
			printf("%s", ((IRSymbol*)lhs)->name.c_str());
			break;

		case TypeLoadImm:
			printf("%s, %i", ((IRSymbol*)lhs)->name.c_str(), rhs1);
			break;

		case TypeLoad:
			printf("%s, %s", ((IRSymbol*)lhs)->name.c_str(), ((IRSymbol*)rhs1)->name.c_str());
			break;

		case TypeAdd:
		case TypeMult:
			printf("%s, %s, %s", ((IRSymbol*)lhs)->name.c_str(), ((IRSymbol*)rhs1)->name.c_str(), ((IRSymbol*)rhs2)->name.c_str());
			break;
	}
	printf("\n");
}