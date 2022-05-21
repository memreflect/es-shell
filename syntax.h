/* syntax.h -- abstract syntax tree interface */

#include "estypes.h"

#include "esbool.h"

#define CAR u[0].p
#define CDR u[1].p

/* syntax.c */

extern Tree errornode;

Tree *treecons(Tree *car, Tree *cdr);
Tree *treecons2(Tree *car, Tree *cdr);
Tree *treeconsend(Tree *p, Tree *q);
Tree *treeconsend2(Tree *p, Tree *q);
Tree *treeappend(Tree *head, Tree *tail);
Tree *thunkify(Tree *tree);

Tree *prefix(char *s, Tree *t);
Tree *backquote(Tree *ifs, Tree *body);
Tree *flatten(Tree *t, char *sep);
Tree *fnassign(Tree *name, Tree *defn);
Tree *mklambda(Tree *params, Tree *body);
Tree *mkseq(char *op, Tree *t1, Tree *t2);
Tree *mkpipe(Tree *t1, int outfd, int infd, Tree *t2);

Tree *mkclose(int fd);
Tree *mkdup(int fd0, int fd1);
Tree *redirect(Tree *t);
Tree *mkredir(Tree *cmd, Tree *file);
Tree *mkredircmd(char *cmd, int fd);
Tree *redirappend(Tree *t, Tree *r);
Tree *firstprepend(Tree *first, Tree *args);

Tree *mkmatch(Tree *subj, Tree *cases);

/* heredoc.c */

bool readheredocs(bool endfile);
bool queueheredoc(Tree *t);
