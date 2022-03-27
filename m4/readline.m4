# _ES_READLINE_LIB
# ----------------
# Check whether a readline library is available.
#
# Sets es_cv_readline_lib to `no' if no readline library is available.
#
# Otherwise, es_cv_readline_termlib is set to indicate if a
# terminfo/termcap/curses library is necessary.  If may be set to
# `none required' to indicate no such library needs to be linked.
AC_DEFUN([_ES_READLINE_LIB],
[AC_REQUIRE([AC_PROG_CC])
AS_VAR_IF([es_cv_readline_h], [no],
  [AS_VAR_SET([es_cv_readline_lib], [no])],
  [AC_CACHE_CHECK([for readline library], [es_cv_readline_lib],
    [AS_VAR_SET([es_cv_readline_lib], [no])
    AS_VAR_COPY([es_save_LIBS], [LIBS])
    AS_FOR([es_lib], [[es_lib]],
      [readline edit editline],
      [AS_FOR([es_tlib], [[es_tlib]],
        ["" terminfo tinfo ncurses curses termcap],
        [AS_VAR_COPY([LIBS], [es_save_LIBS])
        AS_VAR_APPEND([LIBS], [" -l[]es_lib[]"])
        AS_IF([test -n es_tlib], [AS_VAR_APPEND([LIBS], [" -l[]es_tlib[]"])])
        AC_LINK_IFELSE(
          [AC_LANG_PROGRAM(
            [char *readline(const char *);],
            [(void)readline("; ");])],
          [AS_VAR_SET([es_cv_readline_lib], [-l[]es_lib])
          AS_IF([test -z es_tlib],
            [AS_VAR_SET([es_cv_readline_termlib], ["none required"])],
            [AS_VAR_SET([es_cv_readline_termlib], [-l[]es_tlib])])
          break])])
      AS_VAR_IF([es_cv_readline_lib], [no], [], [break])])
    AS_VAR_IF([es_cv_readline_lib], [no],
      [AS_VAR_COPY([LIBS], [es_save_LIBS])])])])
])dnl _ES_READLINE_LIB

# _ES_READLINE_H
# --------------
# Check how to include readline.h.
# Sets es_cv_readline_h to `no' if readline.h is missing.
AC_DEFUN([_ES_READLINE_H],
[AC_REQUIRE([AC_PROG_CC])
AS_VAR_IF([es_cv_readline_lib], [no],
  [AS_VAR_SET([es_cv_readline_h], [no])],
  [AC_CACHE_CHECK([for readline.h], [es_cv_readline_h],
    [AS_VAR_SET([es_cv_readline_h], [no])
    AS_FOR([es_h], [[es_h]], [readline/readline.h readline.h],
      [AC_COMPILE_IFELSE(
        [AC_LANG_SOURCE(
          [[#include <stdio.h>]]
          [[#include <]es_h[>]])],
        [AS_VAR_SET([es_cv_readline_h], [es_h])
        break])])])])
])dnl _ES_READLINE_H

# ES_READLINE
# -----------
# Check whether readline is available.
# Sets es_cv_readline to `yes' or `no'.
#
# Defines HAVE_READLINE if available and additionally defines
# HAVE_READLINE_H or HAVE_READLINE_READLINE_H, depending on whether
# <readline.h> or <readline/readline.h> should be included.
#
# The relevant libraries will be added to LIBS for linking.
AC_DEFUN([ES_READLINE],
[AC_ARG_WITH([readline],
  [AS_HELP_STRING(
    [--with-readline],
    [enable line editing with readline @<:@auto@:>@])],
  [],
  [with_readline=auto])
AS_VAR_IF([with_readline], [no],
  [],
  [AC_REQUIRE([_ES_READLINE_LIB])
  AC_REQUIRE([_ES_READLINE_H])
  AC_CACHE_CHECK([whether readline is available], [es_cv_readline],
    [AS_VAR_SET([es_cv_readline], [no])
    AS_CASE(["$es_cv_readline_lib"],
      [no], [],
      [-lreadline|-ledit|-leditline],
          [AS_CASE(["$es_cv_readline_h"],
            [no], [],
            [readline.h|readline/readline.h],
                [AS_VAR_SET([es_cv_readline], [yes])],
            [AS_VAR_SET([es_cv_readline], [error-h])])],
      [AS_VAR_SET([es_cv_readline], [error-lib])])])
  AS_CASE(["$es_cv_readline"],
    [error-lib],
        [AC_MSG_ERROR(
          [unexpected value for cache variable -- $es_cv_readline_lib])],
    [error-h],
        [AC_MSG_ERROR(
          [unexpected value for cache variable -- $es_cv_readline_h])],
    [no],
        [AS_VAR_IF([with_readline], [yes],
          [AC_MSG_FAILURE([readline not available])])],
    [yes],
        [AC_DEFINE([HAVE_READLINE], [1],
          [Define to 1 if readline is available.])
        AS_CASE(["$es_cv_readline_h"],
          [readline.h],
              [AC_DEFINE([HAVE_READLINE_H], [1],
                [Define to 1 if <readline.h> should be included.])],
          [readline/readline.h],
              [AC_DEFINE([HAVE_READLINE_READLINE_H], [1],
                [Define to 1 if <readline/readline.h> should be included.])],
          [AC_MSG_ERROR(
            [unexpected way to include readline.h -- $es_cv_readline_h])])],
    [AC_MSG_ERROR(
      [unexpected value for cache varaible -- $es_cv_readline])])])
])dnl ES_READLINE

# ES_READLINE_HISTORY
# -------------------
# Check how to include the history.h header of readline.
# Sets es_cv_readline_history to `no' if history.h is missing.
#
# Defines HAVE_HISTORY_H or HAVE_READLINE_HISTORY_H, depending on whether
# <history.h> or <readline/history.h> should be included.
AC_DEFUN([ES_READLINE_HISTORY],
[AC_REQUIRE([ES_READLINE])
AS_CASE(["$es_cv_readline"],
  [no], [AS_VAR_SET([es_cv_readline_history], [no])],
  [yes],
      [AC_CACHE_CHECK([for history.h @{:@readline@:}@],
        [es_cv_readline_history],
        [AS_VAR_SET([es_cv_readline_history], [no])
        AS_FOR([es_h], [[es_h]], [readline/history.h history.h],
          [AC_COMPILE_IFELSE(
            [AC_LANG_PROGRAM(
              [[#include <stdio.h>]]
              [[#if !HAVE_READLINE]]
              [[# readline should have been enabled]]
              [[#elif HAVE_READLINE_READLINE_H]]
              [[# include <readline/readline.h>]]
              [[#elif HAVE_READLINE_H]]
              [[# include <readline.h>]]
              [[#else]]
              [[# failed to find readline.h]]
              [[#endif]]
              [[#include <]es_h[>]],
              [[add_history("foo");]])],
            [AS_VAR_SET([es_cv_readline_history], [es_h])
            break])])])],
  [AC_MSG_ERROR([unexpected value for cache variable -- $es_cv_readline])])
AS_CASE(["$es_cv_readline_history"],
  [no], [],
  [history.h],
      [AC_DEFINE([HAVE_HISTORY_H], [1],
        [Define to 1 if you should include <history.h> for history.])],
  [readline/history.h],
      [AC_DEFINE([HAVE_READLINE_HISTORY_H], [1],
        [Define to 1 if should include <readline/history.h> for history.])],
  [AC_MSG_ERROR(
    [unexpected way to include history.h -- $es_cv_readline_history])])
AS_VAR_IF([es_cv_readline_history], [no],
  [],
  [AC_DEFINE([HAVE_READLINE_HISTORY], [1],
    [Define to 1 if readline history is available.])])
])dnl ES_READLINE_HISTORY
