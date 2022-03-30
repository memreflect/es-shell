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

case "$host" in
	*sun5* | *solaris2*)
		AC_DEFINE(SOLARIS, [], [Are we Solaris?])
	;;
esac

dnl saved_CFLAGS="$CFLAGS"

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MKDIR_P
AC_PROG_YACC

dnl CFLAGS="$CFLAGS $saved_CFLAGS"

dnl ----------------------------
dnl CHECK FOR /dev/fd FILESYSTEM
dnl ----------------------------
AC_CACHE_CHECK(for /dev/fd filesystem, es_cv_sys_dev_fd,
[test -d /dev/fd && es_cv_sys_dev_fd=yes || es_cv_sys_dev_fd=no])
if test $es_cv_sys_dev_fd = yes; then
  AC_DEFINE(HAVE_DEV_FD, [1], [Do you have a /dev/fd/ directory?])
fi

AC_SYS_INTERPRETER
if test "$ac_cv_sys_interpreter" = yes
then
	AC_DEFINE(KERNEL_POUNDBANG, [1], [Does your kernel support #!?])
fi


dnl Checks for libraries.
AC_CHECK_LIB(sun, getpwuid)
ES_READLINE

dnl Checks for header files.
AC_CHECK_HEADERS([fcntl.h unistd.h sys/ioctl.h sys/time.h])

dnl Checks for typedefs, structures, and compiler characteristics.

# Check for _Noreturn
# Some implementations define _Noreturn to __attribute__((noreturn))
# depending on the values of __GNUC__ and __GNUC_MINOR__.
AC_CACHE_CHECK([whether _Noreturn is available], [es_cv_noreturn],
               [AS_VAR_SET([es_cv_noreturn], [no])
               AC_COMPILE_IFELSE(
                 [AC_LANG_SOURCE([[_Noreturn void foo(void);]])],
                 [AS_VAR_SET([es_cv_noreturn], [yes])],
                 [AC_COMPILE_IFELSE(
                   [AC_LANG_SOURCE([[#include <stdio.h>]])],
                   [AS_VAR_SET([es_cv_noreturn], [yes])])])])
AS_VAR_IF([es_cv_noreturn], [yes],
          [AC_DEFINE([HAVE__NORETURN], [1],
             [Define to 1 if you have _Noreturn functionality.])],
          [AC_CACHE_CHECK(
            [if the noreturn attribute is available],
            [es_cv_attrib_noreturn],
            [AS_VAR_SET([es_cv_attrib_noreturn], [no])
            AC_COMPILE_IFELSE(
              [AC_LANG_SOURCE([[#include <stdlib.h>
                              __attribute__((noreturn)) void
                              foo(void) {exit(0);}]])],
              [AS_VAR_SET([es_cv_attrib_noreturn], [yes])])])
          AS_VAR_IF([es_cv_attrib_noreturn], [yes],
                    [AC_DEFINE([HAVE_NORETURN_ATTRIB], [1],
                      [Define to 1 if you have the noreturn attribute.])])])
dnl Checks for library functions.
AC_FUNC_MMAP
AC_FUNC_WAIT3
AC_CHECK_FUNCS(strerror strtol lstat setrlimit sigrelse sighold sigaction \
sysconf setsid sigsetjmp)

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
# If C11 features are available, then we assume va_copy is available as all
# C implementations since C99 are required to have it.
# If neither va_copy nor __va_copy is found, check whether a compiler has a
# flag to enable C99 mode, which will also ensure va_copy is available.
# If that fails, then we have neither.
AC_CACHE_CHECK([for va_copy], [es_cv_va_copy],
               [AS_VAR_IF([ac_cv_prog_cc_c11], [no],
                 [AC_COMPILE_IFELSE(
                   [AC_LANG_SOURCE(
                     [[#include <stdarg.h>
                     void foo(va_list ap) {
                        va_list apsave;
                        va_copy(apsave, ap);
                        va_end(apsave);
                     }]])],
                   [AS_VAR_SET([es_cv_va_copy], [yes])],
                   [AS_VAR_SET([es_cv_va_copy], [no])])],
                 [AS_VAR_SET([es_cv_va_copy], [yes])])])
AS_VAR_IF([es_cv_va_copy], [no],
          [AC_CACHE_CHECK([for __va_copy], [es_cv___va_copy],
                          [AC_COMPILE_IFELSE(
                            [AC_LANG_SOURCE(
                              [[#include <stdarg.h>
                              void foo(va_list ap) {
                                 va_list apsave;
                                 __va_copy(apsave, ap);
                                 va_end(apsave);
                              }]])],
                            [AS_VAR_SET([es_cv___va_copy], [yes])],
                            [AS_VAR_SET([es_cv___va_copy], [no])])])])
AS_IF([test "$es_cv_va_copy:$es_cv___va_copy" = "no:no"],
      [AS_VAR_COPY([es_save_CC], [CC])
      AS_VAR_COPY([es_save_CFLAGS], [CFLAGS])
      AC_PROG_CC_C99([AS_VAR_SET([es_cv_va_copy], [yes])],
                     [AS_VAR_COPY([CC], [es_save_CC])
                     AS_VAR_COPY([CFLAGS], [es_save_CFLAGS])])])
AS_VAR_IF([es_cv_va_copy], [yes],
          [AC_DEFINE([HAVE_VA_COPY], [1],
                     [Define to 1 if you have va_copy.])],
          [AS_VAR_IF([es_cv___va_copy], [yes],
                     [AC_DEFINE([HAVE___VA_COPY], [1],
                                [Define to 1 if you have __va_copy.])])])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
