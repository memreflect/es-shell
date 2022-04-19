/* stdenv.h -- set up an environment we can use */

#include "esconfig.h"

#include <sys/types.h>

#if BUILTIN_TIME
# include <sys/resource.h>
#endif

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

#if HAVE_READLINE
# include <stdio.h>
# if HAVE_READLINE_READLINE_H
#  include <readline/readline.h>
# elif HAVE_READLINE_H
#  include <readline.h>
# else
#  invalid configuration -- readline.h
# endif
# define READLINE 1
# if HAVE_READLINE_HISTORY
#  if HAVE_READLINE_HISTORY_H
#   include <readline/history.h>
#  elif HAVE_HISTORY_H
#   include <history.h>
#  else
#   invalid configuration -- history.h
#  endif
# endif /* HAVE_READLINE_HISTORY */
#endif /* HAVE_READLINE */


/*
 * things that rely on specific versions of ISO C
 */

#if !ENABLE_C11
# undef _Noreturn
# if HAVE_NORETURN_ATTRIBUTE
#  define _Noreturn __attribute__((noreturn))
# else
#  define _Noreturn /* nothing */
# endif
#endif

#if !ENABLE_C99
# if HAVE___VA_COPY
#  define va_copy(dest, src)    __va_copy(dest, src)
# else
#  define va_copy(dest, src)    memcpy(&dest, &src, sizeof(src))
# endif
#endif


/*
 * macros
 */

#define atoi(s)                 strtol(s, NULL, 0)
#define arraysize(a)            (sizeof (a) / sizeof (*a))
#define hasprefix(s, p)         strneq(s, p, (sizeof p) - 1)
#if HAVE_EXPLICIT_BZERO
# include <strings.h>
# define memzero(dest, count)   explicit_bzero(dest, count)
#else
# define memzero(dest, count)   memset(dest, 0, count)
#endif
#define streq(s, t)             (strcmp(s, t) == 0)
#define strneq(s, t, n)         (strncmp(s, t, n) == 0)

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

typedef volatile sig_atomic_t Atomic;


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
