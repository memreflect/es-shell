/* input.h -- definitions for es lexical analyzer */

#include "estypes.h"

#include <stddef.h>

#include "esbool.h"

#define MAXUNGET 2 /* maximum 2 character pushback */

typedef struct Input Input;
typedef int          InputMethod(Input *self);
typedef void         InputCleanup(Input *self);
struct Input {
	InputMethod   *get;
	InputMethod   *fill;
	InputMethod   *rfill;
	InputCleanup  *cleanup;
	Input         *prev;
	const char    *name;
	unsigned char *buf;
	unsigned char *bufend;
	unsigned char *bufbegin;
	unsigned char *rbuf;
	size_t         buflen;
	int            unget[MAXUNGET];
	int            ungot;
	int            lineno;
	int            fd;
	int            runflags;
};

#define GETC()    (*input->get)(input)
#define UNGETC(c) unget(input, c)

/* input.c */

extern Input *input;
extern bool   disablehistory;

void unget(Input *in, int c);
void yyerror(char *s);

/* token.c */

extern const char dnw[];

int  yylex(void);
void inityy(void);
void print_prompt2(void);

/* parse.y */

extern Tree *parsetree;

int  yyparse(void);
void initparse(void);

/* heredoc.c */

void emptyherequeue(void);
