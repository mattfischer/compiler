#ifndef PARSE_NODE_H
#define PARSE_NODE_H

typedef enum {
	ParseNodeProcedureList,
	ParseNodeProcedure,
	ParseNodeStatementList,
	ParseNodePrint,
	ParseNodeVarDecl,
	ParseNodeAssign,
	ParseNodeIf,
	ParseNodeWhile,
	ParseNodeEqual,
	ParseNodeNequal,
	ParseNodeAdd,
	ParseNodeMultiply,
	ParseNodeInt,
	ParseNodeId,
	ParseNodeCall,
	ParseNodeReturn,
	ParseNodeArgDeclList,
	ParseNodeArgumentDecl
} ParseNodeType;

struct _ParseNode;
typedef struct _ParseNode ParseNode;
struct _ParseNode {
	ParseNodeType type;
	int line;
	int numChildren;
	ParseNode **children;
	union {
		int _int;
		char *_id;
	} lexVal;
};

#endif