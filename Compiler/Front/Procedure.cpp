#include "Front/Procedure.h"

#include <iostream>

namespace Front {
	void printNode(Node *node, std::string prefix)
	{
		std::cout << prefix << node << std::endl;
		for(unsigned int i=0; i<node->children.size(); i++) {
			printNode(node->children[i], prefix + "  ");
		}
	}

	void Procedure::print()
	{
		std::cout << "Procedure " << name << std::endl;
		std::cout << "  Arguments:" << std::endl;
		for(unsigned int i=0; i<arguments.size(); i++) {
			std::cout << "    " << arguments[i]->type->name << " " << arguments[i]->name << std::endl;
		}
		std::cout << "  Body:" << std::endl;
		printNode(body, "    ");
	}
}