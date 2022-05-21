/* gc.h -- garbage collector interface for es */

#include <stddef.h>

/* see also es.h for more generally applicable definitions */

/*
 * tags
 */

struct Tag {
	void *(*copy)(void *);
	size_t (*scan)(void *);
#if ASSERTIONS || GCVERBOSE
	long magic;
	char *typename;
#endif
};

extern struct Tag StringTag;

#if ASSERTIONS || GCVERBOSE
enum { TAGMAGIC = 0xDefaced };
#	define DefineTag(t, storage)              \
		static void  *CONCAT(t, Copy)(void *); \
		static size_t CONCAT(t, Scan)(void *); \
		storage Tag   CONCAT(t, Tag) = {       \
				  CONCAT(t, Copy),             \
				  CONCAT(t, Scan),             \
				  TAGMAGIC,                    \
				  STRING(t),                   \
        }
#else
#	define DefineTag(t, storage)              \
		static void  *CONCAT(t, Copy)(void *); \
		static size_t CONCAT(t, Scan)(void *); \
		storage Tag   CONCAT(t, Tag) = {       \
				  CONCAT(t, Copy),             \
				  CONCAT(t, Scan),             \
        }
#endif

/*
 * allocation
 */

void *gcalloc(size_t, struct Tag *);

typedef struct Buffer Buffer;
struct Buffer {
	size_t len;
	size_t current;
	char   str[1];
};

Buffer *openbuffer(size_t minsize);
Buffer *expandbuffer(Buffer *buf, size_t minsize);
Buffer *bufncat(Buffer *buf, const char *s, size_t len);
Buffer *bufcat(Buffer *buf, const char *s);
Buffer *bufputc(Buffer *buf, char c);
char   *sealbuffer(Buffer *buf);
char   *sealcountedbuffer(Buffer *buf);
void    freebuffer(Buffer *buf);

void *forward(void *p);
