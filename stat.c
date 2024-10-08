/* stat.c -- file info retrieval ($Revision: 1.0 $) */

#define	REQUIRE_STAT	1
#define	REQUIRE_PARAM	1

#include "es.h"
#include "prim.h"

#define	READ	4
#define	WRITE	2
#define	EXEC	1

#define	USER	6
#define	GROUP	3
#define	OTHER	0

#ifndef S_ISBLK
#define S_ISBLK(x) (((x) & S_IFBLK) == S_IFBLK)
#endif
#ifndef S_ISCHR
#define S_ISCHR(x) (((x) & S_IFCHR) == S_IFCHR)
#endif
#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFDIR) == S_IFDIR)
#endif
#ifndef S_ISFIFO
#define S_ISFIFO(x) (((x) & S_IFFIFO) == S_IFFIFO)
#endif
#ifndef S_ISLNK
#define S_ISLNK(x) (((x) & S_IFLNK) == S_IFLNK)
#endif
#ifndef S_ISREG
#define S_ISREG(x) (((x) & S_IFREG) == S_IFREG)
#endif
#ifndef S_ISSOCK
#define S_ISSOCK(x) (((x) & S_IFSOCK) == S_IFSOCK)
#endif

/* ingroupset -- determine whether gid lies in the user's set of groups */
static Boolean ingroupset(gidset_t gid) {
#ifdef NGROUPS
	int i;
	static int ngroups;
	static gidset_t gidset[NGROUPS];
	static Boolean initialized = FALSE;
	if (!initialized) {
		initialized = TRUE;
		ngroups = getgroups(NGROUPS, gidset);
	}
	for (i = 0; i < ngroups; i++)
		if (gid == gidset[i])
			return TRUE;
#endif
	return FALSE;
}

static int getfperm(struct stat *buf) {
	static gidset_t uid, gid;
	static Boolean initialized = FALSE;
	int perm = 0;
	int r = S_IROTH, w = S_IWOTH, x = S_IXOTH;
	if (!initialized) {
		initialized = TRUE;
		uid = geteuid();
		gid = getegid();
	}
	if (uid == 0)
		return (READ |
			WRITE |
			((buf->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
			 ? EXEC
			 : 0));
	if (uid == buf->st_uid) {
		r = S_IRUSR;
		w = S_IWUSR;
		x = S_IXUSR;
	} else if (gid == buf->st_gid || ingroupset(buf->st_gid)) {
		r = S_IRGRP;
		w = S_IWGRP;
		x = S_IXGRP;
	}
	if (buf->st_mode & r)
		perm |= READ;
	if (buf->st_mode & w)
		perm |= WRITE;
	if (buf->st_mode & x)
		perm |= EXEC;
	return perm;
}

static int getftype(struct stat *buf) {
	if (S_ISREG(buf->st_mode))
		return 'f';
	if (S_ISDIR(buf->st_mode))
		return 'd';
	if (S_ISCHR(buf->st_mode))
		return 'c';
	if (S_ISBLK(buf->st_mode))
		return 'b';
	if (S_ISSOCK(buf->st_mode))
		return 's';
	if (S_ISFIFO(buf->st_mode))
		return 'p';
	if (S_ISLNK(buf->st_mode))
		return 'l';
	return 0;
}

static Boolean dostat(struct stat *buf, char *path, Boolean use_lstat) {
	if (use_lstat)
		return lstat(path, buf) != -1;
	return stat(path, buf) != -1;
}

PRIM(stat) {
	int c, perm = 0;
	Boolean use_lstat = TRUE;
	const char * const usage = "$&stat [-P] path";
	char *name;
	char sbuf[4];
	char *sbufp = sbuf;
	struct stat buf;

	gcdisable();
	esoptbegin(list, "$&stat", usage, TRUE);
	while ((c = esopt("P")) != EOF)
		switch (c) {
		case 'P':
			use_lstat = FALSE;
			break;
		default:
			esoptend();
			fail("$&stat", "$&stat -%c is not supported on this system", c);
		}
	list = esoptend();

	if (list->next != NULL)
		fail("$&stat", usage);

	name = getstr(list->term);
	if (!dostat(&buf, name, use_lstat))
		fail("$&stat", "%s", esstrerror(errno));
	else {
		c = getftype(&buf);
		if (c == 0)
			fail("$&stat", "unknown file type");
		*sbufp++ = c;
		perm = getfperm(&buf);
		if (perm & READ)
			*sbufp++ = 'r';
		if (perm & WRITE)
			*sbufp++ = 'w';
		if (perm & EXEC)
			*sbufp++ = 'x';
		Ref(List *, result, mklist(mkstr(gcndup(sbuf, sbufp-sbuf)), NULL));
		gcenable();
		RefReturn(result);
	}
}

extern Dict *initprims_access(Dict *primdict) {
	X(stat);
	return primdict;
}

extern Boolean isexecutable(char *file) {
	struct stat buf;
	errno = 0;
	return dostat(&buf, file, FALSE) && getftype(&buf) == 'f' && (getfperm(&buf) & EXEC);
}
