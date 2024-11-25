#include "es.h"
#include "prim.h"
#include "prims.h"

static int prim_cmp(const void *a, const void *b) {
	const struct prim *aprim = a;
	const struct prim *bprim = b;
	return strcmp(aprim->name, bprim->name);
}

extern void addprim(const char *name, List *(*impl)(List *, Binding *, int)) {
	struct prim *p, key = {0, 0};
	key.name = (char *)name;
	p = bsearch(&key, prims, nprims, sizeof key, prim_cmp);
	assert(p != NULL && p->impl == 0);
	p->impl = impl;
}

static void sortprims(void) {
	qsort(prims, nprims, sizeof prims[0], prim_cmp);
}

static void validateprims(void) {
	size_t i;
	for (i = 0; i < nprims; i++)
		assert(prims[i].impl != 0);
}

static List *(*getprim(const char *name))(List *, Binding *, int) {
	struct prim *p, key = {0, 0};
	key.name = (char *)name;
	p = bsearch(&key, prims, nprims, sizeof key, prim_cmp);
	if (p == NULL)
		return NULL;
	return p->impl;
}

extern List *prim(char *s, List *list, Binding *binding, int evalflags) {
	List *(*p)(List *, Binding *, int);
	p = getprim(s);
	if (p == NULL)
		fail("es:prim", "unknown primitive: %s", s);
	return (*p)(list, binding, evalflags);
}

extern List *primswithprefix(char *prefix) {
	size_t lo, mid, hi;
	Ref(List *, matching, NULL);
	for (lo = 0, hi = nprims; lo < hi;) {
		mid = (lo + hi) / 2;
		if (strncmp(prims[mid].name, prefix, strlen(prefix)) < 0)
			lo = mid + 1;
		else
			hi = mid;
	}
	for (; lo < nprims && strneq(prims[lo].name, prefix, strlen(prefix)); lo++)
		addtolist(&matching, prims[lo].name, NULL);
	RefReturn(matching);
}

PRIM(primitives) {
	static List *primlist = NULL;
	if (primlist == NULL) {
		size_t i;
		globalroot(&primlist);
		for (i = 0; i < nprims; i++)
			addtolist(&primlist, prims[i].name, NULL);
		primlist = reverse(primlist);
	}
	return primlist;
}

extern void initprims(void) {
	globalroot(&prims);
	sortprims();

	initprims_controlflow();
	initprims_io();
	initprims_etc();
	initprims_sys();
	initprims_proc();
	initprims_access();
	X(primitives);

	validateprims();
}
