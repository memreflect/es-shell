/* conv.c -- convert between internal and external forms ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "print.h"

#include <wchar.h>
#include <wctype.h>


/* %L -- print a list */
static Boolean Lconv(Format *f) {
	List *lp, *next;
	char *sep;
	const char *fmt = (f->flags & FMT_altform) ? "%S%s" : "%s%s";

	lp = va_arg(f->args, List *);
	sep = va_arg(f->args, char *);
	for (; lp != NULL; lp = next) {
		next = lp->next;
		fmtprint(f, fmt, getstr(lp->term), next == NULL ? "" : sep);
	}
	return FALSE;
}

/* treecount -- count the number of nodes in a flattened tree list */
static int treecount(Tree *tree) {
	return tree == NULL
		 ? 0
		 : tree->kind == nList
		    ? treecount(tree->u[0].p) + treecount(tree->u[1].p)
		    : 1;
}

/* binding -- print a binding statement */
static void binding(Format *f, char *keyword, Tree *tree) {
	Tree *np;
	char *sep = "";
	fmtprint(f, "%s(", keyword);
	for (np = tree->u[0].p; np != NULL; np = np->u[1].p) {
		Tree *binding;
		assert(np->kind == nList);
		binding = np->u[0].p;
		assert(binding != NULL);
		assert(binding->kind == nAssign);
		fmtprint(f, "%s%#T=%T", sep, binding->u[0].p, binding->u[1].p);
		sep = ";";
	}
	fmtprint(f, ")");
}

/* %T -- print a tree */
static Boolean Tconv(Format *f) {
	Tree *n = va_arg(f->args, Tree *);
	Boolean group = (f->flags & FMT_altform) != 0;


#define	tailcall(tree, altform) \
	STMT(n = (tree); group = (altform); goto top)

top:
	if (n == NULL) {
		if (group)
			fmtcat(f, "()");
		return FALSE;
	}

	switch (n->kind) {

	case nWord:
		fmtprint(f, "%s", n->u[0].s);
		return FALSE;

	case nQword:
		fmtprint(f, "%#S", n->u[0].s);
		return FALSE;

	case nPrim:
		fmtprint(f, "$&%s", n->u[0].s);
		return FALSE;

	case nAssign:
		fmtprint(f, "%#T=", n->u[0].p);
		tailcall(n->u[1].p, FALSE);

	case nConcat:
		fmtprint(f, "%#T^", n->u[0].p);
		tailcall(n->u[1].p, TRUE);

	case nMatch:
		fmtprint(f, "~ %#T%s", n->u[0].p, (n->u[1].p != NULL ? " " : ""));
		tailcall(n->u[1].p, FALSE);

	case nExtract:
		fmtprint(f, "~~ %#T%s", n->u[0].p, (n->u[1].p != NULL ? " " : ""));
		tailcall(n->u[1].p, FALSE);

	case nThunk:
		fmtprint(f, "{%T}", n->u[0].p);
		return FALSE;

	case nVarsub:
		fmtprint(f, "$%#T(%T)", n->u[0].p, n->u[1].p);
		return FALSE;

	case nLocal:
		binding(f, "local", n);
		tailcall(n->u[1].p, FALSE);

	case nLet:
		binding(f, "let", n);
		tailcall(n->u[1].p, FALSE);

	case nFor:
		binding(f, "for", n);
		tailcall(n->u[1].p, FALSE);

	case nClosure:
		binding(f, "%closure", n);
		tailcall(n->u[1].p, FALSE);

	case nCall: {
		Tree *t = n->u[0].p;
		fmtprint(f, "<=");
		if (t != NULL && (t->kind == nThunk || t->kind == nPrim))
			tailcall(t, FALSE);
		fmtprint(f, "{%T}", t);
		return FALSE;
	}

	case nVar:
		fmtputc(f, '$');
		n = n->u[0].p;
		switch (treecount(n)) {
		case 0: default:
			tailcall(n, TRUE);
		case 1:
			if (n->kind == nWord || n->kind == nQword ||
				(n->kind == nList &&
				 (n->u[0].p->kind == nWord || n->u[0].p->kind == nQword)))
				goto top;
			fmtprint(f, "(%#T)", n);
		}
		return FALSE;

	case nLambda:
		fmtprint(f, "@ ");
		if (n->u[0].p == NULL)
			fmtprint(f, "*");
		else
			fmtprint(f, "%T", n->u[0].p);
		fmtprint(f, "{%T}", n->u[1].p);
		return FALSE;

	case nList:
		if (!group) {
			for (; n->u[1].p != NULL; n = n->u[1].p)
				fmtprint(f, "%T ", n->u[0].p);
			n = n->u[0].p;
			goto top;
		}
		switch (treecount(n)) {
		case 0:
			fmtcat(f, "()");
			break;
		case 1:
			fmtprint(f, "%T%T", n->u[0].p, n->u[1].p);
			break;
		default:
			fmtprint(f, "(%T", n->u[0].p);
			while ((n = n->u[1].p) != NULL) {
				assert(n->kind == nList);
				fmtprint(f, " %T", n->u[0].p);
			}
			fmtputc(f, ')');
		}
		return FALSE;

	default:
		panic("bad node kind: %d", n->kind);

	}
	NOTREACHED;
}

/* enclose -- build up a closure */
static void enclose(Format *f, Binding *binding, const char *sep) {
	if (binding != NULL) {
		Binding *next = binding->next;
		enclose(f, next, ";");
		fmtprint(f, "%S=%#L%s", binding->name, binding->defn, " ", sep);
	}
}

#if 0
typedef struct Chain Chain;
struct Chain {
	Closure *closure;
	Chain *next;
};
static Chain *chain = NULL;
#endif

/* %C -- print a closure */
static Boolean Cconv(Format *f) {
	Closure *closure = va_arg(f->args, Closure *);
	Tree *tree = closure->tree;
	Binding *binding = closure->binding;
	Boolean altform = (f->flags & FMT_altform) != 0;

#if 0
	int i;
	Chain me, *cp;
	assert(tree->kind == nThunk || tree->kind == nLambda || tree->kind == nPrim);
	assert(binding == NULL || tree->kind != nPrim);

	for (cp = chain, i = 0; cp != NULL; cp = cp->next, i++)
		if (cp->closure == closure) {
			fmtprint(f, "%d $&nestedbinding", i);
			return FALSE;
		}
	me.closure = closure;
	me.next = chain;
	chain = &me;
#endif

	if (altform)
		fmtprint(f, "%S", str("%C", closure));
	else {
		if (binding != NULL) {
			fmtprint(f, "%%closure(");
			enclose(f, binding, "");
			fmtprint(f, ")");
		}
		fmtprint(f, "%T", tree);
	}

#if 0
	chain = chain->next;	/* TODO: exception unwinding? */
#endif
	return FALSE;
}

/* %E -- print a term */
static Boolean Econv(Format *f) {
	Term *term = va_arg(f->args, Term *);
	Closure *closure = getclosure(term);

	if (closure != NULL)
		fmtprint(f, (f->flags & FMT_altform) ? "%#C" : "%C", closure);
	else
		fmtprint(f, (f->flags & FMT_altform) ? "%S" : "%s", getstr(term));
	return FALSE;
}

/* bisprint -- return whether a single-byte char is printable */
static Boolean bisprint(int c, size_t *n) {
	/* *n matches semantics of mbrtowc() in chisprint(). */
	*n = (c != '\0');
	return isprint(c);
}

/* chisprint -- return whether a (possibly multibyte) character is printable */
static Boolean chisprint(const unsigned char *s, size_t *n) {
#if !HAVE_MBRTOWC
	return bisprint(*s, n);
#else
	mbstate_t mbs;
	wchar_t wc;

	/* avoid issues on systems using single-byte character sets/encodings
	 * where iswprint() is potentially useless.
	 */
	if (MB_CUR_MAX == 1)
		return bisprint(*s, n);

	memset(&mbs, 0, sizeof mbs);
	*n = mbrtowc(&wc, (const char *)s, MB_CUR_MAX, &mbs);
	if (*n >= (size_t)-2)
		return FALSE;
	return iswprint(wc);
#endif
}

/* subprint -- print an initial substring of unquoted, quoted, or escaped bytes */
static const unsigned char *subprint(Format *f, const unsigned char *s) {
	/* this classifies an initial substring of 's'
	 * as an unquoted, quoted, or backslash-escaped sequence of bytes
	 * and prints the substring accordingly.
	 *
	 * note: Sconv() correctly handles the empty string,
	 *       so this function assumes it received a non-empty string.
	 */
	enum { Unquoted, Quoted, Escape } state;
	const unsigned char *end;
	extern const char nw[];
	size_t n;

	/* determine whether the first byte sequence is printable.
	 * if it is printable, determine whether it requires quoting.
	 */
	if (!chisprint(s, &n)) {
		state = Escape;
		if (n >= (size_t)-2)
			n = 1;
	} else if ((f->flags & FMT_altform) || *s == '@' || nw[*s])
		state = Quoted;
	else
		state = Unquoted;

	/* initialize "end" pointer.
	 * if first byte sequence was not printable, "end" is iterated up to the
	 * next printable byte sequence, which is where the substring of
	 * non-printable chars actually ends.
	 * similarly, if it was printable, "end" is iterated up to the next
	 * non-printable byte sequence, and if any printable char in that
	 * substring requires quoting, the entire substring must be quoted.
	 */
	end = &s[n];
	if (state == Unquoted) {
		for (; chisprint(end, &n); end += n)
			if ((f->flags & FMT_altform) || *end == '@' || nw[*end]) {
				end += n;
				state = Quoted;
				break;
			}
	}
	if (state == Quoted)
		while (chisprint(end, &n))
			end += n;
	else if (state == Escape) {
		while (!chisprint(end, &n) && n != 0)
			if (n >= (size_t)-2)
				end++;
			else
				end += n;
	}

	/* print the substring appropriately. */
	switch (state) {
	case Unquoted:
		for (; s != end; s++)
			fmtputc(f, *s);
		break;
	case Quoted:
		fmtputc(f, '\'');
		for (; s != end; s++) {
			if (*s == '\'')
				fmtputc(f, '\'');
			fmtputc(f, *s);
		}
		fmtputc(f, '\'');
		break;
	case Escape:
		switch (*s) {
		case '\a': fmtprint(f, "\\a"); break;
		case '\b': fmtprint(f, "\\b"); break;
		case '\f': fmtprint(f, "\\f"); break;
		case '\n': fmtprint(f, "\\n"); break;
		case '\r': fmtprint(f, "\\r"); break;
		case '\t': fmtprint(f, "\\t"); break;
		case '\033': fmtprint(f, "\\e"); break;
		default: fmtprint(f, "\\%o", *s); break;
		}
		/* while Sconv() prints the '^' for us, doing things that way
		 * means unnecessary calls to subprint() when n > 1 already
		 * implies there is more than one byte to print.
		 * for the sake of performance, we print '^' before additional
		 * bytes here when necessary.
		 */
		while (++s != end) {
			switch (*s) {
			case '\a': fmtprint(f, "^\\a"); break;
			case '\b': fmtprint(f, "^\\b"); break;
			case '\f': fmtprint(f, "^\\f"); break;
			case '\n': fmtprint(f, "^\\n"); break;
			case '\r': fmtprint(f, "^\\r"); break;
			case '\t': fmtprint(f, "^\\t"); break;
			case '\033': fmtprint(f, "^\\e"); break;
			default: fmtprint(f, "^\\%o", *s); break;
			}
		}
		break;
	}

	/* return pointer to next byte sequence, printable or not */
	return s;
}

/* %S -- print a string with conservative quoting rules */
static Boolean Sconv(Format *f) {
	const unsigned char *s;

	s = va_arg(f->args, const unsigned char *);
	if (*s == '\0')
		fmtprint(f, "''");
	else
		/* if the result of subprint() does not point to a null byte,
		 * then there are more bytes to process.
		 * '^' is printed between printable and nonprintable substrings.
		 */
		while (*(s = subprint(f, s)) != '\0')
			fmtputc(f, '^');
	return FALSE;
}

/* %Z -- print a StrList */
static Boolean Zconv(Format *f) {
	StrList *lp, *next;
	char *sep;

	lp = va_arg(f->args, StrList *);
	sep = va_arg(f->args, char *);
	for (; lp != NULL; lp = next) {
		next = lp->next;
		fmtprint(f, "%s%s", lp->str, next == NULL ? "" : sep);
	}
	return FALSE;
}

/* %F -- protect an exported name from brain-dead shells */
static Boolean Fconv(Format *f) {
	int c;
	unsigned char *next;

	/* 0=encode as __XX, 1=ASCII alphabetic, 2=ASCII digit */
	static const char xdw[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/*   0 -  15 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/*  16 -  32 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* ' ' - '/' */
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0,		/* '0' - '?' */
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		/* '@' - 'O' */
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,		/* 'P' - '_' */
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		/* '`' - 'o' */
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,		/* 'p' - DEL */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 128 - 143 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 144 - 159 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 160 - 175 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 176 - 191 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 192 - 207 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 208 - 223 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 224 - 239 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 240 - 255 */
	};
	/* praw1 - char is [A-Za-z] or char is '_' and not followed by '_'
	   praw  - char may also be [0-9] */
#define praw1	(xdw[c] == 1 || (c == '_' && *next != '_'))
#define praw	(xdw[c] != 0 || (c == '_' && *next != '_'))

	next = va_arg(f->args, unsigned char *);
	c = *next++;

	if (praw1)
		fmtputc(f, c);
	else
		fmtprint(f, "__%02x", c);
	while ((c = *next++) != '\0')
		if (praw)
			fmtputc(f, c);
		else
			fmtprint(f, "__%02x", c);
	return FALSE;
}

/* %N -- undo %F */
static Boolean Nconv(Format *f) {
	int c;
	unsigned char *s = va_arg(f->args, unsigned char *);

	while ((c = *s++) != '\0') {
		if (c == '_' && *s == '_') {
			static const char hexchar[] = "0123456789abcdef";
			const char *h1 = strchr(hexchar, s[1]);
			const char *h2 = strchr(hexchar, s[2]);
			if (h1 != NULL && h2 != NULL) {
				c = ((h1 - hexchar) << 4) | (h2 - hexchar);
				s += 3;
			}
		}
		fmtputc(f, c);
	}
	return FALSE;
}

/* %W -- print a list for exporting to the environment, merging and quoting */
static Boolean Wconv(Format *f) {
	List *lp, *next;

	for (lp = va_arg(f->args, List *); lp != NULL; lp = next) {
		int c;
		const char *s;
		for (s = getstr(lp->term); (c = *s) != '\0'; s++) {
			if (c == ENV_ESCAPE || c == ENV_SEPARATOR)
				fmtputc(f, ENV_ESCAPE);
			fmtputc(f, c);
		}
		next = lp->next;
		if (next != NULL)
			fmtputc(f, ENV_SEPARATOR);
	}
	return FALSE;
}


#if LISPTREES
static Boolean Bconv(Format *f) {
	Tree *n = va_arg(f->args, Tree *);
	if (n == NULL) {
		fmtprint(f, "nil");
		return FALSE;
	}
	switch (n->kind) {

	case nWord:
		fmtprint(f, "(word \"%s\")", n->u[0].s);
		break;

	case nQword:
		fmtprint(f, "(qword \"%s\")", n->u[0].s);
		break;

	case nPrim:
		fmtprint(f, "(prim %s)", n->u[0].s);
		break;

	case nCall:
		fmtprint(f, "(call %B)", n->u[0].p);
		break;

	case nThunk:
		fmtprint(f, "(thunk %B)", n->u[0].p);
		break;

	case nVar:
		fmtprint(f, "(var %B)", n->u[0].p);
		break;

	case nAssign:
		fmtprint(f, "(assign %B %B)", n->u[0].p, n->u[1].p);
		break;

	case nConcat:
		fmtprint(f, "(concat %B %B)", n->u[0].p, n->u[1].p);
		break;

	case nClosure:
		fmtprint(f, "(%%closure %B %B)", n->u[0].p, n->u[1].p);
		break;

	case nFor:
		fmtprint(f, "(for %B %B)", n->u[0].p, n->u[1].p);
		break;

	case nLambda:
		fmtprint(f, "(lambda %B %B)", n->u[0].p, n->u[1].p);
		break;

	case nLet:
		fmtprint(f, "(let %B %B)", n->u[0].p, n->u[1].p);
		break;

	case nLocal:
		fmtprint(f, "(local %B %B)", n->u[0].p, n->u[1].p);
		break;

	case nMatch:
		fmtprint(f, "(match %B %B)", n->u[0].p, n->u[1].p);
		break;

	case nExtract:
		fmtprint(f, "(extract %B %B)", n->u[0].p, n->u[1].p);
		break;

	case nRedir:
		fmtprint(f, "(redir %B %B)", n->u[0].p, n->u[1].p);
		break;

	case nVarsub:
		fmtprint(f, "(varsub %B %B)", n->u[0].p, n->u[1].p);
		break;

	case nPipe:
		fmtprint(f, "(pipe %d %d)", n->u[0].i, n->u[1].i);
		break;

	case nList: {
		fmtprint(f, "(list");
		do {
			assert(n->kind == nList);
			fmtprint(f, " %B", n->u[0].p);
		} while ((n = n->u[1].p) != NULL);
		fmtprint(f, ")");
		break;
	}

	}
	return FALSE;
}
#endif

/* install the conversion routines */
void initconv(void) {
	fmtinstall('C', Cconv);
	fmtinstall('E', Econv);
	fmtinstall('F', Fconv);
	fmtinstall('L', Lconv);
	fmtinstall('N', Nconv);
	fmtinstall('S', Sconv);
	fmtinstall('T', Tconv);
	fmtinstall('W', Wconv);
	fmtinstall('Z', Zconv);
#if LISPTREES
	fmtinstall('B', Bconv);
#endif
}
