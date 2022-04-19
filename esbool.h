#ifndef ES_ESBOOL_H
#define ES_ESBOOL_H 1

#include "esconfig.h"

#if ENABLE_C99
# include <stdbool.h>
#else
# ifndef __cplusplus
#  ifndef HAVE__BOOL
typedef unsigned char _Bool;
#  endif
#  define bool  _Bool
#  define false 0
#  define true  1
#  define __bool_true_false_are_defined 1
# endif
#endif

#endif  /* !ES_ESBOOL_H */
