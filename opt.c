/* opt.c -- option parsing ($Revision: 1.1.1.1 $) */

#include "es.h"

static const char *usage, *invoker;
static List *args;
static Term *termarg;
static int nextchar;
static Boolean throwonerr;

extern void esoptbegin(List *list, const char *caller, const char *usagemsg, Boolean throws) {
	static Boolean initialized = FALSE;
	if (!initialized) {
		initialized = TRUE;
		globalroot(&usage);
		globalroot(&invoker);
		globalroot(&args);
		globalroot(&termarg);
	}
	es_assert(usage == NULL);
	usage = usagemsg;
	invoker = caller;
	args = list;
	termarg = NULL;
	nextchar = 0;
	throwonerr = throws;
}

extern int esopt(const char *options) {
	int c;
	const char *arg, *opt;

	es_assert(!throwonerr || usage != NULL);
	es_assert(termarg == NULL);
	if (nextchar == 0) {
		if (args == NULL)
			return EOF;
		es_assert(args->term != NULL);
		arg = getstr(args->term);
		if (*arg != '-')
			return EOF;
		if (arg[1] == '-' && arg[2] == '\0') {
			args = args->next;
			return EOF;
		}
		nextchar = 1;
	} else {
		es_assert(args != NULL && args->term != NULL);
		arg = getstr(args->term);
	}

	c = arg[nextchar++];
	opt = strchr(options, c);
	if (opt == NULL) {
		const char *msg = usage;
		usage = NULL;
		args = NULL;
		nextchar = 0;
		if (throwonerr)
			fail(invoker, "illegal option: -%c -- usage: %s", c, msg);
		else return '?';
	}

	if (arg[nextchar] == '\0') {
		nextchar = 0;
		args = args->next;
	}

	if (opt[1] == ':') {
		if (args == NULL) {
			const char *msg = usage;
			if (throwonerr)
				fail(invoker,
				     "option -%c expects an argument -- usage: %s",
				     c, msg);
			else return ':';
		}
		termarg = (nextchar == 0)
				? args->term
				: mkstr(gcdup(arg + nextchar));
		nextchar = 0;
		args = args->next;
	}
	return c;
}

extern Term *esoptarg(void) {
	Term *t = termarg;
	es_assert(t != NULL);
	termarg = NULL;
	return t;
}

extern List *esoptend(void) {
	List *result = args;
	args = NULL;
	usage = NULL;
	return result;
}
