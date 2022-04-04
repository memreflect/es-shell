dnl Process this file with autoconf to produce a configure script.
AC_INIT
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([access.c])

m4_pattern_forbid([^_?ES_])
m4_pattern_forbid([^_es_])

dnl AC_CMDSTDOUT_CPP(variable, command, headers)
AC_DEFUN([AC_CMDSTDOUT_CPP],
[AC_REQUIRE([AC_PROG_CPP])
cat > conftest.$ac_ext <<EOF
[#]line __oline__ "configure"
#include "confdefs.h"
[$3]
EOF
$1=`(eval "$ac_cpp conftest.$ac_ext") 2>&AS_MESSAGE_LOG_FD | $2`
rm -f conftest*
])


AC_CANONICAL_HOST

AS_CASE(["$host"],
  [*sun5*|*solaris2*], [AC_DEFINE([SOLARIS], [1], [Are we Solaris?])])

dnl saved_CFLAGS="$CFLAGS"

dnl Checks for programs.
AC_PROG_CC
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
  [yes], [AC_DEFINE([KERNEL_POUNDBANG], [1], [Does your kernel support #!?])])

dnl Checks for libraries.
AC_SEARCH_LIBS([getpwuid], [sun])
AS_CASE(["$ac_cv_search_getpwuid"],
        [no],              [],
        ["none required"], [],
        [AC_DEFINE([HAVE_LIBSUN], [1],
                   [Define to 1 if you have the 'sun' library (-lsun).])])
ES_READLINE

dnl Checks for header files.
AC_CHECK_HEADERS([fcntl.h unistd.h sys/ioctl.h sys/time.h])

dnl Checks for typedefs, structures, and compiler characteristics.

ES_STDC_VERSION

# Check for _Noreturn
# If C11 or newer, then we assume it is available.  Otherwise, we test for its
# presence.  If it is not available, we check for GCC __attribute__((noreturn)).
AC_CACHE_CHECK(
  [for _Noreturn],
  [es_cv_c_noreturn],
  [AS_VAR_SET([es_cv_c_noreturn], [yes])
  AS_VERSION_COMPARE([$es_stdc_version], [ES_STDC_C11],
                     [AC_COMPILE_IFELSE([AC_LANG_SOURCE([[_Noreturn void
foo(void);]])],
                                        [],
                                        [AS_VAR_SET([es_cv_c_noreturn],
                                                    [no])])])])
AS_VAR_IF([es_cv_c_noreturn],
  [yes], [AC_DEFINE([HAVE__NORETURN], [1],
                    [Define to 1 if you have _Noreturn.])],
  [AC_CACHE_CHECK([if the noreturn attribute is available],
                  [es_cv_c_attribute_noreturn],
                  [AS_VAR_SET([es_cv_c_attribute_noreturn], [no])
                  AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <stdlib.h>
__attribute__((noreturn)) void foo(void) {exit(0);}]])],
                                    [AS_VAR_SET([es_cv_c_attribute_noreturn],
                                                [yes])])])
  AS_VAR_IF([es_cv_c_attribute_noreturn],
            [yes], [AC_DEFINE([HAVE_NORETURN_ATTRIBUTE], [1],
                              [Define to 1 if you have the noreturn attribute.])])])

dnl Checks for library functions.
AC_FUNC_MMAP
AC_FUNC_WAIT3
AC_CHECK_FUNCS([lstat setrlimit setsid strerror strtol sysconf])
AC_CHECK_FUNCS([sigaction sighold sigrelse sigsetjmp])

AC_CACHE_CHECK(for files to extract signal information from,
es_cv_sigfiles,
AC_CMDSTDOUT_CPP(es_cv_sigfiles,
[changequote(,)
egrep '^#[ 	]+1[	 ]+' | sed 's/.*"\(.*\)".*/\1/' |sort -u |
grep '^/' |tr '\012' ' '
changequote([,])],
[#include <signal.h>], /usr/include/signal.h))

AC_SUBST(SIGFILES, $es_cv_sigfiles)

# Check for va_copy or __va_copy
# If C99 or newer, then we assume va_copy is available.  Otherwise, we test for
# va_copy, then __va_copy.
AC_CACHE_CHECK(
  [for va_copy],
  [es_cv_func_va_copy],
  [AS_VAR_SET([es_cv_func_va_copy], [yes])
  AS_VERSION_COMPARE([$es_stdc_version], [ES_STDC_C99],
                     [AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <stdarg.h>
void foo(va_list ap) {
  va_list apsave;
  va_copy(apsave, ap);
  va_arg(ap, int);
  va_end(apsave);
}]])],
                                        [],
                                        [AS_VAR_SET([es_cv_func_va_copy],
                                                    [no])])])])
AS_VAR_IF([es_cv_func_va_copy],
  [no], [AC_CACHE_CHECK([for __va_copy],
                        [es_cv_func___va_copy],
                        [AS_VAR_SET([es_cv_func___va_copy], [no])
                        AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <stdarg.h>
void foo(va_list ap) {
  va_list apsave;
  __va_copy(apsave, ap);
  va_arg(ap, int);
  va_end(apsave);
}]])],
                                    [AS_VAR_SET([es_cv_func___va_copy],
                                                [yes])])])])
AS_VAR_IF([es_cv_func_va_copy],
  [yes], [AC_DEFINE([HAVE_VA_COPY], [1], [Define to 1 if you have va_copy.])],
  [AS_VAR_IF([es_cv_func___va_copy],
             [yes], [AC_DEFINE([HAVE___VA_COPY], [1],
                               [Define to 1 if you have __va_copy.])])])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
