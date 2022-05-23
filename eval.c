/* eval.c -- evaluation of lists and trees */

#include <errno.h>

#include <unistd.h>

#include "es.h"

unsigned long evaldepth    = 0;
unsigned long maxevaldepth = MAXmaxevaldepth;

static _Noreturn void
failexec(char *file, List *args) {
	List *fn;
	assert(gcisblocked());
	fn = varlookup("fn-%exec-failure", NULL);
	if (fn != NULL) {
		int olderror = errno;
		Ref(List *, list, append(fn, mklist(mkstr(file), args)));
		RefAdd(file);
		gcenable();
		RefRemove(file);
		eval(list, NULL, 0);
		RefEnd(list);
		errno = olderror;
	}
	eprint("%s: %s\n", file, esstrerror(errno));
	exit(1);
}

/* forkexec -- fork (if necessary) and exec */
List *
forkexec(char *file, List *list, bool inchild) {
	pid_t   pid;
	int     status;
	Vector *env;
	gcdisable();
	env = mkenv();
	pid = efork(!inchild, false);
	if (pid == 0) {
		execve(file, vectorize(list)->vector, env->vector);
		failexec(file, list);
	}
	gcenable();
	status = ewaitfor(pid);
	if ((status & 0xff) == 0) {
		sigint_newline = false;
		SIGCHK();
		sigint_newline = true;
	} else
		SIGCHK();
	printstatus(0, status);
	return mklist(mkterm(mkstatus(status), NULL), NULL);
}

/* assign -- bind a list of values to a list of variables */
static List *
assign(Tree *varform, Tree *valueform0, Binding *binding0) {
	Ref(List *, result, NULL);

	Ref(Tree *, valueform, valueform0);
	Ref(Binding *, binding, binding0);
	Ref(List *, vars, glom(varform, binding, false));

	if (vars == NULL)
		fail("es:assign", "null variable name");

	Ref(List *, values, glom(valueform, binding, true));
	result = values;

	for (; vars != NULL; vars = vars->next) {
		List *value;
		Ref(char *, name, getstr(vars->term));
		if (values == NULL)
			value = NULL;
		else if (vars->next == NULL || values->next == NULL) {
			value  = values;
			values = NULL;
		} else {
			value  = mklist(values->term, NULL);
			values = values->next;
		}
		vardef(name, binding, value);
		RefEnd(name);
	}

	RefEnd4(values, vars, binding, valueform);
	RefReturn(result);
}

/* letbindings -- create a new Binding containing let-bound variables */
static Binding *
letbindings(Tree *defn0, Binding *outer0, Binding *context0, int evalflags) {
	Ref(Binding *, binding, outer0);
	Ref(Binding *, context, context0);
	Ref(Tree *, defn, defn0);

	for (; defn != NULL; defn = defn->u[1].p) {
		assert(defn->kind == nList);
		if (defn->u[0].p == NULL)
			continue;

		Ref(Tree *, assign, defn->u[0].p);
		assert(assign->kind == nAssign);
		Ref(List *, vars, glom(assign->u[0].p, context, false));
		Ref(List *, values, glom(assign->u[1].p, context, true));

		if (vars == NULL)
			fail("es:let", "null variable name");

		for (; vars != NULL; vars = vars->next) {
			List *value;
			Ref(char *, name, getstr(vars->term));
			if (values == NULL)
				value = NULL;
			else if (vars->next == NULL || values->next == NULL) {
				value  = values;
				values = NULL;
			} else {
				value  = mklist(values->term, NULL);
				values = values->next;
			}
			binding = mkbinding(name, value, binding);
			RefEnd(name);
		}

		RefEnd3(values, vars, assign);
	}

	RefEnd2(defn, context);
	RefReturn(binding);
}

/* localbind -- recursively convert a Bindings list into dynamic binding */
static List *
localbind(Binding *dynamic0, Binding *lexical0, Tree *body0, int evalflags) {
	if (dynamic0 == NULL)
		return walk(body0, lexical0, evalflags);
	else {
		Push p;
		Ref(List *, result, NULL);
		Ref(Tree *, body, body0);
		Ref(Binding *, dynamic, dynamic0);
		Ref(Binding *, lexical, lexical0);

		varpush(&p, dynamic->name, dynamic->defn);
		result = localbind(dynamic->next, lexical, body, evalflags);
		varpop(&p);

		RefEnd3(lexical, dynamic, body);
		RefReturn(result);
	}
}

/* local -- build, recursively, one layer of local assignment */
static List *
local(Tree *defn, Tree *body0, Binding *bindings0, int evalflags) {
	Ref(List *, result, NULL);
	Ref(Tree *, body, body0);
	Ref(Binding *, bindings, bindings0);
	Ref(Binding *, dynamic, reversebindings(letbindings(defn, NULL, bindings, evalflags)));

	result = localbind(dynamic, bindings, body, evalflags);

	RefEnd3(dynamic, bindings, body);
	RefReturn(result);
}

/* forloop -- evaluate a for loop */
static List *
forloop(Tree *defn0, Tree *body0, Binding *binding, int evalflags) {
	static List MULTIPLE = {NULL, NULL};

	Ref(List *, result, ltrue);
	Ref(Binding *, outer, binding);
	Ref(Binding *, looping, NULL);
	Ref(Tree *, body, body0);

	Ref(Tree *, defn, defn0);
	for (; defn != NULL; defn = defn->u[1].p) {
		assert(defn->kind == nList);
		if (defn->u[0].p == NULL)
			continue;
		Ref(Tree *, assign, defn->u[0].p);
		assert(assign->kind == nAssign);
		Ref(List *, vars, glom(assign->u[0].p, outer, false));
		Ref(List *, list, glom(assign->u[1].p, outer, true));
		if (vars == NULL)
			fail("es:for", "null variable name");
		for (; vars != NULL; vars = vars->next) {
			char *var = getstr(vars->term);
			looping   = mkbinding(var, list, looping);
			list      = &MULTIPLE;
		}
		RefEnd3(list, vars, assign);
		SIGCHK();
	}
	looping = reversebindings(looping);
	RefEnd(defn);

	ExceptionHandler

		for (;;) {
			bool allnull = true;
			Ref(Binding *, bp, outer);
			Ref(Binding *, lp, looping);
			Ref(Binding *, sequence, NULL);
			for (; lp != NULL; lp = lp->next) {
				Ref(List *, value, NULL);
				if (lp->defn != &MULTIPLE)
					sequence = lp;
				assert(sequence != NULL);
				if (sequence->defn != NULL) {
					value          = mklist(sequence->defn->term,
                                   NULL);
					sequence->defn = sequence->defn->next;
					allnull        = false;
				}
				bp = mkbinding(lp->name, value, bp);
				RefEnd(value);
			}
			RefEnd2(sequence, lp);
			if (allnull) {
				RefPop(bp);
				break;
			}
			result = walk(body, bp, evalflags & eval_exitonfalse);
			RefEnd(bp);
			SIGCHK();
		}

	CatchException(e)

		if (!termeq(e->term, "break"))
			fire(e);
		result = e->next;

	EndExceptionHandler

	RefEnd3(body, looping, outer);
	RefReturn(result);
}

/* matchpattern -- does the text match a pattern? */
static List *
matchpattern(Tree *subjectform0, Tree *patternform0, Binding *binding) {
	bool     result;
	List    *pattern;
	StrList *quote = NULL;
	Ref(Binding *, bp, binding);
	Ref(Tree *, patternform, patternform0);
	Ref(List *, subject, glom(subjectform0, bp, true));
	pattern = glom2(patternform, bp, &quote);
	result  = listmatch(subject, pattern, quote);
	RefEnd3(subject, patternform, bp);
	return result ? ltrue : lfalse;
}

/* extractpattern -- Like matchpattern, but returns matches */
static List *
extractpattern(Tree *subjectform0, Tree *patternform0, Binding *binding) {
	List    *pattern;
	StrList *quote = NULL;
	Ref(List *, result, NULL);
	Ref(Binding *, bp, binding);
	Ref(Tree *, patternform, patternform0);
	Ref(List *, subject, glom(subjectform0, bp, true));
	pattern = glom2(patternform, bp, &quote);
	result  = (List *)extractmatches(subject, pattern, quote);
	RefEnd3(subject, patternform, bp);
	RefReturn(result);
}

/* walk -- walk through a tree, evaluating nodes */
List *
walk(Tree *tree0, Binding *binding0, int flags) {
	Tree *volatile tree       = tree0;
	Binding *volatile binding = binding0;

	SIGCHK();

top:
	if (tree == NULL)
		return ltrue;

	switch (tree->kind) {
	case nConcat:
	case nList:
	case nQword:
	case nVar:
	case nVarsub:
	case nWord:
	case nThunk:
	case nLambda:
	case nCall:
	case nPrim: {
		List *list;
		Ref(Binding *, bp, binding);
		list    = glom(tree, binding, true);
		binding = bp;
		RefEnd(bp);
		return eval(list, binding, flags);
	}

	case nAssign:
		return assign(tree->u[0].p, tree->u[1].p, binding);

	case nLet:
	case nClosure:
		Ref(Tree *, body, tree->u[1].p);
		binding = letbindings(tree->u[0].p, binding, binding, flags);
		tree    = body;
		RefEnd(body);
		goto top;

	case nLocal:
		return local(tree->u[0].p, tree->u[1].p, binding, flags);

	case nFor:
		return forloop(tree->u[0].p, tree->u[1].p, binding, flags);

	case nMatch:
		return matchpattern(tree->u[0].p, tree->u[1].p, binding);

	case nExtract:
		return extractpattern(tree->u[0].p, tree->u[1].p, binding);

	default:
		panic("walk: bad node kind %d", tree->kind);
	}
	NOTREACHED;
}

/* bindargs -- bind an argument list to the parameters of a lambda */
Binding *
bindargs(Tree *params, List *args, Binding *binding) {
	if (params == NULL)
		return mkbinding("*", args, binding);

	gcdisable();

	for (; params != NULL; params = params->u[1].p) {
		Tree *param;
		List *value;
		assert(params->kind == nList);
		param = params->u[0].p;
		assert(param->kind == nWord || param->kind == nQword);
		if (args == NULL)
			value = NULL;
		else if (params->u[1].p == NULL || args->next == NULL) {
			value = args;
			args  = NULL;
		} else {
			value = mklist(args->term, NULL);
			args  = args->next;
		}
		binding = mkbinding(param->u[0].s, value, binding);
	}

	Ref(Binding *, result, binding);
	gcenable();
	RefReturn(result);
}

/* pathsearch -- evaluate fn %pathsearch + some argument */
List *
pathsearch(Term *term) {
	List *search;
	List *list;
	search = varlookup("fn-%pathsearch", NULL);
	if (search == NULL)
		fail("es:pathsearch", "%E: fn %%pathsearch undefined", term);
	list = mklist(term, NULL);
	return eval(append(search, list), NULL, 0);
}

/* eval -- evaluate a list, producing a list */
List *
eval(List *list0, Binding *binding0, int flags) {
	Closure *volatile cp;
	List *fn;

	if (++evaldepth >= maxevaldepth)
		fail("es:eval", "max-eval-depth exceeded");

	Ref(List *, list, list0);
	Ref(Binding *, binding, binding0);
	Ref(char *, funcname, NULL);

restart:
	if (list == NULL) {
		RefPop3(funcname, binding, list);
		--evaldepth;
		return ltrue;
	}
	assert(list->term != NULL);

	if ((cp = getclosure(list->term)) != NULL) {
		switch (cp->tree->kind) {
		case nPrim:
			assert(cp->binding == NULL);
			list = prim(cp->tree->u[0].s, list->next, binding, flags);
			break;
		case nThunk:
			list = walk(cp->tree->u[0].p, cp->binding, flags);
			break;
		case nLambda:
			ExceptionHandler

				Push p;
				Ref(Tree *, tree, cp->tree);
				Ref(Binding *, context, bindargs(tree->u[0].p, list->next, cp->binding));
				if (funcname != NULL)
					varpush(&p, "0", mklist(mkterm(funcname, NULL), NULL));
				list = walk(tree->u[1].p, context, flags);
				if (funcname != NULL)
					varpop(&p);
				RefEnd2(context, tree);

			CatchException(e)

				if (termeq(e->term, "return")) {
					list = e->next;
					goto done;
				}
				fire(e);

			EndExceptionHandler
			break;
		case nList: {
			list = glom(cp->tree, cp->binding, true);
			list = append(list, list->next);
			goto restart;
		}
		default:
			panic("eval: bad closure node kind %d",
			      cp->tree->kind);
		}
		goto done;
	}

	/* the logic here is duplicated in $&whatis */

	Ref(char *, name, getstr(list->term));
	fn = varlookup2("fn-", name, binding);
	if (fn != NULL) {
		funcname = name;
		list     = append(fn, list->next);
		RefPop(name);
		goto restart;
	}
	if (isabsolute(name)) {
		char *error = checkexecutable(name);
		if (error != NULL)
			fail("$&whatis", "%s: %s", name, error);
		list = forkexec(name, list, flags & eval_inchild);
		RefPop(name);
		goto done;
	}
	RefEnd(name);

	fn = pathsearch(list->term);
	if (fn != NULL && fn->next == NULL
	    && (cp = getclosure(fn->term)) == NULL) {
		char *name = getstr(fn->term);
		list       = forkexec(name, list, flags & eval_inchild);
		goto done;
	}

	list = append(fn, list->next);
	goto restart;

done:
	--evaldepth;
	if ((flags & eval_exitonfalse) && !istrue(list))
		exit(exitstatus(list));
	RefEnd2(funcname, binding);
	RefReturn(list);
}

/* eval1 -- evaluate a term, producing a list */
List *
eval1(Term *term, int flags) {
	return eval(mklist(term, NULL), NULL, flags);
}
