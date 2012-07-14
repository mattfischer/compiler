#include "Front/Parser.h"
#include "Middle/Optimizer.h"

int main(int arg, char *argv[])
{
	IR::Program *program = Front::Parser::parse("input.lang");
	if(!program) {
		return 1;
	}

	Middle::Optimizer::optimize(program);

	return 0;
}