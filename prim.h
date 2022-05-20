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

extern Dict  *initprims_controlflow(Dict *primdict); /* prim-ctl.c */
extern Dict  *initprims_io(Dict *primdict);          /* prim-io.c */
extern Dict  *initprims_etc(Dict *primdict);         /* prim-etc.c */
extern Dict  *initprims_sys(Dict *primdict);         /* prim-sys.c */
extern Dict  *initprims_proc(Dict *primdict);        /* proc.c */
extern Dict  *initprims_access(Dict *primdict);      /* access.c */
