#ifndef PARSE_NODE_H
#define PARSE_NODE_H

typedef enum {
	ParseNodeStatementList,
	ParseNodePrint,
	ParseNodeVarDecl,
	ParseNodeAssign,
	ParseNodeEqual,
	ParseNodeAdd,
	ParseNodeMultiply,
	ParseNodeInt,
	ParseNodeId
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