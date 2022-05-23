/* fd.c -- file descriptor manipulations */

#include <errno.h>

#include <unistd.h>

#include "es.h"

/* mvfd -- duplicate a fd and close the old */
void
mvfd(int old, int new) {
	if (old != new) {
		int fd = dup2(old, new);
		if (fd == -1)
			fail("es:mvfd", "dup2: %s", esstrerror(errno));
		assert(fd == new);
		close(old);
	}
}

/*
 * deferred file descriptor operations
 *	we maintain a stack of file descriptor operations as they occur,
 *	if we're operating in the context of a parent shell.  (if we're
 *	already in a forked process, we just do them.)  the operations
 *	are actually done at closefds() time.
 */

typedef struct {
	int realfd;
	int userfd;
} Defer;

static Defer *deftab;
static int    defcount = 0;
static int    defmax   = 0;

static void
dodeferred(int realfd, int userfd) {
	assert(userfd >= 0);
	releasefd(userfd);

	if (realfd == -1)
		close(userfd);
	else {
		assert(realfd >= 0);
		mvfd(realfd, userfd);
	}
}

static int
pushdefer(bool parent, int realfd, int userfd) {
	if (parent) {
		Defer *defer;
		if (defcount >= defmax) {
			int i;
			for (i = 0; i < defcount; i++)
				unregisterfd(&deftab[i].realfd);
			defmax += 10;
			deftab = erealloc(deftab, defmax * sizeof(Defer));
			for (i = 0; i < defcount; i++)
				registerfd(&deftab[i].realfd, true);
		}
		defer         = &deftab[defcount++];
		defer->realfd = realfd;
		defer->userfd = userfd;
		registerfd(&defer->realfd, true);
		return defcount - 1;
	} else {
		dodeferred(realfd, userfd);
		return UNREGISTERED;
	}
}

int
defer_mvfd(bool parent, int old, int new) {
	assert(old >= 0);
	assert(new >= 0);
	return pushdefer(parent, old, new);
}

int
defer_close(bool parent, int fd) {
	assert(fd >= 0);
	return pushdefer(parent, -1, fd);
}

void
undefer(int ticket) {
	if (ticket != UNREGISTERED) {
		Defer *defer;
		assert(ticket >= 0);
		assert(defcount > 0);
		defer = &deftab[--defcount];
		assert(ticket == defcount);
		unregisterfd(&defer->realfd);
		if (defer->realfd != -1)
			close(defer->realfd);
	}
}

/* fdmap -- turn a deferred (user) fd into a real fd */
int
fdmap(int fd) {
	int i = defcount;
	while (--i >= 0) {
		Defer *defer = &deftab[i];
		if (fd == defer->userfd) {
			fd = defer->realfd;
			if (fd == -1)
				return -1;
		}
	}
	return fd;
}

/* remapfds -- apply the fd map to the current file descriptor table */
static void
remapfds(void) {
	Defer *defer;
	Defer *defend = &deftab[defcount];
	for (defer = deftab; defer < defend; defer++) {
		unregisterfd(&defer->realfd);
		dodeferred(defer->realfd, defer->userfd);
	}
	defcount = 0;
}

/*
 * the registered file descriptor list
 *	this is actually a list of pointers to file descriptors.
 *	when we start to work with a user-defined fd, we scan
 *	this list (releasefds) and it an entry matches the user
 *	defined one, we dup it to a different number.  when we fork,
 *	we close all the descriptors on this list.
 */

typedef struct {
	int *fdp;
	bool closeonfork;
} Reserve;

static Reserve *reserved = NULL;
static int      rescount = 0;
static int      resmax   = 0;

/* registerfd -- reserve a file descriptor for es */
void
registerfd(int *fdp, bool closeonfork) {
#if ASSERTIONS
	int i;
	for (i = 0; i < rescount; i++)
		assert(fdp != reserved[i].fdp);
#endif
	if (rescount >= resmax) {
		resmax += 10;
		reserved = erealloc(reserved, resmax * sizeof(Reserve));
	}
	reserved[rescount].fdp         = fdp;
	reserved[rescount].closeonfork = closeonfork;
	rescount++;
}

/* unregisterfd -- give up our hold on a file descriptor */
void
unregisterfd(int *fdp) {
	int i;
	assert(reserved != NULL);
	assert(rescount > 0);
	for (i = 0; i < rescount; i++)
		if (reserved[i].fdp == fdp) {
			reserved[i] = reserved[--rescount];
			return;
		}
	panic("%x not on file descriptor reserved list", fdp);
}

/* closefds -- close file descriptors after a fork() */
void
closefds(void) {
	int i;
	remapfds();
	for (i = 0; i < rescount; i++) {
		Reserve *r = &reserved[i];
		if (r->closeonfork) {
			int fd = *r->fdp;
			if (fd >= 3)
				close(fd);
			*r->fdp = -1;
		}
	}
}

/* releasefd -- release a specific file descriptor from its es uses */
void
releasefd(int n) {
	int i;
	assert(n >= 0);
	for (i = 0; i < rescount; i++) {
		int *fdp = reserved[i].fdp;
		int  fd  = *fdp;
		if (fd == n) {
			*fdp = dup(fd);
			if (*fdp == -1) {
				assert(errno != EBADF);
				fail("es:releasefd", "%s", esstrerror(errno));
			}
			close(fd);
		}
	}
}

/* isdeferred -- is this file descriptor on the deferral list */
static bool
isdeferred(int fd) {
	Defer *defer;
	Defer *defend = &deftab[defcount];
	for (defer = deftab; defer < defend; defer++)
		if (defer->userfd == fd)
			return true;
	return false;
}

/* newfd -- return a new, free file descriptor */
int
newfd(void) {
	int i;
	for (i = 3;; i++)
		if (!isdeferred(i)) {
			int fd = dup(i);
			if (fd == -1) {
				if (errno != EBADF)
					fail("$&newfd", "newfd: %s", esstrerror(errno));
				return i;
			} else if (isdeferred(fd)) {
				int n = newfd();
				close(fd);
				return n;
			} else {
				close(fd);
				return fd;
			}
		}
}
