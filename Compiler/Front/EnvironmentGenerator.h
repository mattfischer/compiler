#ifndef FRONT_ENVIRONMENT_GENERATOR_H
#define FRONT_ENVIRONMENT_GENERATOR_H

#include "Front/Node.h"
#include "Front/Types.h"
#include "Front/ExportInfo.h"

#include <vector>
#include <set>

namespace Front {

/*!
 * \brief Generate a compilation environment, consisting of a type list and global symbols
 */
class EnvironmentGenerator {
public:
	EnvironmentGenerator(Node *tree, const std::vector<ExportInfo*> &imports);

	Types *types() { return mTypes; } //!< Types in environment
	Scope *scope() { return mScope; } //!< Global scope of environment

	const std::string &errorMessage() { return mErrorMessage; } //!< Error message
	const std::string &errorLocation() { return mErrorLocation; } //!< Error line

private:
	Types *mTypes; //!< Types in environment
	Scope *mScope; //!< Global scope of environment
	std::set<Type*> mCompleteTypes; //!< List of known-complete types
	std::vector<Type*> mCompletionStack; //!< List of types currently being completed
	std::string mErrorMessage; //!< Error message
	std::string mErrorLocation; //!< Error location

	void addStruct(Node *node);
	void addClass(Node *node);
	Type *createType(Node *node, bool dummy);
	Type *completeType(Type *type);
	void completeTypes();
	void constructScope(TypeStruct *typeStruct, Scope *scope);
};

}

#endif
