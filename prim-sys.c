/* prim-sys.c -- system call primitives */

#include "esconfig.h"

#include <sys/stat.h>

#if HAVE_SETRLIMIT || BUILTIN_TIME
#	include <sys/resource.h>
#	if BUILTIN_TIME
#		include <sys/time.h>
#	endif
#endif

#include <ctype.h>
#include <errno.h>

#include <unistd.h>

#include "es.h"
#include "prim.h"

PRIM(newpgrp) {
	int pid;
	if (list != NULL)
		fail("$&newpgrp", "usage: newpgrp");
	pid = getpid();
	setpgid(pid, pid);
	{
		Sigeffect sigttou = esignal(SIGTTOU, sig_ignore);
		tcsetpgrp(2, pid);
		esignal(SIGTTOU, sigttou);
	}
	return ltrue;
}

PRIM(background) {
	int pid = efork(true, true);
	if (pid == 0) {
#if JOB_PROTECT
		/* job control safe version: put it in a new pgroup. */
		setpgid(0, getpid());
#endif
		mvfd(eopen("/dev/null", oOpen), 0);
		exit(exitstatus(eval(list, NULL, evalflags | eval_inchild)));
	}
	return mklist(mkstr(str("%d", pid)), NULL);
}

PRIM(fork) {
	int pid;
	int status;
	pid = efork(true, false);
	if (pid == 0)
		exit(exitstatus(eval(list, NULL, evalflags | eval_inchild)));
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
	lp   = forkexec(file, lp->next, (evalflags & eval_inchild) != 0);
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
		int   mask;
		char *s;
		char *t;
		s    = getstr(list->term);
		mask = strtol(s, &t, 8);
		if ((t != NULL && *t != '\0') || ((unsigned)mask) > 07777)
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
	int       i;
	Sigeffect effects[NSIG];
	for (i = 0; i < NSIG; i++)
		effects[i] = sig_default;
	Ref(List *, lp, list);
	for (; lp != NULL; lp = lp->next) {
		int         sig;
		const char *s      = getstr(lp->term);
		Sigeffect   effect = sig_catch;
		switch (*s) {
		case '-':
			effect = sig_ignore;
			s++;
			break;
		case '/':
			effect = sig_noop;
			s++;
			break;
		case '.':
			effect = sig_special;
			s++;
			break;
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

/*
 * limit builtin -- this is too much code for what it gives you
 */

#if HAVE_SETRLIMIT
typedef struct Suffix Suffix;
struct Suffix {
	const char *name;
	long        amount;
};

static const Suffix sizesuf[] = {
  /* TODO add support for t and p on 64-bit systems */
		{"g", 1024 * 1024 * 1024},
		{"m", 1024 * 1024       },
		{"k", 1024              },
};
static const Suffix *sizesuf_end = sizesuf + arraysize(sizesuf);

static const Suffix  timesuf[] = {
		 {"h", 60 * 60},
		 {"m", 60     },
		 {"s", 1      },
};
static const Suffix *timesuf_end = timesuf + arraysize(timesuf);

typedef struct {
	char		 *name;
	int           flag;
	const Suffix *suffix;
} Limit;

static const Limit limits[] = {

  /* resource limits from single unix specification */
		{"coredumpsize",     RLIMIT_CORE,      sizesuf},
		{"cputime",          RLIMIT_CPU,       timesuf},
		{"datasize",         RLIMIT_DATA,      sizesuf},
		{"filesize",         RLIMIT_FSIZE,     sizesuf},
		{"descriptors",      RLIMIT_NOFILE,    NULL   },
		{"stacksize",        RLIMIT_STACK,     sizesuf},
#	ifdef RLIMIT_AS
		{"available",        RLIMIT_AS,        sizesuf},
#	endif

  /* platform-specific resource limits */
#	ifdef RLIMIT_KQUEUES
		{"kqueues",          RLIMIT_KQUEUES,   NULL   },
#	endif
#	ifdef RLIMIT_LOCKS
		{"locks",            RLIMIT_LOCKS,     NULL   },
#	endif
#	ifdef RLIMIT_MEMLOCK
		{"lockedmemory",     RLIMIT_MEMLOCK,   sizesuf},
#	endif
#	ifdef RLIMIT_MSGQUEUE
		{"mqmax",            RLIMIT_MSGQUEUE,  NULL   },
#	endif
#	ifdef RLIMIT_NICE
		{"nice",             RLIMIT_NICE NULL},
#	endif
#	ifdef RLIMIT_NPROC
		{"processes",      RLIMIT_NPROC,             NULL       },
#	endif
#	ifdef RLIMIT_NPTS
		{"ptys",       RLIMIT_NPTS,             NULL},
#	endif
#	ifdef RLIMIT_RSS
		{"memoryuse",        RLIMIT_RSS,          sizesuf       },
#	endif
#	ifdef RLIMIT_RTPRIO
		{"rtprio",     RLIMIT_RTPRIO,             NULL},
#	endif
#	ifdef RLIMIT_RTTIME
		{"rttime",     RLIMIT_RTTIME,          timesuf       },
#	endif
#	ifdef RLIMIT_SBSIZE
		{"sockbufsize",     RLIMIT_SBSIZE,          sizesuf},
#	endif
#	ifdef RLIMIT_SIGPENDING
		{"pendsigs", RLIMIT_SIGPENDING,             NULL       },
#	endif
#	ifdef RLIMIT_SWAP
		{"swap",       RLIMIT_SWAP,          sizesuf},
#	endif
  /* some platforms equate this with RLIMIT_AS */
#	ifdef RLIMIT_VMEM
		{"memorysize",       RLIMIT_VMEM,          sizesuf       },
#	endif
};
static const Limit *limits_end = limits + arraysize(limits);

static void
printlimit(const Limit *limit, bool hard) {
	struct rlimit rlim;
	rlim_t        lim;
	getrlimit(limit->flag, &rlim);
	if (hard)
		lim = rlim.rlim_max;
	else
		lim = rlim.rlim_cur;
	if (lim == RLIM_INFINITY)
		print("%-8s\tunlimited\n", limit->name);
	else {
		const Suffix *suf;
		const Suffix *end = limit->suffix == sizesuf
		                          ? sizesuf_end
		                  : limit->suffix == timesuf
		                          ? timesuf_end
		                          : NULL;

		for (suf = limit->suffix; suf != end; ++suf)
			if (lim % suf->amount == 0 && (lim != 0 || suf->amount > 1)) {
				lim /= suf->amount;
				break;
			}
		print("%-8s\t%d%s\n",
		      limit->name,
		      (int)lim,
		      (suf == NULL || lim == 0) ? "" : suf->name);
	}
}

static long
parselimit(const Limit *limit, char *s) {
	long          lim;
	char		 *t;
	const Suffix *suf = limit->suffix;
	const Suffix *end = limit->suffix == sizesuf
	                          ? sizesuf_end
	                  : limit->suffix == timesuf
	                          ? timesuf_end
	                          : NULL;
	if (streq(s, "unlimited"))
		return RLIM_INFINITY;
	if (!isdigit(*s))
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
			for (;; ++suf) {
				if (suf == end)
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
	const Limit *lim  = limits;
	bool         hard = false;
	Ref(List *, lp, list);

	if (lp != NULL && streq(getstr(lp->term), "-h")) {
		hard = true;
		lp   = lp->next;
	}

	if (lp == NULL)
		for (; lim != limits_end; ++lim)
			printlimit(lim, hard);
	else {
		char *name = getstr(lp->term);
		for (;; ++lim) {
			if (lim == limits_end)
				fail("$&limit", "%s: no such limit", name);
			if (streq(name, lim->name))
				break;
		}
		lp = lp->next;
		if (lp == NULL)
			printlimit(lim, hard);
		else {
			long          n;
			struct rlimit rlim;
			getrlimit(lim->flag, &rlim);
			if ((n = parselimit(lim, getstr(lp->term))) < 0)
				fail("$&limit", "%s: bad limit value", getstr(lp->term));
			if (hard)
				rlim.rlim_max = n;
			else
				rlim.rlim_cur = n;
			if (setrlimit(lim->flag, &rlim) == -1)
				fail("$&limit", "setrlimit: %s", esstrerror(errno));
		}
	}
	RefEnd(lp);
	return ltrue;
}
#endif /* HAVE_SETRLIMIT */

#if BUILTIN_TIME
PRIM(time) {
	int           pid;
	int           status;
	time_t        t0;
	time_t        t1;
	struct rusage r;

	Ref(List *, lp, list);

	gc(); /* do a garbage collection first to ensure reproducible results */
	t0  = time(NULL);
	pid = efork(true, false);
	if (pid == 0)
		exit(exitstatus(eval(lp, NULL, evalflags | eval_inchild)));
	status = ewait(pid, false, &r);
	t1     = time(NULL);
	SIGCHK();
	printstatus(0, status);

	eprint(
			"%6ldr %5ld.%ldu %5ld.%lds\t%L\n",
			t1 - t0,
			r.ru_utime.tv_sec,
			(long)(r.ru_utime.tv_usec / 100000),
			r.ru_stime.tv_sec,
			(long)(r.ru_stime.tv_usec / 100000),
			lp,
			" ");

	RefEnd(lp);
	return mklist(mkstr(mkstatus(status)), NULL);
}
#endif /* BUILTIN_TIME */

#if !KERNEL_POUNDBANG
PRIM(execfailure) {
	int    fd;
	int    len;
	size_t argc;
	char   header[1024];
	char  *args[10];
	char  *s;
	char  *end;
	char  *file;

	gcdisable();
	if (list == NULL)
		fail("$&execfailure", "usage: %%exec-failure name argv");

	file = getstr(list->term);
	fd   = eopen(file, oOpen);
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

	s    = &header[2];
	end  = &header[len];
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

extern Dict *
initprims_sys(Dict *primdict) {
	X(newpgrp);
	X(background);
	X(umask);
	X(cd);
	X(fork);
	X(run);
	X(setsignals);
#if HAVE_SETRLIMIT
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
