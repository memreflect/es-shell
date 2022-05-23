/* status.c -- status manipulations */

#include <sys/wait.h>

#include "es.h"
#include "term.h"

#ifndef WCOREDUMP
# define WCOREDUMP(status) ((status) & 0x80)
#endif

static const Term
	ltrueterm	= { "0", NULL },
	lfalseterm	= { "1", NULL };
static const List
	ltruelist	= { (Term *) &ltrueterm, NULL },
	lfalselist	= { (Term *) &lfalseterm, NULL };
List
	*ltrue		= (List *) &ltruelist,
	*lfalse		= (List *) &lfalselist;

/* istrue -- is this status list ltrue? */
extern bool istrue(List *status) {
	for (; status != NULL; status = status->next) {
		Term *term = status->term;
		if (term->closure != NULL)
			return false;
		else {
			const char *str = term->str;
			assert(str != NULL);
			if (*str != '\0' && (*str != '0' || str[1] != '\0'))
				return false;
		}
	}
	return true;
}

/* exitstatus -- turn a status list into an exit(2) value */
extern int exitstatus(List *status) {
	Term *term;
	char *s;
	unsigned long n;

	if (status == NULL)
		return 0;
	if (status->next != NULL)
		return istrue(status) ? 0 : 1;
	term = status->term;
	if (term->closure != NULL)
		return 1;

	s = term->str;
	if (*s == '\0')
		return 0;
	n = strtol(s, &s, 0);
	if (*s != '\0' || n > 255)
		return 1;
	return n;
}

/* mkstatus -- turn a unix exit(2) status into a string */
extern char *mkstatus(int status) {
	if (WIFSIGNALED(status)) {
		char *name = signame(WTERMSIG(status));
		if (WCOREDUMP(status))
			name = str("%s+core", name);
		return name;
	}
	return str("%d", WEXITSTATUS(status));
}

/* printstatus -- print the status if we should */
extern void printstatus(int pid, int status) {
	if (WIFSIGNALED(status)) {
		const char *msg = sigmessage(WTERMSIG(status)), *tail = "";
		if (WCOREDUMP(status)) {
			tail = "--core dumped";
			if (*msg == '\0')
				tail += (sizeof "--") - 1;
		}
    if (*msg != '\0' || *tail != '\0') {
      if (pid == 0)
        eprint("%s%s\n", msg, tail);
      else
        eprint("%d: %s%s\n", pid, msg, tail);
    }
	}
}
