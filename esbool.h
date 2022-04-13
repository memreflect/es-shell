#ifndef ES_ESBOOL_H
#define ES_ESBOOL_H 1

#include "esconfig.h"

#if HAVE_STDBOOL_H
# include <stdbool.h>
#else
# ifndef __cplusplus
#  if !HAVE__BOOL
typedef unsigned char _Bool;
#  endif
#  define bool  _Bool
#  define false bool_false
#  define true  bool_true
#  define __bool_true_false_are_defined 1
# endif
#endif

#endif  /* !ES_ESBOOL_H */
