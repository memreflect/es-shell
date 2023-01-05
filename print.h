/* print.h -- interface to formatted printing routines */

#include <stdarg.h>
#include <stddef.h>

#include "esbool.h"

typedef struct Format Format;
struct Format {
	/* for the formatting routines */
	va_list args;
	long    flags;
	long    f1;
	long    f2;
	int     invoker;
	/* for the buffer maintenance routines */
	char *buf;
	char *bufbegin;
	char *bufend;
	int   flushed;
	void (*grow)(Format *, size_t);
	union {
		int   n;
		void *p;
	} u;
};

/* Format->flags values */
enum {
	FMT_long     = 1,  /* %l */
	FMT_short    = 2,  /* %h */
	FMT_unsigned = 4,  /* %u */
	FMT_zeropad  = 8,  /* %0 */
	FMT_leftside = 16, /* %- */
	FMT_altform  = 32, /* %# */
	FMT_f1set    = 64, /* %<n> */
	FMT_f2set    = 128 /* %.<n> */
};

typedef bool Conv(Format *);

Conv *fmtinstall(int, Conv *);
int   printfmt(Format *, const char *);
int   fmtprint(Format *, const char *, ...);
void  fmtappend(Format *, const char *, size_t);
void  fmtcat(Format *, const char *);

int print(const char *fmt, ...);
int eprint(const char *fmt, ...);
int fprint(int fd, const char *fmt, ...);

char *strv(const char *fmt, va_list args); /* varargs interface to str() */

#define FPRINT_BUFSIZ 1024

/*
 * the following macro should by rights be coded as an expression, not
 * a statement, but certain compilers (notably DEC) have trouble with
 * void expressions inside the ?: operator. (sheesh, give me a break!)
 */
#define fmtputc(f, c)                                               \
	STMT(if ((f)->buf >= (f)->bufend) (*(f)->grow)((f), (size_t)1); \
	     *(f)->buf++ = (c))
