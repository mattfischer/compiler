#include "Front/Procedure.h"

#include <iostream>

namespace Front {
	void printNode(Node *node, std::ostream &o, std::string prefix)
	{
		o << prefix << node << std::endl;
		for(Node *child : node->children) {
			printNode(child, o, prefix + "  ");
		}
	}

	void Procedure::print(std::ostream &o)
	{
		o << "Procedure " << name << std::endl;
		o << "  Arguments:" << std::endl;
		for(Symbol *argument : arguments) {
			o << "    " << argument->type->name << " " << argument->name << std::endl;
		}
		o << "  Body:" << std::endl;
		printNode(body, o, "    ");
	}
}