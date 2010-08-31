#ifndef PARSE_NODE_H
#define PARSE_NODE_H

typedef enum {
	ParseNodeStatementList,
	ParseNodePrint,
	ParseNodeMultiply,
	ParseNodeAdd,
	ParseNodeInt
} ParseNodeType;

struct _ParseNode;
typedef struct _ParseNode ParseNode;
struct _ParseNode {
	ParseNodeType type;
	int numChildren;
	ParseNode **children;
	union {
		int _int;
	} lexVal;
};

#endif