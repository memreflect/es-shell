#ifndef ES_GETOPT_C
#define ES_GETOPT_C 1

#include <stdio.h>
#include <string.h>

#define OPTINFO_INITIALIZER \
	{ NULL, 1, 0, 0 }

static struct optinfo {
	/* -f => arg=NULL
	   -fVALUE, -f VALUE => arg="VALUE" */
	char *arg;
	/*
	 * ind: argv index, i.e. argv[ind]
	 * opt: option character found on command-line
	 * pos: position of next character in argv[ind], i.e. argv[ind][pos]
	 */
	int ind, opt, pos;
} getopt_info = OPTINFO_INITIALIZER;

static void
getopt_reset(void) {
	const static struct optinfo ref = OPTINFO_INITIALIZER;
	getopt_info                            = ref;
}

/*
 * getopt_internal: return the next recognized command-line option
 *
 * # Command-line semantics
 *
 * Option recognition ends when an argument, including "-", is encountered.
 * Option recognition also ends when a standalone "--" argument is used;
 * "--" is discarded, and all subsequent arguments should be treated as
 * program operands.
 *
 * Flag values may be specified adjacent to the flag character as -fVALUE or in
 * the subsequent argument as in -f value.
 * Flag characters without arguments may be concatenated, meaning -xevcCMD is
 * equivalent to -x -e -v -cCMD.
 *
 * # Function semantics
 *
 * optstring is a string containing all option characters.
 * If a ':' character appears after the option character in optstring,
 * it requires a value.
 * Otherwise, the flag does not accept a value.
 * If the first character of optstring is a ':', error messages are suppressed.
 *
 * If option recognition ends, -1 is returned.
 * If a option is unknown, the entire argument is discarded, an error message is
 * printed if not suppressed, and '?' is returned.
 * If a option is missing a value, an error message is printed if not suppressed
 * and ':' is returned.
 *
 * # Example
 *
 * int c;
 * getopt_reset();
 * while ((c = getopt_internal(argc, argv, "eilxvnpodsc:?GIL")) != -1) {
 *   switch (c) {
 *   case 'c':
 *     cmd = getopt_info.arg;
 *     break;
 *   case 'e':
 *     runflags |= eval_exitonfalse;
 *     break;
 *   ...
 *   default:
 *     usage();
 *   }
 * }
 * argc -= getopt_info.ind;
 * argv += getopt_info.ind;
 */
static int
getopt_internal(int argc, char *const *argv, const char *optstring) {
	char *c;
	if (getopt_info.pos != 0) {
		/* in the middle of parsing concatenated short options,
		   pos is correctly set */
	} else if (getopt_info.ind >= argc || argv[getopt_info.ind][0] != '-'
	           || argv[getopt_info.ind][1] == '\0') {
		/* non-option argument encountered or end of argument list,
		   end parsing */
		return -1;
	} else {
		if (argv[getopt_info.ind][1] == '-' && argv[getopt_info.ind][2] == '\0') {
			/* "--" encountered, skip "--" and end parsing */
			getopt_info.ind++;
			return -1;
		}
		getopt_info.pos = 1;
	}

	getopt_info.arg = NULL;
	getopt_info.opt = argv[getopt_info.ind][getopt_info.pos];

	if (getopt_info.opt == ':' || (c = strchr(optstring, getopt_info.opt)) == NULL) {
		/* print error and skip rest of this argument */
		if (*optstring != ':') {
			fprintf(stderr, "es: unknown option -%c\n", getopt_info.opt);
		}
		getopt_info.pos++;
		if (argv[getopt_info.ind][getopt_info.pos] == '\0') {
			getopt_info.ind++;
			getopt_info.pos = 0;
		}
		return '?';
	}

	if (c[1] != ':') {
		/* option does not require a value,
		   advance to the next short option or to the next argument */
		getopt_info.pos++;
		if (argv[getopt_info.ind][getopt_info.pos] == '\0') {
			getopt_info.ind++;
			getopt_info.pos = 0;
		}
		return *c;
	}

	/* option requires a value */
	switch (argv[getopt_info.ind][getopt_info.pos + 1]) {
	case '\0':
		/* -f ARG? */
		getopt_info.ind++;
		getopt_info.pos = 0;
		if (getopt_info.ind >= argc) {
			if (*optstring != ':') {
				fprintf(stderr, "es: missing value for -%c\n", getopt_info.opt);
			}
			return ':';
		}
		getopt_info.arg = argv[getopt_info.ind];
		break;
	default:
		/* -fARG */
		getopt_info.arg = &argv[getopt_info.ind][getopt_info.pos + 1];
		getopt_info.pos = 0;
		break;
	}
	getopt_info.ind++;
	return *c;
}

#endif /* not ES_GETOPT_C */
