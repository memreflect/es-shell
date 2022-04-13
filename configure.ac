dnl Process this file with autoconf to produce a configure script.
AC_INIT
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([access.c])

m4_pattern_forbid([^_?ES_])
m4_pattern_forbid([^_es_])

ES_WITH_READLINE

AC_CANONICAL_HOST

AS_CASE(["$host"],
  [*sun5*|*solaris2*], [AC_DEFINE([SOLARIS], [1], [Are we Solaris?])])

dnl saved_CFLAGS="$CFLAGS"

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_EGREP
AC_PROG_INSTALL
AC_PROG_MKDIR_P
AC_PROG_YACC

# Check for /dev/fd filesystem
AC_CACHE_CHECK(
  [for /dev/fd filesystem],
  [es_cv_sys_dev_fd],
  [AS_IF([test -d /dev/fd],
         [AS_VAR_SET([es_cv_sys_dev_fd], [yes])],
         [AS_VAR_SET([es_cv_sys_dev_fd], [no])])])
AS_VAR_IF([es_cv_sys_dev_fd],
  [yes], [AC_DEFINE([HAVE_DEV_FD], [1], [Do you have a /dev/fd/ directory?])])

AC_SYS_INTERPRETER
AS_VAR_IF([ac_cv_sys_interpreter],
  [yes], [AC_DEFINE([KERNEL_POUNDBANG], [1],
                    [Does your kernel support @%:@!?])])

dnl Checks for libraries.
AC_SEARCH_LIBS([getpwuid], [sun])
AS_CASE(["$ac_cv_search_getpwuid"],
        [no],              [],
        ["none required"], [],
        [AC_DEFINE([HAVE_LIBSUN], [1],
                   [Define to 1 if you have the 'sun' library (-lsun).])])
ES_LIB_READLINE

dnl Checks for header files.
AC_CHECK_HEADERS([fcntl.h unistd.h sys/ioctl.h sys/time.h])
AC_HEADER_STDBOOL
ES_HEADER_READLINE

dnl Checks for typedefs, structures, and compiler characteristics.
ES_STDC_VERSION

# Check for boolean type
# If AC_HEADER_STDBOOL found a working <stdbool.h>, we use it.
# If C99 or newer, then we assume _Bool is available.
# Otherwise, we test for it.
m4_define([_es_bool], [ac_cv_type__Bool])
AH_TEMPLATE([bool_false], [Define to 0 (pre-C2x) or ((_Bool)+0u).])
AH_TEMPLATE([bool_true], [Define to 1 (pre-C2x) or ((_Bool)+1u).])
AS_VAR_IF([ac_cv_header_stdbool_h],
  [no], [AC_CACHE_CHECK([for boolean type support],
                        [_es_bool],
                        [AS_VAR_SET([_es_bool], [yes])
                        AS_VERSION_COMPARE([$es_stdc_version], [ES_STDC_C99],
                                           [AS_VAR_SET([_es_bool], [no])])
                        AS_VAR_IF([_es_bool],
                                  [no], [AC_COMPILE_IFELSE([AC_LANG_SOURCE([_Bool b;])],
                                                           [],
                                                           [AS_VAR_SET([_es_bool],
                                                                       [no])])])])
        AS_VAR_IF([_es_bool],
                  [yes], [AS_VERSION_COMPARE([$es_stdc_version], [ES_STDC_C17],
                                             [AC_DEFINE([bool_false], [0])
                                             AC_DEFINE([bool_true], [1])],
                                             [AC_DEFINE([bool_false], [0])
                                             AC_DEFINE([bool_true], [1])],
                                             [AC_DEFINE([bool_false],
                                                        [((_Bool)+0u)])
                                             AC_DEFINE([bool_true],
                                                       [((_Bool)+1u)])])])])

# Check for _Noreturn
# If C11 or newer, then we assume it is available.  Otherwise, we test for its
# presence.  If it is not available, we check for GCC __attribute__((noreturn)).
m4_define([_es_noreturn], [es_cv_c__Noreturn])
m4_define([_es_noreturn_attrib], [es_cv_c_attribute_noreturn])
AH_TEMPLATE([HAVE__NORETURN], [Define to 1 if you have _Noreturn.])
AH_TEMPLATE([HAVE_NORETURN_ATTRIBUTE],
  [Define to 1 if you have the noreturn attribute.])
AC_CACHE_CHECK(
  [for _Noreturn],
  [_es_noreturn],
  [AS_VAR_SET([_es_noreturn], [yes])
  AS_VERSION_COMPARE([$es_stdc_version], [ES_STDC_C11],
                     [AC_COMPILE_IFELSE([AC_LANG_SOURCE([[_Noreturn void
foo(void);]])],
                                        [],
                                        [AS_VAR_SET([_es_noreturn], [no])])])])
AS_VAR_IF([_es_noreturn],
  [yes], [AC_DEFINE([HAVE__NORETURN])],
  [AC_CACHE_CHECK([if the noreturn attribute is available],
                  [_es_noreturn_attrib],
                  [AS_VAR_SET([_es_noreturn_attrib], [no])
                  AC_COMPILE_IFELSE([AC_LANG_SOURCE([[__attribute__((noreturn))
void foo(void);]])],
                                    [AS_VAR_SET([_es_noreturn_attrib], [yes])])])
  AS_VAR_IF([_es_noreturn_attrib],
            [yes], [AC_DEFINE([HAVE_NORETURN_ATTRIBUTE])])])

dnl Checks for library functions.
AC_FUNC_MMAP
AC_FUNC_WAIT3
AC_CHECK_FUNCS([setrlimit sigaction sigsetjmp sysconf])

# Check for va_copy or __va_copy
# If C99 or newer, then we assume va_copy is available.  Otherwise, we test for
# va_copy, then __va_copy.
m4_define([_es_va_copy], [es_cv_func_va_copy])
m4_define([_es_va_copy_alt], [es_cv_func___va_copy])
AH_TEMPLATE([HAVE_VA_COPY], [Define to 1 if you have va_copy.])
AH_TEMPLATE([HAVE___VA_COPY], [Define to 1 if you have __va_copy.])
AC_CACHE_CHECK(
  [for va_copy],
  [_es_va_copy],
  [AS_VAR_SET([_es_va_copy], [yes])
  AS_VERSION_COMPARE([$es_stdc_version], [ES_STDC_C99],
                     [AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <stdarg.h>
void foo(va_list ap) {
  va_list apsave;
  va_copy(apsave, ap);
  va_arg(ap, int);
  va_end(apsave);
}]])],
                                        [],
                                        [AS_VAR_SET([_es_va_copy], [no])])])])
AS_VAR_IF([_es_va_copy],
  [no], [AC_CACHE_CHECK([for __va_copy],
                        [_es_va_copy_alt],
                        [AS_VAR_SET([_es_va_copy_alt], [no])
                        AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <stdarg.h>
void foo(va_list ap) {
  va_list apsave;
  __va_copy(apsave, ap);
  va_arg(ap, int);
  va_end(apsave);
}]])],
                                          [AS_VAR_SET([_es_va_copy_alt],
                                                      [yes])])])])
AS_VAR_IF([_es_va_copy],
  [yes], [AC_DEFINE([HAVE_VA_COPY])],
  [AS_VAR_IF([_es_va_copy_alt],
             [yes], [AC_DEFINE([HAVE___VA_COPY])])])

ES_READLINE
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
