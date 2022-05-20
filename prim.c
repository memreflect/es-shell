/* prim.c -- primitives and primitive dispatching */

#include "prim.h"

#include <stddef.h>

#include "es.h"

static Dict *prims;

extern List *
prim(char *s, List *list, Binding *binding, int evalflags) {
	PrimFunc *p;
	p = (PrimFunc *)dictget(prims, s);
	if (p == NULL)
		fail("es:prim", "unknown primitive: %s", s);
	return (*p)(list, binding, evalflags);
}

PRIM(primitives) {
	static List *primlist = NULL;
	if (primlist == NULL) {
		globalroot(&primlist);
		dictforall(prims, addtolist, &primlist);
		primlist = sortlist(primlist);
	}
	return primlist;
}

extern void
initprims(void) {
	prims = mkdict();
	globalroot(&prims);

	prims = initprims_controlflow(prims);
	prims = initprims_io(prims);
	prims = initprims_etc(prims);
	prims = initprims_sys(prims);
	prims = initprims_proc(prims);
	prims = initprims_access(prims);

#define primdict prims
	X(primitives);
}
