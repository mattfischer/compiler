#include "Front/EnvironmentGenerator.h"

#include "Front/Scope.h"
#include "Front/Symbol.h"

#include <set>
#include <sstream>

namespace Front {

/*!
 * \brief Exception thrown when a type error occurs
 */
class EnvironmentError : public std::exception {
public:
	/*!
	 * \brief Constructor
	 * \param node Node on which error occurred
	 * \param message Error message
	 */
	EnvironmentError(const std::string &location, const std::string &message)
		: mLocation(location), mMessage(message)
	{}

	EnvironmentError(Node &node, const std::string &message)
		: mMessage(message)
	{
		std::stringstream s;
		s << "Line " << node.line;
		mLocation = s.str();
	}

	const std::string &location() { return mLocation; } //!< Error location
	const std::string &message() { return mMessage; } //!< Error message

private:
	std::string mLocation; //!< Error location
	std::string mMessage; //!< Error message
};

/*!
 * \brief Constructor
 * \param tree Parse tree
 */
EnvironmentGenerator::EnvironmentGenerator(Node &tree, const std::vector<std::reference_wrapper<ExportInfo>> &imports)
{
	try {
		mTypes = std::make_unique<Types>();
		mScope = std::make_unique<Scope>();

		// Loop through the tree and pre-populate all declared type names
		for(std::unique_ptr<Node> &node : tree.children) {
			std::stringstream s;

			switch(node->nodeType) {
				case Node::Type::StructDef:
					addStruct(*node);
					break;

				case Node::Type::ClassDef:
					addClass(*node);
					break;
			}
		}

		// Read types from imported binary files
		for(ExportInfo &info : imports) {
			info.read(*mTypes, *mScope);
		}

		// Complete each type, and construct scopes for class types
		std::set<Type*> completeTypes;
		for(std::shared_ptr<Type> &type : mTypes->types()) {
			completeType(type);
			if(type->kind == Type::Kind::Class) {
				std::shared_ptr<TypeStruct> typeStruct = std::static_pointer_cast<TypeStruct>(type);
				constructScope(typeStruct, *mScope);
			}
		}

		// Iterate through the tree, and construct symbols for each procedure
		for(std::unique_ptr<Node> &node : tree.children) {
			if(node->nodeType == Node::Type::ProcedureDef) {
				std::unique_ptr<Symbol> symbol = std::make_unique<Symbol>(createType(*node, false), node->lexVal.s);
				mScope->addSymbol(std::move(symbol));
			}
		}
	} catch(EnvironmentError error) {
		// Collect the error message and line from the exception
		mErrorLocation = error.location();
		mErrorMessage = error.message();
	}
}

/*!
 * \brief Add a structure to the type list
 * \param node Node describing structure
 */
void EnvironmentGenerator::addStruct(Node &node)
{
	// Create the type
	std::shared_ptr<TypeStruct> type = std::make_shared<TypeStruct>(Type::Kind::Struct, node.lexVal.s);
	if(!mTypes->registerType(type)) {
		std::stringstream s;
		s << "Redefinition of structure " << type->name;

		throw EnvironmentError(node, s.str());
	}

	type->parent = 0;
	type->scope = 0;
	type->constructor = 0;

	// Iterate through the member nodes, and create type members for each
	for(std::unique_ptr<Node> &memberNode : node.children[0]->children) {
		std::shared_ptr<Type> memberType = createType(*memberNode->children[0], true);
		type->addMember(memberType, memberNode->lexVal.s, false);
	}
}

/*!
 * \brief Add a class to the type list
 * \param node Node describing structure
 * \param program Program to add to
 */
void EnvironmentGenerator::addClass(Node &node)
{
	// Create the type
	std::shared_ptr<TypeStruct> type = std::make_shared<TypeStruct>(Type::Kind::Class, node.lexVal.s);
	if(!mTypes->registerType(type)) {
		std::stringstream s;
		s << "Redefinition of class " << type->name;
		throw EnvironmentError(node, s.str());
	}

	type->scope = 0;
	type->constructor = 0;

	if(node.children.size() == 2) {
		std::stringstream s;
		s << "Line " << node.line;
		type->parent = std::static_pointer_cast<TypeStruct>(std::static_pointer_cast<Type>(std::make_shared<TypeDummy>(node.children[0]->lexVal.s, s.str())));
	} else {
		type->parent = 0;
	}

	// Iterate through the member nodes, and create type members for each
	std::unique_ptr<Node> &members = node.children[node.children.size() - 1];
	for(std::unique_ptr<Node> &child : members->children) {
		Node &qualifiersNode = *child->children[0];
		Node &memberNode = *child->children[1];

		switch(memberNode.nodeType) {
			case Node::Type::VarDecl:
			{
				std::shared_ptr<Type> memberType = createType(*memberNode.children[0], true);
				type->addMember(memberType, memberNode.lexVal.s, 0);
				break;
			}

			case Node::Type::ProcedureDef:
			{
				unsigned int qualifiers = 0;
				for(std::unique_ptr<Node> &qualifierNode : qualifiersNode.children) {
					switch(qualifierNode->nodeSubtype) {
						case Node::Subtype::Virtual:
							qualifiers |= TypeStruct::Member::QualifierVirtual;
							break;

						case Node::Subtype::Native:
							qualifiers |= TypeStruct::Member::QualifierNative;
							break;

						case Node::Subtype::Static:
							qualifiers |= TypeStruct::Member::QualifierStatic;
							break;
					}
				}

				if((qualifiers & TypeStruct::Member::QualifierStatic) && (qualifiers & TypeStruct::Member::QualifierVirtual)) {
					throw EnvironmentError(memberNode, "Virtual function cannot be static");
				}

				std::shared_ptr<TypeProcedure> procedureType = std::static_pointer_cast<TypeProcedure>(createType(memberNode, true));
				type->addMember(procedureType, memberNode.lexVal.s, qualifiers);
				if(memberNode.lexVal.s == type->name) {
					type->constructor = procedureType;
				}

				break;
			}
		}
	}
}

/*!
 * \brief Search for a type named by a node
 * \param node Node describing type
 * \param dummy Whether to create a dummy type if not found
 * \return Type
 */
std::shared_ptr<Type> EnvironmentGenerator::createType(Node &node, bool dummy)
{
	std::shared_ptr<Type> type;
	switch(node.nodeType) {
		case Node::Type::Array:
			type = createType(*node.children[0], dummy);
			if(Type::equals(*type, *Types::intrinsic(Types::Void))) {
				throw EnvironmentError(node, "Cannot declare array of voids");
			}
			type = std::make_shared<TypeArray>(type);
			break;

		case Node::Type::ProcedureDef:
			{
				// Iterate the tree's argument items
				Node &argumentList = *node.children[1];
				std::vector<std::shared_ptr<Type>> argumentTypes;
				for(std::unique_ptr<Node> &argumentNode : argumentList.children) {
					// Construct the argument type, and add it to the list of types
					std::shared_ptr<Type> argumentType = createType(*argumentNode->children[0], dummy);
					if(Type::equals(*argumentType, *Types::intrinsic(Types::Void))) {
						throw EnvironmentError(*argumentNode, "Cannot declare procedure argument of type void");
					}
					argumentTypes.push_back(argumentType);
				}

				// Construct the procedure type
				std::shared_ptr<Type> returnType = node.children[0] ? createType(*node.children[0], dummy) : Types::intrinsic(Types::Void);
				type = std::make_shared<TypeProcedure>(returnType, argumentTypes);
				break;
			}

		default:
			type = mTypes->findType(node.lexVal.s);
			if(!type) {
				if(dummy) {
					std::stringstream s;
					s << "Line " << node.line;
					type = std::make_shared<TypeDummy>(node.lexVal.s, s.str());
				} else {
					std::stringstream s;
					s << "Type '" << node.lexVal.s << "' not found";
					throw EnvironmentError(node, s.str());
				}
			}
			break;
	}

	return type;
}

/*!
 * \brief Complete a type, replace dummy types with real types, and populating structure/class member offsets
 * \param type Type to complete
 * \return Resulting type, possibly different than parameter if it is a dummy type
 */
std::shared_ptr<Type> EnvironmentGenerator::completeType(std::shared_ptr<Type> type)
{
	// Bail out early if the type is known to be complete
	if(mCompleteTypes.find(type) != mCompleteTypes.end()) {
		return type;
	}

	mCompletionStack.push_back(type);

	switch(type->kind) {
		case Type::Kind::Procedure:
			{
				// Complete the procedure's return and argument types
				std::shared_ptr<TypeProcedure> typeProcedure = std::static_pointer_cast<TypeProcedure>(type);

				typeProcedure->returnType = completeType(typeProcedure->returnType);

				for(unsigned int i=0; i<typeProcedure->argumentTypes.size(); i++) {
					typeProcedure->argumentTypes[i] = completeType(typeProcedure->argumentTypes[i]);
				}
				break;
			}

		case Type::Kind::Array:
			{
				// Complete the array's base type
				std::shared_ptr<TypeArray> typeArray = std::static_pointer_cast<TypeArray>(type);
				typeArray->baseType = completeType(typeArray->baseType);
				break;
			}

		case Type::Kind::Struct:
		case Type::Kind::Class:
			{
				std::shared_ptr<TypeStruct> typeStruct = std::static_pointer_cast<TypeStruct>(type);
				if(typeStruct->parent) {
					for(std::shared_ptr<Type> &completionType : mCompletionStack) {
						if(completionType->name == typeStruct->parent->name) {
							std::stringstream s;
							s << "Class " << typeStruct->name;
							std::stringstream s2;
							s2 << "Inheritance cycle with parent " << typeStruct->parent->name;
							throw EnvironmentError(s.str(), s2.str());
						}
					}

					// Complete parent type
					typeStruct->parent = std::static_pointer_cast<TypeStruct>(completeType(typeStruct->parent));

					// Now that parent is complete, populate sizes and offsets
					typeStruct->vtableOffset = typeStruct->parent->vtableOffset;
					typeStruct->vtableSize = typeStruct->parent->vtableSize;
					typeStruct->allocSize = typeStruct->parent->allocSize;
				} else {
					typeStruct->vtableOffset = 0;
					typeStruct->vtableSize = 0;
				}

				for(TypeStruct::Member &member : typeStruct->members) {
					// Complete member type
					member.type = completeType(member.type);

					if(member.type->kind == Type::Kind::Procedure) {
						// If the member is a procedure, check to see whether the parent class has an
						// identically-named member
						TypeStruct::Member *parentMember = 0;
						if(typeStruct->parent) {
							parentMember = typeStruct->parent->findMember(member.name);
						}

						if(parentMember) {
							// Set the member's offset to the parent class's member offset
							member.qualifiers |= TypeStruct::Member::QualifierVirtual;
							member.offset = parentMember->offset;
						} else if(member.qualifiers & TypeStruct::Member::QualifierVirtual) {
							// Allocate a new vtable slot for the function
							member.offset = typeStruct->vtableSize;
							typeStruct->vtableSize++;
						}
					} else {
						// Allocate space at the end of the object for the member
						member.offset = typeStruct->allocSize;
						typeStruct->allocSize += member.type->valueSize;
					}
				}

				if(typeStruct->constructor) {
					// Complete the constructor type
					typeStruct->constructor = std::static_pointer_cast<TypeProcedure>(completeType(typeStruct->constructor));
				}

				// Mark this type as complete
				mCompleteTypes.insert(type);

				break;
			}

		case Type::Kind::Dummy:
			{
				std::shared_ptr<TypeDummy> typeDummy = std::static_pointer_cast<TypeDummy>(type);

				// Locate the actual (non-dummy) version of the type
				std::shared_ptr<Type> realType = mTypes->findType(type->name);
				if(!realType) {
					std::stringstream s;
					s << "Type '" << type->name << "' not found.";
					throw EnvironmentError(typeDummy->origin, s.str());
				}

				// Complete the type
				type = completeType(realType);
			}
	}

	mCompletionStack.pop_back();

	return type;
}

/*!
 * \brief Construct a scope for a class type
 * \param typeStruct Class to construct scope for
 * \param globalScope Scope containing globals
 */
void EnvironmentGenerator::constructScope(std::shared_ptr<TypeStruct> &typeStruct, Scope &globalScope)
{
	if(typeStruct->parent) {
		// Ensure the parent scope has been constructed
		if(!typeStruct->parent->scope) {
			constructScope(typeStruct->parent, globalScope);
		}

		// Construct a new scope based on the parent's scope
		std::unique_ptr<Scope> scope = std::make_unique<Scope>(typeStruct);
		typeStruct->scope = scope.get();
		typeStruct->parent->scope->addChild(std::move(scope));
	} else {
		// Construct a new scope based on the global scope
		std::unique_ptr<Scope> scope = std::make_unique<Scope>(typeStruct);
		typeStruct->scope = scope.get();
		globalScope.addChild(std::move(scope));
	}

	// Add symbols for each member of the class
	for(TypeStruct::Member &member : typeStruct->members) {
		std::unique_ptr<Symbol> symbol = std::make_unique<Symbol>(member.type, member.name);
		typeStruct->scope->addSymbol(std::move(symbol));
	}
}

}