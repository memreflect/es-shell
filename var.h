/* var.h -- es variables */

#include "estypes.h"

typedef struct Var Var;
struct Var {
	List *defn;
	char *env;
	int flags;
};

#define	var_hasbindings		1
#define	var_isinternal		2

extern Dict *vars;
