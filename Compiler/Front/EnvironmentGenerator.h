#ifndef FRONT_ENVIRONMENT_GENERATOR_H
#define FRONT_ENVIRONMENT_GENERATOR_H

#include "Front/Node.h"
#include "Front/Types.h"

#include <vector>

namespace Front {

class EnvironmentGenerator {
public:
	EnvironmentGenerator(Node *tree);

	Types *types() { return mTypes; }
	Scope *scope() { return mScope; }

	std::string errorMessage() { return mErrorMessage; } //!< Error message
	int errorLine() { return mErrorLine; } //!< Error line

private:
	Types *mTypes;
	Scope *mScope;
	std::string mErrorMessage; //!< Error message
	int mErrorLine; //!< Error line

	TypeProcedure *addProcedure(Node *node, Types *types, Scope *scope);
	void addStruct(Node *node, Types *types);
	void addClass(Node *node, Types *types, Scope *scope);
	void addClasses(std::vector<Node*> nodes, Types *types, Scope *scope);
	Type *createType(Node *node, Types *types);
};

}

#endif
