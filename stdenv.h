/* stdenv.h -- set up an environment we can use ($Revision: 1.3 $) */

#include "esconfig.h"

/*
 * type qualifiers
 */

/*
 * protect the rest of es source from the dance of the includes
 */

#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>

typedef struct dirent Dirent;

#if !HAVE__NORETURN
# undef _Noreturn
# if HAVE_NORETURN_ATTRIB
#  define _Noreturn __attribute__((noreturn))
# else
#  define _Noreturn /* nothing */
# endif
#endif

/*
 * things that should be defined by header files but might not have been
 */

/* setjmp */

#if defined sigsetjmp || HAVE_SIGSETJMP
/* under linux, sigsetjmp and setjmp are both macros
 * -- need to undef setjmp to avoid problems
 */
# ifdef setjmp
#  undef setjmp
# endif
# define setjmp(buf)    sigsetjmp(buf,1)
# define longjmp(x,y)   siglongjmp(x,y)
# define jmp_buf        sigjmp_buf
#endif


/*
 * macros
 */

#define streq(s, t)             (strcmp(s, t) == 0)
#define strneq(s, t, n)         (strncmp(s, t, n) == 0)
#define hasprefix(s, p)         strneq(s, p, (sizeof p) - 1)
#define arraysize(a)            ((int) (sizeof (a) / sizeof (*a)))
#define memzero(dest, count)    memset(dest, 0, count)
#define atoi(s)                 strtol(s, NULL, 0)

#if SOLARIS
#define STMT(stmt)      if (1) { stmt; } else
#define NOP             if (1) ; else
#else
#define STMT(stmt)      do { stmt; } while (0)
#define NOP             do {} while (0)
#endif

#define CONCAT(a,b) a ## b
#define STRING(s)   #s


/*
 * types we use throughout es
 */

#undef FALSE
#undef TRUE
typedef enum { FALSE, TRUE } Boolean;

typedef volatile sig_atomic_t Atomic;
typedef gid_t gidset_t;


/*
 * variable argument lists
 */

#if !HAVE_VA_COPY
# if HAVE___VA_COPY
#  define va_copy(dest, src)    __va_copy(dest, src)
# else
#  define va_copy(dest, src)    memcpy(&dest, &src, sizeof(src))
# endif
#endif


/*
 * assertion checking
 */

#if ASSERTIONS
#define assert(expr) \
    STMT( \
        if (!(expr)) { \
            eprint("%s:%d: assertion failed (%s)\n", \
                __FILE__, __LINE__, STRING(expr)); \
            abort(); \
        } \
    )
#else
#define assert(ignore)  NOP
#endif

enum { UNREACHABLE = 0 };


#define NOTREACHED  STMT(assert(UNREACHABLE))

/*
 * hacks to present a standard system call interface
 */

#if HAVE_SETSID
# define setpgrp(a, b)  setsid()
#elif defined(linux) || defined(__GLIBC__)
# define setpgrp(a, b)  setpgid(a, b)
#endif

#if !HAVE_LSTAT
#define lstat   stat
#endif
