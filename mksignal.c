/* mksignal.c -- generate signal mappings */
#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sigmsgs.h"

/*
 * POSIX signals
 *
 * These should theoretically be available on all systems.
 */
Sigmsgs posix[] = {
#ifdef SIGABRT
    {SIGABRT, "sigabrt", "abort"},
#endif
#ifdef SIGALRM
    {SIGALRM, "sigalrm", "alarm clock"},
#endif
#ifdef SIGBUS
    {SIGBUS, "sigbus", "bus error"},
#endif
#ifdef SIGCHLD
    {SIGCHLD, "sigchld", "child status changed"},
#endif
#ifdef SIGCONT
    {SIGCONT, "sigcont", "continued"},
#endif
#ifdef SIGFPE
    {SIGFPE, "sigfpe", "floating-point exception"},
#endif
#ifdef SIGHUP
    {SIGHUP, "sighup", "hangup"},
#endif
#ifdef SIGILL
    {SIGILL, "sigill", "illegal instruction"},
#endif
#ifdef SIGINT
    {SIGINT, "sigint", ""},
#endif
#ifdef SIGKILL
    {SIGKILL, "sigkill", "killed"},
#endif
#ifdef SIGPIPE
    {SIGPIPE, "sigpipe", ""},
#endif
#ifdef SIGQUIT
    {SIGQUIT, "sigquit", "quit"},
#endif
#ifdef SIGSEGV
    {SIGSEGV, "sigsegv", "segmentation fault"},
#endif
#ifdef SIGSTOP
    {SIGSTOP, "sigstop", "stopped (signal)"},
#endif
#ifdef SIGTERM
    {SIGTERM, "sigterm", "terminated"},
#endif
#ifdef SIGTSTP
    {SIGTSTP, "sigtstp", "stopped"},
#endif
#ifdef SIGTTIN
    {SIGTTIN, "sigttin", "background tty read"},
#endif
#ifdef SIGTTOU
    {SIGTTOU, "sigttou", "background tty write"},
#endif
#ifdef SIGUSR1
    {SIGUSR1, "sigusr1", "user signal 1"},
#endif
#ifdef SIGUSR2
    {SIGUSR2, "sigusr2", "user signal 2"},
#endif
};
const size_t nsig_posix = sizeof(posix) / sizeof(posix[0]);

/*
 * These are X/Open system interfaces, and some may be obsolescent, notably
 * SIGPOLL used with X/Open STREAMS interfaces and SIGPROF.
 */
Sigmsgs std[] = {
#ifdef SIGPOLL
    {SIGPOLL, "sigpoll", "pollable event occurred"},
#endif
#ifdef SIGPROF
    {SIGPROF, "sigprof", "profiling timer expired"},
#endif
#ifdef SIGSYS
    {SIGSYS, "sigsys", "bad argument to system call"},
#endif
#ifdef SIGTRAP
    {SIGTRAP, "sigtrap", "trace/breakpoint trap"},
#endif
#ifdef SIGURG
    {SIGURG, "sigurg", "urgent condition on i/o channel"},
#endif
#ifdef SIGVTALRM
    {SIGVTALRM, "sigvtalrm", "virtual timer expired"},
#endif
#ifdef SIGXCPU
    {SIGXCPU, "sigxcpu", "exceeded CPU time limit"},
#endif
#ifdef SIGXFSZ
    {SIGXFSZ, "sigxfsz", "exceeded file size limit"},
#endif
};
const size_t nsig_std = sizeof(std) / sizeof(std[0]);

/*
 * Nonstandard signals that may or may not be defined
 *
 * In the event that a signal is an old name for a standard signal, the
 * standard signal is preferred.  For example, SIGCLD is only used if
 * SIGCHLD is not defined, or they are not the same.
 *
 * In the event that two nonstandard signals below may have the same value
 * on some systems, such as SIGLWP and SIGTHR, an arbitrary choice is made
 * which one to prefer.
 */
Sigmsgs other[] = {
#ifdef SIGCLD
    /*
     * SIGCLD is equivalent to SIGCHLD on all systems I'm aware of, but on
     * the off chance that there's a system where that's not true...
     */
    {SIGCLD, "sigcld", "child status changed"},
#endif
#ifdef SIGEMT
    /*
     * Many systems implement this emulation trap, even if it's not
     * implemented for all architectures (e.g. signal(7) on Debian GNU/Linux
     * indicates it's only defined on MIPS builds).
     */
    {SIGEMT, "sigemt", "emulation trap"},
#endif
#ifdef SIGIO
    /*
     * Some systems do not have SIGPOLL, instead defining SIGIO,
     * and some systems have both with distinct values.
     */
    {SIGIO, "sigio", "input/output possible"},
#endif
#ifdef SIGPWR
    /* Some systems have this, but it is never actually used. */
    {SIGPWR, "sigpwr", "power failure/restart"},
#endif
#ifdef SIGINFO
    /* SIGINFO==SIGPWR on some systems (e.g. Debian GNU/Linux on x86) */
    {SIGINFO, "siginfo", "information request"},
#endif
#ifdef SIGTHR
    /*
     * SIGTHR is pretty much the BSD equivalent to Sun's SIGLWP, though it
     * is not available on all modern BSDs.  We prefer Sun's SIGLWP here
     * since it is available on more implementations.
     */
    {SIGTHR, "sigthr", "thread library signal"},
#endif
#ifdef SIGWINCH
    /*
     * Several virtual terminal implementations raise SIGWINCH when the
     * window size changes (e.g. GNU Screen, tmux, xterm).
     */
    {SIGWINCH, "sigwinch", "window size changed"},
#endif

    /*
     * FreeBSD defines this despite the fact it is intended for the
     * real-time library.
     */
#ifdef SIGLIBRT
    {SIGLIBRT, "siglibrt", "real-time library signal"},
#endif

    /*
     * SunOS/Solaris/Illumos
     */
#ifdef SIGCANCEL
    {SIGCANCEL, "sigcancel", "thread cancelled"},
#endif
#ifdef SIGFREEZE
    {SIGFREEZE, "sigfreeze", "checkpoint freeze"},
#endif
#ifdef SIGJVM1
    {SIGJVM1, "sigjvm1", "jvm signal 1"},
#endif
#ifdef SIGJVM2
    {SIGJVM2, "sigjvm2", "jvm signal 2"},
#endif
#ifdef SIGLOST
    {SIGLOST, "siglost", "resource lost"},
#endif
#ifdef SIGLWP
    {SIGLWP, "siglwp", "lightweight process library signal"},
#endif
#ifdef SIGTHAW
    {SIGTHAW, "sigthaw", "checkpoint thaw"},
#endif
#ifdef SIGWAITING
    {SIGWAITING, "sigwaiting", "all lightweight processes blocked"},
#endif
#ifdef SIGXRES
    {SIGXRES, "sigxres", "exceeded resource control"},
#endif

    /* Linux systems define this, but it is apparently never used? */
#ifdef SIGSTKFLT
    {SIGSTKFLT, "sigstkflt", "stack fault"},
#endif

    /*
     * These are in AIX v7.3, released in 2021.
     */
#ifdef SIGDANGER
    {SIGDANGER, "sigdanger", "danger - system page space full"},
#endif
#ifdef SIGGRANT
    {SIGGRANT, "siggrant", "HFT monitor mode granted"},
#endif
#ifdef SIGIOT
    {SIGIOT, "sigiot", "IOT trap"},
#endif
#ifdef SIGMIGRATE
    {SIGMIGRATE, "sigmigrate", "migrate process"},
#endif
#ifdef SIGMSG
    {SIGMSG, "sigmsg", "HFT monitor mode data available"},
#endif
#ifdef SIGRETRACT
    {SIGRETRACT, "sigretract", "HFT monitor mode should be relinguished"},
#endif
#ifdef SIGSAK
    {SIGSAK, "sigsak", "secure attention key"},
#endif
#ifdef SIGSOUND
    {SIGSOUND, "sigsound", "HFT sound control completed"},
#endif
#ifdef SIGTALRM
    {SIGTALRM, "sigtalrm", "thread alarm"},
#endif

    /*
     * Miscellaneous signals
     *
     * These were considered dubious already, but they're kept in the event
     * that some modern system still has them.
     */
#ifdef SIGAIO
    {SIGAIO, "sigaio", "base lan i/o available"},
#endif
#ifdef SIGEMSG
    {SIGEMSG, "sigemsg", "process received an emergency message"},
#endif
#ifdef SIGIOINT
    {SIGIOINT, "sigioint", "printer to backend error"},
#endif
#ifdef SIGPRE
    {SIGPRE, "sigpre", "programming exception"},
#endif
#ifdef SIGPTY
    {SIGPTY, "sigpty", "pty i/o available"},
#endif
#ifdef SIGUNUSED
    {SIGUNUSED, "sigunused", "unused signal"},
#endif
#ifdef SIGVIRT
    {SIGVIRT, "sigvirt", "virtual timer expired"},
#endif
#ifdef SIGWINDOW
    {SIGWINDOW, "sigwindow", "window size changed"},
#endif

    /*
     * Unless I missed something, anything else should be between SIGRTMIN
     * and SIGRTMAX if an OS supports real-time signals, not that they're
     * typically available for general use.
     */
};
const size_t nsig_other = sizeof(other) / sizeof(other[0]);

int es_sigcmp(const void *a, const void *b) {
    const Sigmsgs *sa = a;
    const Sigmsgs *sb = b;
    return sa->sig - sb->sig;
}

void sort_siggroups(void) {
    qsort(posix, nsig_posix, sizeof(posix[0]), es_sigcmp);
    qsort(std  , nsig_std  , sizeof(std  [0]), es_sigcmp);
    qsort(other, nsig_other, sizeof(other[0]), es_sigcmp);
}

char *ucase(const char *lsigname) {
    static char usigname[16];
    char *p;
    strncpy(usigname, lsigname, sizeof(usigname));
    if (memchr(usigname, '\0', sizeof(usigname)) == NULL)
        abort();
    for (p = usigname; *p != '\0'; ++p)
        *p = toupper(*p);
    return usigname;
}

int main(void) {
    Sigmsgs *pposix, *pstd, *pother;
    Sigmsgs *eposix, *estd, *eother;

    eposix = nsig_posix + (pposix = posix);
    estd   = nsig_std   + (pstd   = std);
    eother = nsig_other + (pother = other);

    sort_siggroups();

#define PHEAD() \
    printf("#include \"es.h\"\n" \
	   "#include \"sigmsgs.h\"\n" \
	   "\n" \
           "const Sigmsgs signals[] = {\n")
#define PSIG(ptr) \
    printf("#if %s < NSIG\n" \
	   "\t{%s, \"%s\", \"%s\"},\n" \
	   "#endif\n", \
           ucase((ptr)->name), ucase((ptr)->name), (ptr)->name, (ptr)->msg)
#define PFOOT() \
    printf("};\n" \
           "const int nsignals = (int)(sizeof(signals)/sizeof(signals[0]));\n")

    PHEAD();
    while (pposix != eposix || pstd != estd || pother != eother) {
        int sposix = INT_MAX-1;
        int sstd   = INT_MAX-2;
        int sother = INT_MAX-3;
        if (pposix != eposix) sposix = pposix->sig;
        if (pstd   != estd)   sstd   = pstd->sig;
        if (pother != eother) sother = pother->sig;
#define padvance(signum, ptr) \
        do { \
            PSIG(ptr); \
            ++ptr; \
            for (; pposix != eposix && signum == pposix->sig; ++pposix); \
            for (; pstd   != estd   && signum == pstd  ->sig; ++pstd  ); \
            for (; pother != eother && signum == pother->sig; ++pother); \
        } while (0)
        if (sposix <= sother) {
            if (sposix <= sstd) {
                /* posix <= std   <= other */
                /* posix <= other <  std   */
                padvance(sposix, pposix);
            } else {
                /* std   <  posix <= other */
                padvance(sstd, pstd);
            }
        } else if (sstd <= sother) {
            /* std   <= other <  posix */
            padvance(sstd, pstd);
        } else {
            /* other <  posix <= std   */
            /* other <  std   <  posix */
            padvance(sother, pother);
        }
    }
    PFOOT();

    /* Ensure we include signal.h for signal definitions. */
    (void)signal;
    return 0;
}
