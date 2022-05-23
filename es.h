/* es.h -- definitions for higher order shell */

#include "esconfig.h"
#include "estypes.h"

#include "esbool.h"
#include "stdenv.h"

/*
 * meta-information for exported environment strings
 */

#define ENV_SEPARATOR '\017' /* control-O */
#define ENV_ESCAPE    '\016' /* control-N */

/*
 * our programming environment
 */

/* main.c */

#if GCVERBOSE
extern bool gcverbose; /* -G */
#endif
#if GCINFO
extern bool gcinfo; /* -I */
#endif

/* initial.c (for es) or dump.c (for esdump) */

void runinitial(void);

/* fd.c */

void mvfd(int old, int new);
int  newfd(void);

#define UNREGISTERED (-999)
void registerfd(int *fdp, bool closeonfork);
void unregisterfd(int *fdp);
void releasefd(int fd);
void closefds(void);

int  fdmap(int fd);
int  defer_mvfd(bool parent, int old, int new);
int  defer_close(bool parent, int fd);
void undefer(int ticket);

/* term.c */

#define mkstr(str) (mkterm((str), NULL))
Term    *mkterm(char *str, Closure *closure);
char    *getstr(Term *term);
Closure *getclosure(Term *term);
Term    *termcat(Term *t1, Term *t2);
bool     termeq(Term *term, const char *s);
bool     isclosure(Term *term);

/* list.c */

List *mklist(Term *term, List *next);
List *reverse(List *list);
List *append(List *head, List *tail);
List *listcopy(List *list);
int   length(List *list);
List *listify(int argc, char **argv);
Term *nth(List *list, int n);
List *sortlist(List *list);

/* tree.c */

Tree *mk(NodeKind, ...);

/* closure.c */

Closure *mkclosure(Tree *tree, Binding *binding);
Closure *extractbindings(Tree *tree);
Binding *mkbinding(char *name, List *defn, Binding *next);
Binding *reversebindings(Binding *binding);

/* eval.c */

Binding *bindargs(Tree *params, List *args, Binding *binding);
List    *forkexec(char *file, List *list, bool inchild);
List    *walk(Tree *tree, Binding *binding, int flags);
List    *eval(List *list, Binding *binding, int flags);
List    *eval1(Term *term, int flags);
List    *pathsearch(Term *term);

extern unsigned long evaldepth;
extern unsigned long maxevaldepth;
#define MINmaxevaldepth  100
#define MAXmaxevaldepth  0xffffffffU

#define eval_inchild     1
#define eval_exitonfalse 2
#define eval_flags       (eval_inchild | eval_exitonfalse)

/* glom.c */

List *glom(Tree *tree, Binding *binding, bool globit);
List *glom2(Tree *tree, Binding *binding, StrList **quotep);

/* glob.c */

extern const char *QUOTED;
extern const char *UNQUOTED;

List *glob(List *list, StrList *quote);
bool  haswild(const char *pattern, const char *quoting);

/* match.c */
bool  match(const char *subject, const char *pattern, const char *quote);
bool  listmatch(List *subject, List *pattern, StrList *quote);
List *extractmatches(List *subjects, List *patterns, StrList *quotes);

/* var.c */

void    initvars(void);
void    initenv(char **envp, bool protected);
void    hidevariables(void);
void    validatevar(const char *var);
List   *varlookup(const char *name, Binding *binding);
List   *varlookup2(char *name1, char *name2, Binding *binding);
void    vardef(char *, Binding *, List *);
Vector *mkenv(void);
void    setnoexport(List *list);
void    addtolist(void *arg, char *key, void *value);
List   *listvars(bool internal);

typedef struct Push Push;
extern Push        *pushlist;
void                varpush(Push *, char *, List *);
void                varpop(Push *);

/* status.c */

extern List *ltrue;
extern List *lfalse;

bool  istrue(List *status);
int   exitstatus(List *status);
char *mkstatus(int status);
void  printstatus(pid_t pid, int status);

/* access.c */

char *checkexecutable(char *file);

/* proc.c */

extern bool hasforked;

int efork(bool parent, bool background);
int ewait(pid_t pid, bool interruptible, struct rusage *rusage);
#define ewaitfor(pid) ewait(pid, false, NULL)

/* dict.c */

Dict *mkdict(void);
void  dictforall(Dict *dict, void (*proc)(void *, char *, void *), void *arg);
void *dictget(Dict *dict, const char *name);
Dict *dictput(Dict *dict, char *name, void *value);
void *dictget2(Dict *dict, const char *name1, const char *name2);

/* conv.c */

void initconv(void);

/* print.c -- see print.h for more */

int            print(const char *fmt, ...);
int            eprint(const char *fmt, ...);
int            fprint(int fd, const char *fmt, ...);
_Noreturn void panic(const char *fmt, ...);

/* str.c */

char    *str(const char *fmt, ...);    /* create a gc space string by printing */
char    *mprint(const char *fmt, ...); /* create an ealloc space string by printing */
StrList *mkstrlist(char *, StrList *);

/* vec.c */

Vector *mkvector(int n);
Vector *vectorize(List *list);
void    sortvector(Vector *v);

/* util.c */

char *esstrerror(int err);
void  uerror(char *msg);
void *ealloc(size_t n);
void *erealloc(void *p, size_t n);
void  efree(void *p);
void  ewrite(int fd, const char *s, size_t n);
long  eread(int fd, char *buf, size_t n);
bool  isabsolute(char *path);
bool  streq2(const char *s, const char *t1, const char *t2);

/* input.c */

extern char *prompt;
extern char *prompt2;

Tree *parse(char *esprompt1, char *esprompt2);
Tree *parsestring(const char *str);
void  sethistory(char *file);
bool  isinteractive(void);
void  initinput(void);
void  resetparser(void);
#if HAVE_READLINE
void initgetenv(void);
#endif

List *runfd(int fd, const char *name, int flags);
List *runstring(const char *str, const char *name, int flags);

/* eval_* flags are also understood as runflags */
#define run_interactive 4  /* -i or $0[0] = '-' */
#define run_noexec      8  /* -n */
#define run_echoinput   16 /* -v */
#define run_printcmds   32 /* -x */
#define run_lisptrees   64 /* -L and defined(LISPTREES) */

#if HAVE_READLINE
extern bool resetterminal;
#endif

/* opt.c */

void  esoptbegin(List *list, const char *caller, const char *usage);
int   esopt(const char *options);
Term *esoptarg(void);
List *esoptend(void);

/* prim.c */

List *prim(char *s, List *list, Binding *binding, int evalflags);
void  initprims(void);

/* split.c */

void  startsplit(const char *sep, bool coalesce);
void  splitstring(char *in, size_t len, bool endword);
List *endsplit(void);
List *fsplit(const char *sep, List *list, bool coalesce);

/* signal.c */

int   signumber(const char *name);
char *signame(int sig);
char *sigmessage(int sig);

#define SIGCHK() sigchk()
typedef enum {
	sig_nochange,
	sig_catch,
	sig_default,
	sig_ignore,
	sig_noop,
	sig_special
} Sigeffect;

Sigeffect esignal(int sig, Sigeffect effect);
void      setsigeffects(const Sigeffect effects[]);
void      getsigeffects(Sigeffect effects[]);
void      initsignals(bool interactive, bool allowdumps);
void      sigchk(void);
bool      issilentsignal(List *e);
void      setsigdefaults(void);
void      blocksignals(void);
void      unblocksignals(void);

List *mksiglist(void);

extern Atomic     slow;
extern Atomic     interrupted;
extern sigjmp_buf slowlabel;
extern bool       sigint_newline;

/* open.c */

typedef enum {
	oOpen,
	oCreate,
	oAppend,
	oReadWrite,
	oReadCreate,
	oReadAppend
} OpenKind;
int eopen(char *name, OpenKind k);

/* version.c */

extern const char version[];

/* gc.c -- see gc.h for more */

typedef struct Tag Tag;
#define gcnew(type) ((type *)gcalloc(sizeof(type), &(CONCAT(type, Tag))))

void *gcalloc(size_t n, Tag *t);       /* allocate n with collection tag t */
char *gcdup(const char *s);            /* copy a 0-terminated string into gc space */
char *gcndup(const char *s, size_t n); /* copy a counted string into gc space */

void initgc(void);             /* must be called at the dawn of time */
void gc(void);                 /* provoke a collection, if enabled */
void gcreserve(size_t nbytes); /* provoke a collection, if enabled and not enough space */
void gcenable(void);           /* enable collections */
void gcdisable(void);          /* disable collections */
bool gcisblocked();            /* is collection disabled? */

/*
 * garbage collector tags
 */

typedef struct Root Root;
struct Root {
	void **p;
	Root  *next;
};

extern Root *rootlist;

#if REF_ASSERTIONS
#	define refassert(e) assert(e)
#else
#	define refassert(e) NOP
#endif

/* clang-format off */
#define Ref(t, v, init)                           \
	if (0)                                        \
		;                                         \
	else {                                        \
		t v = init;                               \
		Root(CONCAT(v, __root__));                \
		(CONCAT(v, __root__)).p    = (void **)&v; \
		(CONCAT(v, __root__)).next = rootlist;    \
		rootlist                   = &(CONCAT(v, __root__))
#define RefPop(v)                                      \
		refassert(rootlist == &(CONCAT(v, __root__))); \
		refassert(rootlist->p == (void **)&v);         \
		rootlist = rootlist->next;
#define RefEnd(v)  \
		RefPop(v); \
	}
#define RefReturn(v) \
		RefPop(v);   \
		return v;    \
	}
#define RefAdd(e)                    \
	if (0)                           \
		;                            \
	else {                           \
		Root __root__;               \
		__root__.p    = (void **)&e; \
		__root__.next = rootlist;    \
		rootlist      = &__root__
#define RefRemove(e)                           \
		refassert(rootlist == &__root__);      \
		refassert(rootlist->p == (void **)&e); \
		rootlist = rootlist->next;             \
	}
/* clang-format on */

#define RefEnd2(v1, v2) \
	RefEnd(v1);         \
	RefEnd(v2)
#define RefEnd3(v1, v2, v3) \
	RefEnd(v1);             \
	RefEnd2(v2, v3)
#define RefEnd4(v1, v2, v3, v4) \
	RefEnd(v1);                 \
	RefEnd3(v2, v3, v4)

#define RefPop2(v1, v2) \
	RefPop(v1);         \
	RefPop(v2)
#define RefPop3(v1, v2, v3) \
	RefPop(v1);             \
	RefPop2(v2, v3)
#define RefPop4(v1, v2, v3, v4) \
	RefPop(v1);                 \
	RefPop3(v2, v3, v4)

#define RefAdd2(v1, v2) \
	RefAdd(v1);         \
	RefAdd(v2)
#define RefAdd3(v1, v2, v3) \
	RefAdd(v1);             \
	RefAdd2(v2, v3)
#define RefAdd4(v1, v2, v3, v4) \
	RefAdd(v1);                 \
	RefAdd3(v2, v3, v4)

#define RefRemove2(v1, v2) \
	RefRemove(v1);         \
	RefRemove(v2)
#define RefRemove3(v1, v2, v3) \
	RefRemove(v1);             \
	RefRemove2(v2, v3)
#define RefRemove4(v1, v2, v3, v4) \
	RefRemove(v1);                 \
	RefRemove3(v2, v3, v4)

void globalroot(void *addr);

/* struct Push -- varpush() placeholder */

struct Push {
	Push *next;
	char *name;
	List *defn;
	int   flags;
	Root  nameroot;
	Root  defnroot;
};

/*
 * exception handling
 *
 *	ExceptionHandler
 *		... body ...
 *	CatchException (e)
 *		... catching code using e ...
 *	EndExceptionHandler
 *
 */

typedef struct Handler Handler;
struct Handler {
	Handler      *up;
	Root		 *rootlist;
	Push		 *pushlist;
	unsigned long evaldepth;
	sigjmp_buf    label;
};

extern Handler *tophandler;
extern Handler *roothandler;
extern List    *exception;

void           pophandler(Handler *handler);
_Noreturn void fire(List *exc);
_Noreturn void fail(const char *from, const char *name, ...);
void           newchildcatcher(void);

#if DEBUG_EXCEPTIONS
List *raised(List *e);
#else
#	define raised(e) (e)
#endif

/* clang-format off */
#define ExceptionHandler                          \
	{                                             \
		Handler _localhandler;                    \
		_localhandler.rootlist  = rootlist;       \
		_localhandler.pushlist  = pushlist;       \
		_localhandler.evaldepth = evaldepth;      \
		_localhandler.up        = tophandler;     \
		tophandler              = &_localhandler; \
		if (!sigsetjmp(_localhandler.label, 1)) {
#define CatchExceptionIf(condition, e)      \
			if (condition)                  \
				pophandler(&_localhandler); \
		}                                   \
		else {                              \
			List *e = raised(exception);
#define CatchException(e) CatchExceptionIf(true, (e))
#define EndExceptionHandler \
		}                   \
	}
