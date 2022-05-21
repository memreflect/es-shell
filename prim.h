/* prim.h -- definitions for es primitives */

#include "estypes.h"

#define PRIM(name) \
	static List *  \
	CONCAT(prim_, name)(List * list, Binding * binding, int evalflags)
#define X(name)                       \
	(primdict = dictput(primdict,     \
	                    STRING(name), \
	                    (void *)CONCAT(prim_, name)))

typedef List *PrimFunc(List *list, Binding *binding, int evalflags);

Dict *initprims_controlflow(Dict *primdict); /* prim-ctl.c */
Dict *initprims_io(Dict *primdict);          /* prim-io.c */
Dict *initprims_etc(Dict *primdict);         /* prim-etc.c */
Dict *initprims_sys(Dict *primdict);         /* prim-sys.c */
Dict *initprims_proc(Dict *primdict);        /* proc.c */
Dict *initprims_access(Dict *primdict);      /* access.c */
