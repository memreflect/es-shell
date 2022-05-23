/* proc.c -- process control system calls */

#include <sys/wait.h>

#include <errno.h>

#include <unistd.h>

#include "es.h"

/* TODO: the rusage code for the time builtin really needs to be cleaned up */

#if BUILTIN_TIME
#	include <sys/resource.h>
#	include <sys/time.h>
#endif

bool hasforked = false;

typedef struct Proc Proc;
struct Proc {
	pid_t pid;
	int   status;
	bool  alive;
	bool  background;
	Proc *next;
	Proc *prev;
#if BUILTIN_TIME
	struct rusage rusage;
#endif
};

static Proc *proclist = NULL;

/* mkproc -- create a Proc structure */
Proc *
mkproc(pid_t pid, bool background) {
	Proc *proc;
	for (proc = proclist; proc != NULL; proc = proc->next)
		if (proc->pid == pid) {   /* are we recycling pids? */
			assert(!proc->alive); /* if lfalse, violates unix semantics */
			break;
		}
	if (proc == NULL) {
		proc       = ealloc(sizeof(Proc));
		proc->next = proclist;
	}
	proc->pid        = pid;
	proc->alive      = true;
	proc->background = background;
	proc->prev       = NULL;
	return proc;
}

/* efork -- fork (if necessary) and clean up as appropriate */
int
efork(bool parent, bool background) {
	if (parent) {
		pid_t pid = fork();
		switch (pid) {
		default: { /* parent */
			Proc *proc = mkproc(pid, background);
			if (proclist != NULL)
				proclist->prev = proc;
			proclist = proc;
			return pid;
		}
		case 0: /* child */
			proclist  = NULL;
			hasforked = true;
			break;
		case -1:
			fail("es:efork", "fork: %s", esstrerror(errno));
		}
	}
	closefds();
	setsigdefaults();
	newchildcatcher();
	return 0;
}

#if BUILTIN_TIME
static struct rusage wait_rusage;
#endif

/* dowait -- a wait wrapper that interfaces with signals */
static int
dowait(int *statusp) {
	int n;
	interrupted = false;
	if (!sigsetjmp(slowlabel, 1)) {
		slow = true;
		n    = interrupted ? -2 :
#if BUILTIN_TIME
		                wait3((void *)statusp, 0, &wait_rusage);
#else
		                wait((void *)statusp);
#endif
	} else
		n = -2;
	slow = false;
	if (n == -2) {
		errno = EINTR;
		n     = -1;
	}
	return n;
}

/* reap -- mark a process as dead and attach its exit status */
static void
reap(pid_t pid, int status) {
	Proc *proc;
	for (proc = proclist; proc != NULL; proc = proc->next)
		if (proc->pid == pid) {
			assert(proc->alive);
			proc->alive  = false;
			proc->status = status;
#if BUILTIN_TIME
			proc->rusage = wait_rusage;
#endif
			return;
		}
}

/* ewait -- wait for a specific process to die, or any process if pid == 0 */
int
ewait(pid_t pid, bool interruptible, struct rusage *rusage) {
	Proc *proc;
top:
	for (proc = proclist; proc != NULL; proc = proc->next)
		if (proc->pid == pid || (pid == 0 && !proc->alive)) {
			int status;
			if (proc->alive) {
				pid_t deadpid;
				int   seen_eintr = false;
				while ((deadpid = dowait(&proc->status)) != pid)
					if (deadpid != -1)
						reap(deadpid, proc->status);
					else if (errno == EINTR) {
						if (interruptible)
							SIGCHK();
						seen_eintr = true;
					} else if (errno == ECHILD && seen_eintr)
						/* TODO: not clear on why this is necessary
						 * (child procs _sometimes_ disappear after SIGINT) */
						break;
					else
						fail("es:ewait", "wait: %s", esstrerror(errno));
				proc->alive = false;
#if BUILTIN_TIME
				proc->rusage = wait_rusage;
#endif
			}
			if (proc->next != NULL)
				proc->next->prev = proc->prev;
			if (proc->prev != NULL)
				proc->prev->next = proc->next;
			else
				proclist = proc->next;
			status = proc->status;
			if (proc->background)
				printstatus(proc->pid, status);
			efree(proc);
#if BUILTIN_TIME
			if (rusage != NULL)
				*rusage = proc->rusage;
#else
			assert(rusage == NULL);
#endif
			return status;
		}
	if (pid == 0) {
		int status;
		while ((pid = dowait(&status)) == -1) {
			if (errno != EINTR)
				fail("es:ewait", "wait: %s", esstrerror(errno));
			if (interruptible)
				SIGCHK();
		}
		reap(pid, status);
		goto top;
	}
	fail("es:ewait", "wait: %ld is not a child of this shell", (long)pid);
	NOTREACHED;
}

#include "prim.h"

PRIM(apids) {
	Proc *p;
	Ref(List *, lp, NULL);
	for (p = proclist; p != NULL; p = p->next)
		if (p->background && p->alive) {
			Term *t = mkstr(str("%d", p->pid));
			lp      = mklist(t, lp);
		}
	/* TODO: sort the return value, but by number? */
	RefReturn(lp);
}

PRIM(wait) {
	pid_t pid;
	if (list == NULL)
		pid = 0;
	else if (list->next == NULL) {
		pid = strtol(getstr(list->term), NULL, 0);
		if (pid <= 0) {
			fail("$&wait", "wait: %ld: bad pid", (long)pid);
			NOTREACHED;
		}
	} else {
		fail("$&wait", "usage: wait [pid]");
		NOTREACHED;
	}
	return mklist(mkstr(mkstatus(ewait(pid, true, NULL))), NULL);
}

Dict *
initprims_proc(Dict *primdict) {
	X(apids);
	X(wait);
	return primdict;
}
