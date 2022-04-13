m4_define([_es_cv_rlheader_bare], [ac_cv_header_readline_h])
m4_define([_es_cv_rlheader], [ac_cv_header_readline_readline_h])
m4_define([_es_cv_rlhistory_bare], [ac_cv_header_history_h])
m4_define([_es_cv_rlhistory], [ac_cv_header_readline_history_h])
m4_define([_es_cv_rllib], [ac_cv_search_readline])
m4_define([_es_cv_rltermlib], [ac_cv_search_readline_termlib])
m4_define([_es_cv_rlenable], [es_cv_readline_enable])

# ES_WITH_READLINE
# ----------------
# Add --with-readline option
#
# The default is 'auto' to automatically detect whether it is available or not.
# 'check' is also accepted as an equivalent alternative to 'auto'.
AC_DEFUN([ES_WITH_READLINE],
[AC_ARG_WITH(
  [readline],
  [AS_HELP_STRING([--with-readline],
                  [enable line editing with readline @<:@auto@:>@])],
  [],
  [AS_VAR_SET([with_readline], [auto])])
AS_CASE(["$with_readline"],
  [no|check|auto|yes], [],
  [AC_MSG_ERROR([invalid argument to --with-readline: $with_readline])])
])# ES_WITH_READLINE

# _ES_LIB_READLINE
# ----------------
# Helper for ES_LIB_READLINE that actually does the work
AC_DEFUN([_ES_LIB_READLINE],
[AC_CACHE_CHECK(
  [for library containing readline],
  [_es_cv_rllib],
  [AS_VAR_SET([_es_cv_rllib], [no])
  m4_pushdef([_es_save], [es_save_LIBS])
  AS_VAR_COPY([_es_save], [LIBS])
  AS_FOR([_es_lib],
         [es_lib],
         [readline edit editline],
         [AS_VAR_COPY([LIBS], [_es_save])
         AS_VAR_APPEND([LIBS], [" -l[]_es_lib[]"])
         AC_LINK_IFELSE([AC_LANG_CALL([], [readline])],
                        [AS_VAR_SET([_es_cv_rllib], [-l[]_es_lib])
                        AS_VAR_SET([_es_cv_rltermlib], ["none required"])
                        break],
                        [m4_pushdef([_es_save], _es_save[2])
                        AS_VAR_COPY([_es_save], [LIBS])
                        AS_FOR([_es_tlib],
                               [es_tlib],
                               [terminfo tinfo ncurses curses termcap],
                               [AS_VAR_COPY([LIBS], [_es_save])
                               AS_VAR_APPEND([LIBS], [" -l[]_es_tlib[]"])
                               AC_LINK_IFELSE([AC_LANG_CALL([], [readline])],
                                              [AS_VAR_SET([_es_cv_rllib],
                                                          [-l[]_es_lib])
                                              AS_VAR_SET([_es_cv_rltermlib],
                                                         [-l[]_es_tlib])
                                              break])])
                        AS_VAR_IF([_es_cv_rllib],
                                  [no], [],
                                  [break])
                        m4_popdef([_es_save])])])
  AS_VAR_COPY([LIBS], [_es_save])
  m4_popdef([_es_save])])
])# _ES_LIB_READLINE

# ES_LIB_READLINE
# ---------------
# Act like AC_SEARCH_LIBS to determine whether a readline library is available.
#
# If a readline library is found, ac_cv_search_readline_termlib is set to
# indicate if a terminfo/termcap/curses library is necessary.  It may be set to
# 'none required', indicating no such library needs to be linked.
#
# Note: this does not add the relevant libraries to LIBS.
AC_DEFUN([ES_LIB_READLINE],
[AC_REQUIRE([AC_PROG_CC])
AC_REQUIRE([ES_WITH_READLINE])
AS_VAR_IF([with_readline],
  [no], [AS_VAR_SET([_es_cv_rllib], [no])],
  [_ES_LIB_READLINE])
])# ES_LIB_READLINE

# ES_HEADER_READLINE
# ------------------
# Use AC_CHECK_HEADER to determine how to include readline.h and history.h
#
# This does not define any HAVE_xxx preprocessor macros.
AC_DEFUN([ES_HEADER_READLINE],
[AC_REQUIRE([ES_LIB_READLINE])
AS_VAR_IF([_es_cv_rllib],
  [no], [AS_VAR_SET([_es_cv_rlheader], [no])
        AS_VAR_SET([_es_cv_rlheader_bare], [no])],
  [AC_CHECK_HEADER([readline/readline.h],
                  [],
                  [AC_CHECK_HEADER([readline.h])])
  AS_VAR_IF([_es_cv_rlheader],
            [no], [AS_VAR_IF([_es_cv_rlheader_bare],
                             [no], [AS_VAR_SET([_es_cv_rlhistory], [no])
                                   AS_VAR_SET([_es_cv_rlhistory_bare], [no])])],
            [AC_CHECK_HEADER([readline/history.h],
                             [],
                             [AC_CHECK_HEADER([history.h])])])])
])# ES_HEADER_READLINE

# ES_READLINE
# -----------
# Check whether readline is available.
#
# This is a wrapper around ES_WITH_READLINE, ES_LIB_READLINE, and
# ES_HEADER_READLINE that defines HAVE_READLINE if a library is usable and the
# readline.h header is found.
#
# Defines HAVE_READLINE_H or HAVE_READLINE_READLINE_H, depending on whether
# <readline.h> or <readline/readline.h> should be included.
#
# If the library is available and a header is found, HAVE_HISTORY_H or
# HAVE_READLINE_HISTORY_H is also defined, depending on whether <history.h> or
# <readline/history.h> should be included.  If at least one of them is
# available, HAVE_READLINE_HISTORY is also defined.
#
# The relevant libraries are also added to LIBS for linking.
AC_DEFUN([ES_READLINE],
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
AC_REQUIRE([ES_WITH_READLINE])
AC_REQUIRE([ES_LIB_READLINE])
AC_REQUIRE([ES_HEADER_READLINE])
AC_CACHE_CHECK(
  [whether to enable readline],
  [_es_cv_rlenable],
  [AS_VAR_SET([_es_cv_rlenable], [no])
  AS_VAR_IF([_es_cv_rllib],
            [no], [],
            [AS_VAR_IF([_es_cv_rlheader],
                       [no], [],
                       [AS_VAR_SET([_es_cv_rlenable], [yes])])
            AS_VAR_IF([_es_cv_rlheader_bare],
                      [no], [],
                      [AS_VAR_SET([_es_cv_rlenable], [yes])])])])
AS_VAR_IF([_es_cv_rlenable],
  [no], [AS_VAR_IF([with_readline],
                   [yes], [AC_MSG_ERROR([cannot enable readline])])],
  [AS_VAR_IF([_es_cv_rlheader],
             [no], [],
             [AC_DEFINE([HAVE_READLINE_READLINE_H])])
  AS_VAR_IF([_es_cv_rlheader_bare],
            [no], [],
            [AC_DEFINE([HAVE_READLINE_H])])
  AS_VAR_IF([_es_cv_rlhistory],
            [no], [],
            [AC_DEFINE([HAVE_READLINE_HISTORY_H])
            AC_DEFINE([HAVE_READLINE_HISTORY])])
  AS_VAR_IF([_es_cv_rlhistory_bare],
            [no], [],
            [AC_DEFINE([HAVE_HISTORY_H])
            AC_DEFINE([HAVE_READLINE_HISTORY])])
  AS_VAR_APPEND([LIBS], [" $[]_es_cv_rllib[]"])
  AS_VAR_IF([_es_cv_rltermlib],
            ["none required"], [],
            [AS_VAR_APPEND([LIBS], [" $[]_es_cv_rltermlib[]"])])
  AC_DEFINE([HAVE_READLINE])])
])# ES_READLINE
