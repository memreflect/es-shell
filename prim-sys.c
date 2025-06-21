/* prim-sys.c -- system call primitives ($Revision: 1.2 $) */

#define	REQUIRE_IOCTL	1

#include "es.h"
#include "prim.h"

#include <locale.h>

#ifdef HAVE_SETRLIMIT
# define BSD_LIMITS 1
#else
# define BSD_LIMITS 0
#endif

#if BSD_LIMITS || BUILTIN_TIME
#include <sys/time.h>
#include <sys/resource.h>
#if !HAVE_GETRUSAGE
#include <sys/times.h>
#include <limits.h>
#endif
#endif

#include <sys/stat.h>

PRIM(newpgrp) {
	int e;
	pid_t pgid;
	if (list != NULL)
		fail("$&newpgrp", "usage: newpgrp");
	pgid = spgrp(getpid());
	if ((e = tctakepgrp()) != 0) {
		spgrp(pgid);
		fail("$&newpgrp", "newpgrp: %s", esstrerror(e));
	}
	return ltrue;
}

PRIM(background) {
	int pid = efork(TRUE, TRUE);
	if (pid == 0) {
#if JOB_PROTECT
		/* job control safe version: put it in a new pgroup, if interactive. */
		if (isinteractive())
			setpgid(0, 0);
#endif
		mvfd(eopen("/dev/null", oOpen), 0);
		esexit(exitstatus(eval(list, NULL, evalflags | eval_inchild)));
	}
	return mklist(mkstr(str("%d", pid)), NULL);
}

PRIM(fork) {
	int pid, status;
	pid = efork(TRUE, FALSE);
	if (pid == 0)
		esexit(exitstatus(eval(list, NULL, evalflags | eval_inchild)));
	status = ewaitfor(pid);
	SIGCHK();
	printstatus(0, status);
	return mklist(mkstr(mkstatus(status)), NULL);
}

PRIM(run) {
	char *file;
	if (list == NULL)
		fail("$&run", "usage: %%run file argv0 argv1 ...");
	Ref(List *, lp, list);
	file = getstr(lp->term);
	lp = forkexec(file, lp->next, (evalflags & eval_inchild) != 0);
	RefReturn(lp);
}

PRIM(umask) {
	if (list == NULL) {
		int mask = umask(0);
		umask(mask);
		print("%04o\n", mask);
		return ltrue;
	}
	if (list->next == NULL) {
		int mask;
		char *s, *t;
		s = getstr(list->term);
		mask = strtol(s, &t, 8);
		if ((t != NULL && *t != '\0') || ((unsigned) mask) > 07777)
			fail("$&umask", "bad umask: %s", s);
		umask(mask);
		return ltrue;
	}
	fail("$&umask", "usage: umask [mask]");
	NOTREACHED;
}

PRIM(cd) {
	char *dir;
	if (list == NULL || list->next != NULL)
		fail("$&cd", "usage: $&cd directory");
	dir = getstr(list->term);
	if (chdir(dir) == -1)
		fail("$&cd", "chdir %s: %s", dir, esstrerror(errno));
	return ltrue;
}

PRIM(setsignals) {
	int i;
	Sigeffect effects[NSIG];
	for (i = 0; i < NSIG; i++)
		effects[i] = sig_default;
	Ref(List *, lp, list);
	for (; lp != NULL; lp = lp->next) {
		int sig;
		const char *s = getstr(lp->term);
		Sigeffect effect = sig_catch;
		switch (*s) {
		case '-':	effect = sig_ignore;	s++; break;
		case '/':	effect = sig_noop;	s++; break;
		case '.':	effect = sig_special;	s++; break;
		}
		sig = signumber(s);
		if (sig < 0)
			fail("$&setsignals", "unknown signal: %s", s);
		effects[sig] = effect;
	}
	RefEnd(lp);
	blocksignals();
	setsigeffects(effects);
	unblocksignals();
	return mksiglist();
}

/* keep the order of these synchronized with locale_info */
enum {
	/* must come first */
	LI_LANG,
	LI_ALL,

	/* add locale categories here */
	LI_COLLATE,
	LI_CTYPE,
	LI_MESSAGES,

	/* must come last */
	LI_END
};
static struct locale_info {
	char *name;
	char *value;
	int   lc;
} locale_info[] = {
	{"LANG",	NULL, LC_ALL},
	{"LC_ALL",	NULL, LC_ALL},
	{"LC_COLLATE",	NULL, LC_COLLATE},
	{"LC_CTYPE",	NULL, LC_CTYPE},
	{"LC_MESSAGES", NULL, LC_MESSAGES},
};

static void effect_locale(const struct locale_info *const p) {
	if (p->value != NULL)
		setlocale(p->lc, p->value);
	else if (locale_info[LI_LANG].value != NULL)
		setlocale(p->lc, locale_info[LI_LANG].value);
	else
		setlocale(p->lc, "C");
}

PRIM(getlocale) {
	int c;
	struct locale_info *lip;
	char *value = locale_info[LI_ALL].value;
	Boolean real = FALSE;

	Ref(List *, result, NULL);
	Ref(List *, args, list);
	esoptbegin(args, "$&getlocale", "$&getlocale [-r]", TRUE);
	while ((c = esopt("r")) != EOF)
		real = TRUE;
	esoptend();
	RefEnd(args);
	if (value != NULL && !real) {
		result = mklist(mkstr(str("%s=%s",
		                          locale_info[LI_ALL].name,
		                          value)),
		                result);
		for (lip = &locale_info[LI_END-1]; lip->lc != LC_ALL; lip--)
			result = mklist(mkstr(str("%s=%s", lip->name, value)),
			                result);
	} else {
		if (value == NULL)
			value = "";
		result = mklist(mkstr(str("%s=%s",
		                          locale_info[LI_ALL].name,
		                          value)),
		                result);
		for (lip = &locale_info[LI_END-1]; lip->lc != LC_ALL; lip--) {
			value = lip->value;
			if (value == NULL) {
				value = "";
				if (!real) {
					value = locale_info[LI_LANG].value;
					if (value == NULL)
						value = "C";
				}
			}
			result = mklist(mkstr(str("%s=%s", lip->name, value)),
			                result);
		}
	}
	value = locale_info[LI_LANG].value;
	if (value == NULL)
		value = "";
	result = mklist(mkstr(str("%s=%s",
	                          locale_info[LI_LANG].name,
	                          value)),
	                result);
	RefReturn(result);
}

PRIM(setlocale) {
#define exit_usage() \
	fail("$&setlocale", "usage: $&setlocale {LANG|LC_COLLATE|LC_CTYPE|LC_MESSAGES|LC_ALL} locale...")

	char *var;
	struct locale_info *lip;
	if (list == NULL)
		exit_usage();

	Ref(List *, result, NULL);
	Ref(List *, lp, list);
	var = getstr(lp->term);
	for (lip = &locale_info[0]; lip != &locale_info[LI_END]; lip++)
		if (streq(lip->name, var))
			break;
	if (lip == &locale_info[LI_END])
		exit_usage();
	lp = lp->next;

	if (lp == NULL)
		lip->value = NULL;
	else {
		for (; lp != NULL; lp = lp->next) {
			size_t plen;
			char *p = getstr(lp->term);
			if (*p == '\0')
				continue;
			p = setlocale(LC_ALL, p);
			if (p == NULL)
				continue;
			plen = 1+strlen(p);
			lip->value = erealloc(lip->value, plen);
			memcpy(lip->value, p, plen);
			result = mklist(mkstr(lip->value), NULL);
			break;
		}
		if (result == NULL)
			fail("$&setlocale", "no valid locales provided -- tried %#L", list->next, " ");
	}
	if (locale_info[LI_ALL].value != NULL)
		setlocale(LC_ALL, locale_info[LI_ALL].value);
	else {
		effect_locale(&locale_info[LI_COLLATE]);
		effect_locale(&locale_info[LI_CTYPE]);
		effect_locale(&locale_info[LI_MESSAGES]);
	}

	RefEnd(lp);
	RefReturn(result);
}

/*
 * limit builtin -- this is too much code for what it gives you
 */

#if BSD_LIMITS
typedef struct Suffix Suffix;
struct Suffix {
	const char *name;
	long amount;
	const Suffix *next;
};

static const Suffix sizesuf[] = {
	{ "g",	1024*1024*1024,	sizesuf + 1 },
	{ "m",	1024*1024,	sizesuf + 2 },
	{ "k",	1024,		NULL },
};

static const Suffix timesuf[] = {
	{ "h",	60 * 60,	timesuf + 1 },
	{ "m",	60,		timesuf + 2 },
	{ "s",	1,		NULL },
};

typedef struct {
	char *name;
	int flag;
	const Suffix *suffix;
} Limit;

static const Limit limits[] = {

	{ "cputime",		RLIMIT_CPU,	timesuf },
	{ "filesize",		RLIMIT_FSIZE,	sizesuf },
	{ "datasize",		RLIMIT_DATA,	sizesuf },
	{ "stacksize",		RLIMIT_STACK,	sizesuf },
	{ "coredumpsize",	RLIMIT_CORE,	sizesuf },

#ifdef RLIMIT_RSS	/* SysVr4 does not have this */
	{ "memoryuse",		RLIMIT_RSS,	sizesuf },
#endif
#ifdef RLIMIT_VMEM	/* instead, they have this! */
	{ "memorysize",		RLIMIT_VMEM,	sizesuf },
#endif

#ifdef RLIMIT_MEMLOCK	/* 4.4bsd adds an unimplemented limit on non-pageable memory */
	{ "lockedmemory",	RLIMIT_CORE,	sizesuf },
#endif

#ifdef RLIMIT_NOFILE	/* SunOS 4.1 adds a limit on file descriptors */
	{ "descriptors",	RLIMIT_NOFILE,	NULL },
#elif defined(RLIMIT_OFILE) /* but 4.4bsd uses this name for it */
	{ "descriptors",	RLIMIT_OFILE,	NULL },
#endif

#ifdef RLIMIT_NPROC	/* 4.4bsd adds a limit on child processes */
	{ "processes",		RLIMIT_NPROC,	NULL },
#endif

	{ NULL, 0, NULL }
};

static void printlimit(const Limit *limit, Boolean hard) {
	struct rlimit rlim;
	LIMIT_T lim;
	getrlimit(limit->flag, &rlim);
	if (hard)
		lim = rlim.rlim_max;
	else
		lim = rlim.rlim_cur;
	if (lim == (LIMIT_T) RLIM_INFINITY)
		print("%-8s\tunlimited\n", limit->name);
	else {
		const Suffix *suf;

		for (suf = limit->suffix; suf != NULL; suf = suf->next)
			if (lim % suf->amount == 0 && (lim != 0 || suf->amount > 1)) {
				lim /= suf->amount;
				break;
			}
		print("%-8s\t%d%s\n", limit->name, (int)lim, (suf == NULL || lim == 0) ? "" : suf->name);
	}
}

static long parselimit(const Limit *limit, char *s) {
	long lim;
	char *t;
	const Suffix *suf = limit->suffix;
	if (streq(s, "unlimited"))
		return RLIM_INFINITY;
	if (memchr("0123456789", *s, 10) == NULL)
		fail("$&limit", "%s: bad limit value", s);
	if (suf == timesuf && (t = strchr(s, ':')) != NULL) {
		char *u;
		lim = strtol(s, &u, 0) * 60;
		if (u != t)
			fail("$&limit", "%s %s: bad limit value", limit->name, s);
		lim += strtol(u + 1, &t, 0);
		if (t != NULL && *t == ':')
			lim = lim * 60 + strtol(t + 1, &t, 0);
		if (t != NULL && *t != '\0')
			fail("$&limit", "%s %s: bad limit value", limit->name, s);
	} else {
		lim = strtol(s, &t, 0);
		if (t != NULL && *t != '\0')
			for (;; suf = suf->next) {
				if (suf == NULL)
					fail("$&limit", "%s %s: bad limit value", limit->name, s);
				if (streq(suf->name, t)) {
					lim *= suf->amount;
					break;
				}
			}
	}
	return lim;
}

PRIM(limit) {
	const Limit *lim = limits;
	Boolean hard = FALSE;
	Ref(List *, lp, list);

	if (lp != NULL && streq(getstr(lp->term), "-h")) {
		hard = TRUE;
		lp = lp->next;
	}

	if (lp == NULL)
		for (; lim->name != NULL; lim++)
			printlimit(lim, hard);
	else {
		char *name = getstr(lp->term);
		for (;; lim++) {
			if (lim->name == NULL)
				fail("$&limit", "%s: no such limit", name);
			if (streq(name, lim->name))
				break;
		}
		lp = lp->next;
		if (lp == NULL)
			printlimit(lim, hard);
		else {
			long n;
			struct rlimit rlim;
			getrlimit(lim->flag, &rlim);
			if ((n = parselimit(lim, getstr(lp->term))) < 0)
				fail("$&limit", "%s: bad limit value", getstr(lp->term));
			if (hard)
				rlim.rlim_max = (LIMIT_T)n;
			else
				rlim.rlim_cur = (LIMIT_T)n;
			if (setrlimit(lim->flag, &rlim) == -1)
				fail("$&limit", "setrlimit: %s", esstrerror(errno));
		}
	}
	RefEnd(lp);
	return ltrue;
}
#endif	/* BSD_LIMITS */

#if BUILTIN_TIME
#if HAVE_GETRUSAGE
/* This function is provided as timersub(3) on some systems, but it's simple enough
 * to do ourselves. */
static void timesub(struct timeval *a, struct timeval *b, struct timeval *res) {
	res->tv_sec = a->tv_sec - b->tv_sec;
	res->tv_usec = a->tv_usec - b->tv_usec;
	if (res->tv_usec < 0) {
		res->tv_sec -= 1;
		res->tv_usec += 1000000;
	}
}
#endif

PRIM(time) {
#if HAVE_GETRUSAGE

	int pid, status;
	time_t t0, t1;
	struct rusage ru_prev, ru_new, ru_diff;

	Ref(List *, lp, list);

	getrusage(RUSAGE_CHILDREN, &ru_prev);
	gc();	/* do a garbage collection first to ensure reproducible results */
	t0 = time(NULL);
	pid = efork(TRUE, FALSE);
	if (pid == 0)
		esexit(exitstatus(eval(lp, NULL, evalflags | eval_inchild)));
	status = ewait(pid, FALSE);
	t1 = time(NULL);
	SIGCHK();
	printstatus(0, status);

	getrusage(RUSAGE_CHILDREN, &ru_new);
	timesub(&ru_new.ru_utime, &ru_prev.ru_utime, &ru_diff.ru_utime);
	timesub(&ru_new.ru_stime, &ru_prev.ru_stime, &ru_diff.ru_stime);

	eprint(
		"%6ldr %5ld.%ldu %5ld.%lds\t%L\n",
		t1 - t0,
		ru_diff.ru_utime.tv_sec, (long) (ru_diff.ru_utime.tv_usec / 100000),
		ru_diff.ru_stime.tv_sec, (long) (ru_diff.ru_stime.tv_usec / 100000),
		lp, " "
	);

	RefEnd(lp);
	return mklist(mkstr(mkstatus(status)), NULL);

#else	/* !HAVE_GETRUSAGE */

	int pid, status;
	Ref(List *, lp, list);

	gc();	/* do a garbage collection first to ensure reproducible results */
	pid = efork(TRUE, FALSE);
	if (pid == 0) {
		clock_t t0, t1;
		struct tms tms;
		static clock_t ticks = 0;

		if (ticks == 0)
			ticks = sysconf(_SC_CLK_TCK);

		t0 = times(&tms);
		pid = efork(TRUE, FALSE);
		if (pid == 0)
			esexit(exitstatus(eval(lp, NULL, evalflags | eval_inchild)));

		status = ewaitfor(pid);
		t1 = times(&tms);
		SIGCHK();
		printstatus(0, status);

		tms.tms_cutime += ticks / 20;
		tms.tms_cstime += ticks / 20;

		eprint(
			"%6ldr %5ld.%ldu %5ld.%lds\t%L\n",
			(t1 - t0 + ticks / 2) / ticks,
			tms.tms_cutime / ticks, ((tms.tms_cutime * 10) / ticks) % 10,
			tms.tms_cstime / ticks, ((tms.tms_cstime * 10) / ticks) % 10,
			lp, " "
		);
		esexit(status);
	}
	status = ewaitfor(pid);
	SIGCHK();
	printstatus(0, status);

	RefEnd(lp);
	return mklist(mkstr(mkstatus(status)), NULL);

#endif	/* !HAVE_GETRUSAGE */
}
#endif	/* BUILTIN_TIME */

#if !KERNEL_POUNDBANG
PRIM(execfailure) {
	int fd, len, argc;
	char header[1024], *args[10], *s, *end, *file;

	gcdisable();
	if (list == NULL)
		fail("$&execfailure", "usage: %%exec-failure name argv");

	file = getstr(list->term);
	fd = eopen(file, oOpen);
	if (fd < 0) {
		gcenable();
		return NULL;
	}
	len = read(fd, header, sizeof header);
	close(fd);
	if (len <= 2 || header[0] != '#' || header[1] != '!') {
		gcenable();
		return NULL;
	}

	s = &header[2];
	end = &header[len];
	argc = 0;
	while (argc < arraysize(args) - 1) {
		int c;
		while ((c = *s) == ' ' || c == '\t')
			if (++s >= end) {
				gcenable();
				return NULL;
			}
		if (c == '\n' || c == '\r')
			break;
		args[argc++] = s;
		do
			if (++s >= end) {
				gcenable();
				return NULL;
			}
		while (s < end && (c = *s) != ' ' && c != '\t' && c != '\n' && c != '\r');
		*s++ = '\0';
		if (c == '\n' || c == '\r')
			break;
	}
	if (argc == 0) {
		gcenable();
		return NULL;
	}

	list = list->next;
	if (list != NULL)
		list = list->next;
	list = mklist(mkstr(file), list);
	while (argc != 0)
		list = mklist(mkstr(args[--argc]), list);

	Ref(List *, lp, list);
	gcenable();
	lp = eval(lp, NULL, eval_inchild);
	RefReturn(lp);
}
#endif /* !KERNEL_POUNDBANG */

extern Dict *initprims_sys(Dict *primdict) {
	X(newpgrp);
	X(background);
	X(umask);
	X(cd);
	X(fork);
	X(run);
	X(setsignals);
	X(getlocale);
	X(setlocale);
#if BSD_LIMITS
	X(limit);
#endif
#if BUILTIN_TIME
	X(time);
#endif
#if !KERNEL_POUNDBANG
	X(execfailure);
#endif /* !KERNEL_POUNDBANG */
	return primdict;
}

extern void initlocale(void) {
	struct locale_info *lip;

	Ref(List *, errors, NULL);
	Ref(List *, value, NULL);
	for (lip = &locale_info[0]; lip != &locale_info[LI_END]; lip++) {
		value = varlookup(lip->name, NULL);
		if (value == NULL)
			continue;
		value = mklist(mkstr(str("%s", lip->name)), value);
		ExceptionHandler
			eval(mklist(mkstr("setlocale"), value), NULL, 0);
		CatchException(e)
			Ref(List *, name, mklist(mkstr(str("$%s", lip->name)), NULL));
			errors = append(errors, name);
			RefEnd(name);
			(void)e;
		EndExceptionHandler
	}
	RefEnd(value);
	if (errors != NULL)
		eprint("warning: locale initialization failed for the following:\n\t%L\n",
		       errors, "\n\t");
	RefEnd(errors);
}
