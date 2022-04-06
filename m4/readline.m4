m4_define([_es_cv_header], [ac_cv_header_readline_h])
m4_define([_es_cv_header_long], [ac_cv_header_readline_readline_h])
m4_define([_es_cv_history], [ac_cv_header_history_h])
m4_define([_es_cv_history_long], [ac_cv_header_readline_history_h])
m4_define([_es_cv_lib], [ac_cv_search_readline])
m4_define([_es_cv_termlib], [ac_cv_search_readline_termlib])

# ES_READLINE_LIB
# ---------------
# Act like AC_SEARCH_LIBS to determine whether a readline library is available.
#
# If one is found, ac_cv_search_readline_termlib is set to indicate if a
# terminfo/termcap/curses library is necessary.  It may be set to
# 'none required', indicating no such library needs to be linked.
#
# This does not add the relevant libraries to LIBS.
AC_DEFUN([ES_READLINE_LIB],
[AC_REQUIRE([AC_PROG_CC])
AC_CACHE_CHECK(
  [for readline],
  [_es_cv_lib],
  [AS_VAR_SET([_es_cv_lib], [no])
  m4_pushdef([_es_save], [es_save_LIBS])
  AS_VAR_COPY([_es_save], [LIBS])
  AS_FOR([_es_lib],
         [es_lib],
         [readline edit editline],
         [AS_VAR_COPY([LIBS], [_es_save])
         AS_VAR_APPEND([LIBS], [" -l[]_es_lib[]"])
         AC_LINK_IFELSE([AC_LANG_CALL([], [readline])],
                        [AS_VAR_SET([_es_cv_lib], [-l[]_es_lib])
                        AS_VAR_SET([_es_cv_termlib], ["none required"])
                        break],
                        [m4_pushdef([_es_save], _es_save[2])
                        AS_VAR_COPY([_es_save], [LIBS])
                        AS_FOR([_es_tlib],
                               [es_tlib],
                               [terminfo tinfo ncurses curses termcap],
                               [AS_VAR_COPY([LIBS], [_es_save])
                               AS_VAR_APPEND([LIBS], [" -l[]_es_tlib[]"])
                               AC_LINK_IFELSE([AC_LANG_CALL([], [readline])],
                                              [AS_VAR_SET([_es_cv_lib],
                                                          [-l[]_es_lib])
                                              AS_VAR_SET([_es_cv_termlib],
                                                         [-l[]_es_tlib])
                                              break])])
                        AS_VAR_IF([_es_cv_lib],
                                  [no], [],
                                  [break])
                        m4_popdef([_es_save])])])
  AS_VAR_COPY([LIBS], [_es_save])])
  m4_popdef([_es_save])
])# ES_READLINE_LIB

# ES_READLINE_H
# -------------
# Use AC_CHECK_HEADER to determine how to include readline.h.
#
# This does not define any HAVE_xxx preprocessor macros.
AC_DEFUN([ES_READLINE_H],
[AC_CHECK_HEADER([readline/readline.h], [], [AC_CHECK_HEADER([readline.h])])
])# ES_READLINE_H

# ES_READLINE_HISTORY
# -------------------
# Use AC_CHECK_HEADERS to determine how to include readline's history.h.
#
# This defines preprocessor macros indicating the header to include and also
# defines HAVE_READLINE_HISTORY.
AC_DEFUN([ES_READLINE_HISTORY],
[AC_CHECK_HEADER([readline/history.h], [], [AC_CHECK_HEADER([history.h])])
])# ES_READLINE_HISTORY

# ES_READLINE
# -----------
# Check whether readline is available.
#
# This is a wrapper around ES_READLINE_LIB, ES_READLINE_H, and
# ES_READLINE_HISTORY that defines HAVE_READLINE if a library is usable and the
# readline.h header is found.
#
# Defines HAVE_READLINE_H or HAVE_READLINE_READLINE_H, depending on whether
# <readline.h> or <readline/readline.h> should be included.
#
# The relevant libraries are also added to LIBS for linking.
#
# Finally, this macro uses AC_ARG_WITH to add a configure option --with-readline
# to disable or require readline.  The default is 'auto' to automatically detect
# whether it is available or not.  'check' is also accepted as an equivalent
# alternative to 'auto'.
AC_DEFUN([ES_READLINE],
[AC_ARG_WITH(
  [readline],
  [AS_HELP_STRING([--with-readline],
                  [enable line editing with readline @<:@auto@:>@])],
  [],
  [AS_VAR_SET([with_readline], [auto])])
AS_CASE(["$with_readline"],
  [no], [],
  [check|auto|yes],
      [AH_TEMPLATE([HAVE_READLINE_READLINE_H],
                   [Define to 1 if you have <readline/readline.h>.])
      AH_TEMPLATE([HAVE_READLINE_H],
                  [Define to 1 if you have <readline.h>.])
      AH_TEMPLATE([HAVE_READLINE],
                  [Define to 1 if you have readline functionality.])
      AH_TEMPLATE([HAVE_READLINE_HISTORY_H],
                  [Define to 1 if you have <readline/history.h>.])
      AH_TEMPLATE([HAVE_HISTORY_H],
                  [Define to 1 if you have <history.h>.])
      AH_TEMPLATE([HAVE_READLINE_HISTORY],
                  [Define to 1 if readline history features are available.])
      ES_READLINE_LIB
      AS_VAR_IF([_es_cv_lib],
                [no], [],
                [ES_READLINE_H
                AS_VAR_IF([_es_cv_header_long],
                          [no], [AS_VAR_IF([_es_cv_header],
                                           [no], [AS_VAR_SET([_es_cv_lib],
                                                             [no])],
                                           [AC_DEFINE([HAVE_READLINE_H])])],
                          [AC_DEFINE([HAVE_READLINE_READLINE_H])])])
      AS_VAR_IF([_es_cv_lib],
                [no], [AS_VAR_IF([with_readline],
                                 [yes], [AC_MSG_ERROR([cannot enable readline])])],
                [AS_VAR_APPEND([LIBS], [" $[]_es_cv_lib[]"])
                AS_VAR_IF([_es_cv_termlib],
                          ["none required"], [],
                          [AS_VAR_APPEND([LIBS], [" $[]_es_cv_termlib[]"])])
                AC_DEFINE([HAVE_READLINE])
                ES_READLINE_HISTORY
                AS_VAR_IF([_es_cv_history_long],
                          [no], [AS_VAR_IF([_es_cv_history],
                                           [no], [],
                                           [AC_DEFINE([HAVE_HISTORY_H])
                                           AC_DEFINE([HAVE_READLINE_HISTORY])])],
                          [AC_DEFINE([HAVE_READLINE_HISTORY_H])
                          AC_DEFINE([HAVE_READLINE_HISTORY])])])],
  [AC_MSG_ERROR([invalid argument to --with-readline -- $with_readline])])
])# ES_READLINE
