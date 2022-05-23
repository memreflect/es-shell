/* prim-ctl.c -- control flow primitives */

#include "es.h"
#include "prim.h"

PRIM(seq) {
	Ref(List *, result, ltrue);
	Ref(List *, lp, list);
	for (; lp != NULL; lp = lp->next)
		result = eval1(lp->term, evalflags & ~(lp->next == NULL ? 0 : eval_inchild));
	RefEnd(lp);
	RefReturn(result);
}

PRIM(if) {
	Ref(List *, lp, list);
	for (; lp != NULL; lp = lp->next) {
		List *cond;
		cond = eval1(lp->term, evalflags & (lp->next == NULL ? eval_inchild : 0));
		lp   = lp->next;
		if (lp == NULL) {
			RefPop(lp);
			return cond;
		}
		if (istrue(cond)) {
			List *result = eval1(lp->term, evalflags);
			RefPop(lp);
			return result;
		}
	}
	RefEnd(lp);
	return ltrue;
}

PRIM(forever) {
	Ref(List *, body, list);
	for (;;)
		list = eval(body, NULL, evalflags & eval_exitonfalse);
	RefEnd(body);
	return list;
}

PRIM(throw) {
	if (list == NULL)
		fail("$&throw", "usage: throw exception [args ...]");
	fire(list);
	NOTREACHED;
}

PRIM(catch) {
	Atomic retry;

	if (list == NULL)
		fail("$&catch", "usage: catch catcher body");

	Ref(List *, result, NULL);
	Ref(List *, lp, list);

	do {
		retry = false;

		ExceptionHandler

			result = eval(lp->next, NULL, evalflags);

		CatchException(frombody)

			blocksignals();
			ExceptionHandler
				result
						= eval(mklist(mkstr("$&noreturn"),
				                      mklist(lp->term, frombody)),
				               NULL,
				               evalflags);
				unblocksignals();
			CatchException(fromcatcher)

				if (termeq(fromcatcher->term, "retry")) {
					retry = true;
					unblocksignals();
				} else {
					unblocksignals();
					fire(fromcatcher);
				}
			EndExceptionHandler

		EndExceptionHandler
	} while (retry);
	RefEnd(lp);
	RefReturn(result);
}

Dict *
initprims_controlflow(Dict *primdict) {
	X(seq);
	X(if);
	X(throw);
	X(forever);
	X(catch);
	return primdict;
}
