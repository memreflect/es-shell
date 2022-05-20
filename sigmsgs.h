/* sigmsgs.h -- interface to signal name and message date */

typedef struct {
	int         sig;
	const char *name;
	const char *msg;
} Sigmsgs;
extern const Sigmsgs signals[];

extern const int     nsignals;
