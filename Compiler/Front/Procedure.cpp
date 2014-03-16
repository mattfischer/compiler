#include "Front/Procedure.h"

#include <iostream>

namespace Front {
	void printNode(Node *node, std::ostream &o, std::string prefix)
	{
		o << prefix << node << std::endl;
		for(unsigned int i=0; i<node->children.size(); i++) {
			printNode(node->children[i], o, prefix + "  ");
		}
	}

	void Procedure::print(std::ostream &o)
	{
		o << "Procedure " << name << std::endl;
		o << "  Arguments:" << std::endl;
		for(unsigned int i=0; i<arguments.size(); i++) {
			o << "    " << arguments[i]->type->name << " " << arguments[i]->name << std::endl;
		}
		o << "  Body:" << std::endl;
		printNode(body, o, "    ");
	}
}