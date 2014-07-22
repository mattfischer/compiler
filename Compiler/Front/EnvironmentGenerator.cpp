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
	EnvironmentError(Node *node, const std::string &message)
		: mNode(node), mMessage(message)
	{}

	Node *node() { return mNode; } //!< Node of error
	const std::string &message() { return mMessage; } //!< Error message

private:
	Node *mNode; //!< Node
	std::string mMessage; //!< Error message
};

EnvironmentGenerator::EnvironmentGenerator(Node *tree)
{
	try {
		Types *types = new Types;
		Scope *scope = new Scope;

		// Loop through the tree and pre-populate all declared type names
		for(unsigned int i=0; i<tree->children.size(); i++) {
			Node *node = tree->children[i];
			std::stringstream s;

			if(node->nodeType == Node::NodeTypeStructDef) {
				TypeStruct *type = new TypeStruct(Type::TypeStruct, node->lexVal.s);

				if(!types->registerType(type)) {
					s << "Redefinition of structure " << type->name;
					throw EnvironmentError(node, s.str());
				}
			} else if(node->nodeType == Node::NodeTypeClassDef) {
				TypeStruct *type = new TypeStruct(Type::TypeClass, node->lexVal.s);

				if(!types->registerType(type)) {
					s << "Redefinition of class " << type->name;
					throw EnvironmentError(node, s.str());
				}
			}
		}

		// Iterate through the tree's procedure definitions
		std::vector<Node*> classes;
		for(unsigned int i=0; i<tree->children.size(); i++) {
			Node *node = tree->children[i];

			switch(node->nodeType) {
				case Node::NodeTypeProcedureDef:
					addProcedure(node, types, scope);
					break;
				case Node::NodeTypeStructDef:
					addStruct(node, types);
					break;
				case Node::NodeTypeClassDef:
					classes.push_back(node);
					break;
			}
		}
		addClasses(classes, types, scope);

		mTypes = types;
		mScope = scope;
	} catch(EnvironmentError error) {
		// Collect the error message and line from the exception
		mTypes = 0;
		mScope = 0;
		mErrorLine = error.node()->line;
		mErrorMessage = error.message();
	}
}

/*!
 * \brief Generate a single procedure
 * \param node Tree node for procedure definition
 * \param program Program to add procedure to
 * \param scope Parent scope of procedure
 * \return New procedure
 */
TypeProcedure *EnvironmentGenerator::addProcedure(Node *node, Types *types, Scope *scope)
{
	// Iterate the tree's argument items
	Node *argumentList = node->children[1];
	std::vector<Type*> argumentTypes;
	for(unsigned int j=0; j<argumentList->children.size(); j++) {
		// Construct the argument type, and add it to the list of types
		Type *argumentType = createType(argumentList->children[j]->children[0], types);
		if(Type::equals(argumentType, Types::intrinsic(Types::Void))) {
			throw EnvironmentError(argumentList->children[j], "Cannot declare procedure argument of type void");
		}
		argumentTypes.push_back(argumentType);
	}

	// Construct the procedure type
	Type *returnType;
	if(node->children[0]) {
		returnType = createType(node->children[0], types);
	} else {
		returnType = Types::intrinsic(Types::Void);
	}

	TypeProcedure *procedureType = new TypeProcedure(returnType, argumentTypes);

	// Construct the procedure symbol
	Symbol *symbol = new Symbol(procedureType, node->lexVal.s);
	scope->addSymbol(symbol);

	return procedureType;
}

/*!
 * \brief Add a structure to the type list
 * \param node Node describing structure
 * \param program Program to add to
 */
void EnvironmentGenerator::addStruct(Node *node, Types *types)
{
	TypeStruct *type = (Front::TypeStruct*)types->findType(node->lexVal.s);
	type->parent = 0;
	type->scope = 0;
	type->constructor = 0;
	Node *members = node->children[0];
	for(unsigned int i=0; i<members->children.size(); i++) {
		Node *memberNode = members->children[i];
		Type *memberType = createType(memberNode->children[0], types);
		type->addMember(memberType, memberNode->lexVal.s, false);
	}
}

/*!
 * \brief Add a class to the type list
 * \param node Node describing structure
 * \param program Program to add to
 */
void EnvironmentGenerator::addClass(Node *node, Types *types, Scope *scope)
{
	TypeStruct *type = (Front::TypeStruct*)types->findType(node->lexVal.s);

	type->constructor = 0;

	if(node->children.size() == 2) {
		type->parent = (Front::TypeStruct*)types->findType(node->children[0]->lexVal.s);
		type->scope = new Scope(type->parent->scope, type);
		type->allocSize = type->parent->allocSize;
		type->vtableSize = type->parent->vtableSize;
		type->vtableOffset = type->parent->vtableOffset;
	} else {
		type->parent = 0;
		type->vtableSize = 0;
		type->vtableOffset = 0;
		type->scope = new Scope(scope, type);
	}

	Node *members = node->children[node->children.size() - 1];
	for(unsigned int i=0; i<members->children.size(); i++) {
		Node *memberNode = members->children[i];
		switch(memberNode->nodeType) {
			case Node::NodeTypeVarDecl:
			{
				Type *memberType = createType(memberNode->children[0], types);
				type->addMember(memberType, memberNode->lexVal.s, false);
				type->scope->addSymbol(new Symbol(memberType, memberNode->lexVal.s));
				break;
			}

			case Node::NodeTypeProcedureDef:
			case Node::NodeTypeVirtual:
			{
				Node *procedureNode;
				bool virtualFunction;
				switch(memberNode->nodeType) {
					case Node::NodeTypeProcedureDef:
						procedureNode = memberNode;
						if(type->parent && type->parent->findMember(procedureNode->lexVal.s)) {
							virtualFunction = true;
						} else {
							virtualFunction = false;
						}
						break;

					case Node::NodeTypeVirtual:
						procedureNode = memberNode->children[0];
						virtualFunction = true;
						break;
				}

				TypeProcedure *procedureType = addProcedure(procedureNode, types, type->scope);
				if(procedureNode->lexVal.s == type->name) {
					type->constructor = procedureType;
				} else if(types->findType(procedureNode->lexVal.s)) {
					throw EnvironmentError(procedureNode, "Illegal procedure name " + procedureNode->lexVal.s);
				} else {
					type->addMember(procedureType, procedureNode->lexVal.s, virtualFunction);
				}

				break;
			}
		}
	}
}

/*!
* \brief Populate all classes, sorting according to inheritance order
* \param nodes Class nodes
* \brief program Program to add to
*/
void EnvironmentGenerator::addClasses(std::vector<Node*> nodes, Types *types, Scope *scope)
{
	std::set<Type*> processed;
	std::vector<Node*> nextNodes;

	// Loop repeatedly through the list of class nodes until all are processed
	while(nodes.size() > 0) {
		for(unsigned int i=0; i<nodes.size(); i++) {
			Node *node = nodes[i];

			// If the class has a parent, check to see if it is processed
			if(node->children.size() == 2) {
				std::string parent = node->children[0]->lexVal.s;
				Type *type = types->findType(parent);
				if(!type) {
					throw EnvironmentError(node, "Invalid class parent " + parent);
				}

				// If the parent is not processed, this node cannot be processed yet
				if(processed.find(type) == processed.end()) {
					nextNodes.push_back(node);
					continue;
				}
			}

			// Add the class, and record that it has been processed
			addClass(node, types, scope);
			Type *type = types->findType(node->lexVal.s);
			processed.insert(type);
		}

		// Check if any progress was made during this iteration
		if(nextNodes.size() == nodes.size()) {
			// If no classes were processed, there is an inheritance cycle
			throw EnvironmentError(nodes[0], "Cycle in parents of class " + nodes[0]->lexVal.s);
		} else {
			// Otherwise, repeat the procedure with any classes that couldn't be processed this time
			nodes = nextNodes;
			nextNodes.clear();
		}
	}
}

/*!
 * \brief Search for a type named by a node
 * \param node Node describing type
 * \return Type
 */
Type *EnvironmentGenerator::createType(Node *node, Types *types)
{
	Type *type = 0;
	if(node->nodeType == Node::NodeTypeArray) {
		type = createType(node->children[0], types);
		if(Type::equals(type, Types::intrinsic(Types::Void))) {
			throw EnvironmentError(node, "Cannot declare array of voids");
		}
		type = new TypeArray(type);
	} else {
		std::string name = node->lexVal.s;
		type = types->findType(name);
		if(!type) {
			std::stringstream s;
			s << "Type '" << name << "' not found.";
			throw EnvironmentError(node, s.str());
		}
	}

	return type;
}

}
