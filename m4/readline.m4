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
  [ac_cv_search_readline],
  [AS_VAR_SET([ac_cv_search_readline], [no])
  AS_VAR_COPY([es_save_LIBS], [LIBS])
  AS_FOR([_es_lib],
         [es_lib],
         [readline edit editline],
         [AS_VAR_COPY([LIBS], [es_save_LIBS])
         AS_VAR_APPEND([LIBS], [" -l[]_es_lib[]"])
         AC_LINK_IFELSE([AC_LANG_CALL([], [readline])],
                        [AS_VAR_SET([ac_cv_search_readline], [-l[]_es_lib])
                        AS_VAR_SET([ac_cv_search_readline_termlib],
                                   ["none required"])
                        break],
                        [AS_VAR_COPY([es_save_LIBS2], [LIBS])
                        AS_FOR([_es_tlib],
                               [es_tlib],
                               [terminfo tinfo ncurses curses termcap],
                               [AS_VAR_COPY([LIBS], [es_save_LIBS2])
                               AS_VAR_APPEND([LIBS], [" -l[]_es_tlib[]"])
                               AC_LINK_IFELSE([AC_LANG_CALL([], [readline])],
                                              [AS_VAR_SET([ac_cv_search_readline],
                                                          [-l[]_es_lib])
                                              AS_VAR_SET([ac_cv_search_readline_termlib],
                                                         [-l[]_es_tlib])
                                              break])])
                        AS_VAR_IF([ac_cv_search_readline],
                                  [no], [],
                                  [break])])])
  AS_VAR_COPY([LIBS], [es_save_LIBS])])
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
  [no], [AS_VAR_SET([es_cv_readline], [no])],
  [check|auto|yes],
      [ES_READLINE_LIB
      AS_VAR_IF([ac_cv_search_readline],
                [no], [],
                [ES_READLINE_H])
      AS_VAR_IF([ac_cv_header_readline_readline_h],
                [no], [AS_VAR_IF([ac_cv_header_readline_h],
                                 [no], [AS_VAR_SET([ac_cv_search_readline],
                                                   [no])],
                                 [AS_VAR_IF([ac_cv_search_readline],
                                            [no], [],
                                            [AC_CHECK_HEADERS([readline.h])])])],
                [AS_VAR_IF([ac_cv_search_readline],
                           [no], [],
                           [AC_CHECK_HEADERS([readline/readline.h])])])
      AS_VAR_IF([ac_cv_search_readline],
                [no], [],
                [AS_VAR_APPEND([LIBS], [" $ac_cv_search_readline"])
                AS_VAR_IF([ac_cv_search_readline_termlib],
                          ["none required"], [],
                          [AS_VAR_APPEND([LIBS],
                                         [" $ac_cv_search_readline_termlib"])])
                AC_DEFINE([HAVE_READLINE], [1],
                          [Define to 1 if readline is available.])
                ES_READLINE_HISTORY
                AS_VAR_IF([ac_cv_header_readline_history_h],
                          [no], [AS_VAR_IF([ac_cv_header_history_h],
                                           [no], [],
                                           [AC_CHECK_HEADERS([history.h])])],
                          [AC_CHECK_HEADERS([readline/history.h])])
                AS_CASE(["$ac_cv_header_readline_history_h:$ac_cv_header_history_h"],
                        [no:no], [],
                        [AC_DEFINE([HAVE_READLINE_HISTORY], [1],
                                   [Define to 1 if readline history features are available.])])])],
  [AC_MSG_ERROR([invalid argument to --with-readline -- $with_readline])])
])# ES_READLINE
