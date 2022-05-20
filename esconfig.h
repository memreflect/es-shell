/* esconfig.h -- es(1) configuration parameters */
#ifndef ES_ESCONFIG_H
#define ES_ESCONFIG_H 1

#include "config.h"

/*
 * Compile time options
 *
 *	These options are best set on the command line in the Makefile.
 *	If the machine you use requires a different set of defaults than
 *	is provided, please send mail to
 *
 *		Paul Haahr <haahr@netcom.com>
 *		Byron Rakitzis <byron@netapp.com>
 *
 *	If you decide to add things to this file, add them before the
 *	defaults and make sure that they can be overriden by command
 *	line definitions.  (That is, remember to do the #ifndef dance.)
 *
 *
 *	ASSERTIONS
 *		if this is on, asserts will be checked, raising errors on
 *		actual assertion failure.
 *
 *	DEVFD_PATH
 *		(used only if DEVFD is on.)  a format string for print() to
 *		a file path for opening file descriptor n.
 *
 *	GCALWAYS
 *		if this is on, the a collection is done after every allocation.
 *		this stress-tests the garbage collector.  any missed Ref()
 *		declarations should cause a crash or assertion failure very
 *		quickly in this mode.
 *
 *	GCDEBUG
 *		when this is on, the garbage collector is run in such a way
 *		that just about any coding error will lead to an almost
 *		immediate crash.  it is equivalent to all 3 of GCALWAYS,
 *		GCPROTECT, and GCVERBOSE
 *
 *	GCINFO
 *		a terse version of GCVERBOSE, which prints a short message
 *		for every collection.
 *
 *	GCPROTECT
 *		makes the garbage collector disable access to pages
 *		that are in old space, making unforwarded references
 *		crasht the interpreter.  requires os/mmu support for
 *		enabling and disabling access to pages.
 *
 *	GCVERBOSE
 *		if this is on, it is possible to run the garbage collector
 *		in a mode where it explains what it is doing at all times.
 *		implied by GCDEBUG.
 *
 *	INITIAL_PATH
 *		this is the default value for $path (and $PATH) when the shell
 *		starts up.  it is replaced by one from the environment if it
 *		exists.
 *
 *	KERNEL_POUNDBANG
 *		this value should be true when the builtin version of execve(2)
 *		understands #! headers.  if false, es provides a builtin for
 *		running #! files.  the default is true; are there any real
 *		systems still out there that don't support it?
 *
 *	JOB_PROTECT
 *		set this to true if you want es to perform
 *		backgrounding as if it were a job controlling shell;
 *		that is, if you want background jobs to be put in new
 *		process groups.  this flag is ignored if the system
 *		does not support the job control signals.  since there
 *		are many broken programs that do not behave correctly
 *		when backgrounded in a v7 non-job-control fashion, the
 *		default for this option is on, even though it is ugly.
 *
 *	PROTECT_ENV
 *		if on, makes all variable names in the environment ``safe'':
 *		that is, makes sure no characters other than c identifier
 *		characters appear in them.
 *
 *	REF_ASSERTIONS
 *		if this is on, assertions about the use of the Ref() macro
 *		will be checked at run-time.  this is only useful if you're
 *		modifying es source, and makes the binary much larger.
 *
 *	SHOW_DOT_FILES
 *		if this option is off (the default), wildcard patterns do not
 *		match files that being with a ``.'' character;  this behavior
 *		is the same as in most unix shells.  if it is on, the only
 *		files not matched are ``.'' and ``..'';  this behavior follows
 *		convention on bell labs research unix systems (since the eighth
 *		edition) and plan 9.  in either case, an explicit ``.'' at
 *		the beginning of a pattern will match the hidden files.
 *		(Contributed by Steve Kilbane.)
 */

/*
 * add your customizations here
 */

/*
 * default defaults -- don't change this section
 */

#ifndef ASSERTIONS
#	define ASSERTIONS 1
#endif

#ifndef DEVFD_PATH
#	define DEVFD_PATH "/dev/fd/%d"
#endif

#ifndef GCALWAYS
#	define GCALWAYS 0
#endif

#ifndef GCDEBUG
#	define GCDEBUG 0
#endif

#ifndef GCINFO
#	define GCINFO 0
#endif

#ifndef GCPROTECT
#	define GCPROTECT 0
#endif

#ifndef GCVERBOSE
#	define GCVERBOSE 0
#endif

#ifndef INITIAL_PATH
#	define INITIAL_PATH "/usr/ucb", "/usr/bin", "/bin", ""
#endif

#ifndef JOB_PROTECT
#	define JOB_PROTECT 1
#endif

#ifndef PROTECT_ENV
#	define PROTECT_ENV 1
#endif

#ifndef REF_ASSERTIONS
#	define REF_ASSERTIONS 0
#endif

#ifndef SHOW_DOT_FILES
#	define SHOW_DOT_FILES 0
#endif

/*
 * enforcing choices that must be made
 */

#if GCDEBUG
#	undef GCALWAYS
#	undef GCINFO
#	undef GCPROTECT
#	undef GCVERBOSE
#	define GCALWAYS  1
#	define GCINFO    1
#	define GCPROTECT 1
#	define GCVERBOSE 1
#endif

#endif /* !ES_ESCONFIG_H */
