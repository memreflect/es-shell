/* term.h -- definition of term structure */

#include "estypes.h"

struct Term {
	char *str;
	Closure *closure;
};
