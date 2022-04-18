/* estypes.h -- type definitions */
#ifndef ES_ESTYPES_H
#define ES_ESTYPES_H 1

typedef struct Dict Dict;
typedef struct Tree Tree;
typedef struct Term Term;
typedef struct List List;
typedef struct Binding Binding;
typedef struct Closure Closure;
typedef struct StrList StrList;

struct List {
	Term *term;
	List *next;
};

struct Binding {
	char *name;
	List *defn;
	Binding *next;
};

struct Closure {
	Binding	*binding;
	Tree *tree;
};

typedef enum {
	nAssign, nCall, nClosure, nConcat, nFor, nLambda, nLet, nList, nLocal,
	nMatch, nExtract, nPrim, nQword, nThunk, nVar, nVarsub, nWord,
	nRedir, nPipe		/* only appear during construction */
} NodeKind;

struct Tree {
	NodeKind kind;
	union {
		Tree *p;
		char *s;
		int i;
	} u[2];
};

struct StrList {
	char *str;
	StrList *next;
};

typedef struct {
	int alloclen, count;
	char *vector[1];
} Vector;			/* environment or arguments */

#endif  /* !ES_ESTYPES_H */
