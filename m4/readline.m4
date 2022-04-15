dnl libraries providing a readline function
m4_define([_es_rllibs], [readline edit editline])
dnl terminal capability libraries that may be needed to link readline
m4_define([_es_rltermlibs], [terminfo tinfo ncurses curses termcap])

dnl cache variables
m4_define([_es_cv_rlheader_bare],   [ac_cv_header_readline_h])
m4_define([_es_cv_rlheader],        [ac_cv_header_readline_readline_h])
m4_define([_es_cv_rlhistory_bare],  [ac_cv_header_history_h])
m4_define([_es_cv_rlhistory],       [ac_cv_header_readline_history_h])
m4_define([_es_cv_rllib],           [ac_cv_search_readline])
m4_define([_es_cv_rltermlib],       [ac_cv_search_readline_termlib])

dnl shell variables we use to simplify the logic in _ES_RLCHECK
m4_define([_es_rlhave_header],  [es_readline_have_header])
m4_define([_es_rlhave_lib],     [es_readline_have_lib])

# _ES_RLFAIL
# ----------
# Fail if --with-readline=yes and something is missing.
AC_DEFUN([_ES_RLFAIL], [AC_MSG_FAILURE([cannot enable readline])])# _ES_RLFAIL

# _ES_RLCHECK
# -----------
# If --with-readline={check|auto}, ensure $[]_es_rlhave_header and
# $[]_es_rlhave_lib match when both of them are either yes or no.  If they do
# not match, a warning is printed and readline is disabled.
AC_DEFUN([_ES_RLCHECK],
  [AS_VAR_IF([_es_rlhave_header],
    [no],
      [AS_VAR_IF([_es_rlhave_lib],
        [yes],
          [AC_MSG_WARN([readline library found, but readline.h missing])
          AS_VAR_IF([with_readline],
            [yes], [_ES_RLFAIL],
            [AS_VAR_SET([with_readline], [no])]
          )]
      )]
  )
  AS_VAR_IF([_es_rlhave_lib],
    [no],
      [AS_VAR_IF([_es_rlhave_header],
        [yes],
          [AC_MSG_WARN([readline.h found, but readline library missing])
          AS_VAR_IF([with_readline],
            [yes], [_ES_RLFAIL],
            [AS_VAR_SET([with_readline], [no])]
          )]
      )]
  )
  AS_VAR_IF([with_readline],
    [no], [AC_MSG_NOTICE([readline functionality will be disabled])]
  )]
)# _ES_RLCHECK


# ES_WITH_READLINE
# ----------------
# Add --with-readline option
#
# The default is 'auto' to automatically detect whether it is available or not.
# 'check' is also accepted as an equivalent alternative to 'auto'.
AC_DEFUN([ES_WITH_READLINE],
  [AC_ARG_WITH(
    [readline],
    [AS_HELP_STRING(
      [--with-readline],
      [enable line editing with readline @<:@auto@:>@]
    )],
    [],
    [AS_VAR_SET([with_readline], [auto])]
  )
  dnl TODO - should --with-readline=READLINE_PREFIX be supported?
  AS_CASE(["$with_readline"],
    [no|check|auto|yes], [],
    [AC_MSG_ERROR([invalid argument to --with-readline: $with_readline])]
  )
  AS_VAR_SET([_es_rlhave_header],  [])
  AS_VAR_SET([_es_rlhave_lib],     [])]
)# ES_WITH_READLINE

# _ES_LIB_READLINE_LIBRARY(LIBRARY, [TERMCAP-LIBRARY])
# ----------------------------------------------------
# Check whether readline is found in LIBRARY, caching the result in
# _es_cv_rllib.
#
# If TERMCAP-LIBRARY is not provided, "none required" is cached in
# _es_cv_rltermlib on success.
AC_DEFUN([_ES_LIB_READLINE_LIBRARY],
  [AS_VAR_PUSHDEF([_es_save], [es_save_LIBS])
  AS_VAR_COPY([_es_save], [LIBS])
  AS_VAR_APPEND([LIBS], [" -l[]$1[]"])
  m4_ifnblank($2, [AS_VAR_APPEND([LIBS], [" -l[]$2[]"])])
  AC_LINK_IFELSE(
    [AC_LANG_CALL([], [readline])],
    [AS_VAR_SET([_es_cv_rllib], [-l[]$1])
    AC_CACHE_VAL(
      [_es_cv_rltermlib],
      [AS_VAR_SET(
        [_es_cv_rltermlib],
        m4_ifblank($2, ["none required"], [-l[]$2])
      )]
    )],
    [AS_VAR_SET([_es_cv_rllib], [no])]
  )
  AS_VAR_COPY([LIBS], [_es_save])
  AS_VAR_POPDEF([_es_save])]
)# _ES_LIB_READLINE_LIBRARY

# _ES_LIB_READLINE([IF-FOUND], [IF-NOT-FOUND])
# --------------------------------------------
# Helper for ES_LIB_READLINE
AC_DEFUN([_ES_LIB_READLINE],
  [AC_CACHE_CHECK(
    [for library containing readline],
    [_es_cv_rllib],
    [AS_VAR_SET([_es_cv_rllib], [no])
    AS_FOR(
      [_es_rllib],
      [es_rllib],
      _es_rllibs,
      [_ES_LIB_READLINE_LIBRARY([_es_rllib])
      AS_VAR_IF([_es_cv_rllib],
        [no],
          [AS_FOR(
            [_es_rltermlib],
            [es_rltermlib],
            _es_rltermlibs,
            [_ES_LIB_READLINE_LIBRARY([_es_rllib], [_es_rltermlib])
            AS_VAR_IF([_es_cv_rllib],
              [no], [],
              [break] # readline lib found, break out of termlib loop
            )]
          )
          AS_VAR_IF([_es_cv_rllib],
            [no], [],
            [break] # readline lib found, break out of readline lib loop
          )],
        [break] # readline lib found without termlib, break out of loop
      )]
    )]
  )
  AS_VAR_IF([_es_cv_rllib],
    [no], [$2],
    [$1]
  )]
)# _ES_LIB_READLINE

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
    [no], [],
    [_ES_LIB_READLINE(
      [AS_VAR_SET([_es_rlhave_lib], [yes])],
      [AS_VAR_SET([_es_rlhave_lib], [no])]
    )
    _ES_RLCHECK]
  )]
)# ES_LIB_READLINE

# _ES_HEADER_READLINE(HEADER, [IF-FOUND], [IF-NOT-FOUND])
# -------------------------------------------------------
# Use AC_CHECK_HEADER to determine how to include the specified header.
# Run IF-FOUND if the header is found, else IF-NOT-FOUND.
AC_DEFUN([_ES_HEADER_READLINE],
  [AC_CHECK_HEADER(
    [readline/$1],
    [$2],
    [AC_CHECK_HEADER([$1], [$2], [$3])]
  )]
)# _ES_HEADER_READLINE


# ES_HEADER_READLINE
# ------------------
# Use AC_CHECK_HEADER to determine how to include readline.h and history.h
#
# This does not define any HAVE_xxx preprocessor macros.
AC_DEFUN([ES_HEADER_READLINE],
  [AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([ES_WITH_READLINE])
  AS_VAR_IF([with_readline],
    [no], [],
    [_ES_HEADER_READLINE(
      [readline.h],
      [AS_VAR_SET([_es_rlhave_header], [yes])],
      [AS_VAR_SET([_es_rlhave_header], [no])]
    )
    _ES_RLCHECK
    _ES_HEADER_READLINE([history.h])]
  )]
)# ES_HEADER_READLINE

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
  [AH_TEMPLATE(
    [HAVE_READLINE_READLINE_H],
    [Define to 1 if you have <readline/readline.h>.]
  )
  AH_TEMPLATE([HAVE_READLINE_H], [Define to 1 if you have <readline.h>.])
  AH_TEMPLATE(
    [HAVE_READLINE],
    [Define to 1 if you have readline functionality.]
  )
  AH_TEMPLATE(
    [HAVE_READLINE_HISTORY_H],
    [Define to 1 if you have <readline/history.h>.]
  )
  AH_TEMPLATE([HAVE_HISTORY_H], [Define to 1 if you have <history.h>.])
  AH_TEMPLATE(
    [HAVE_READLINE_HISTORY],
    [Define to 1 if readline history features are available.]
  )
  AC_REQUIRE([ES_WITH_READLINE])
  AC_REQUIRE([ES_LIB_READLINE])
  AC_REQUIRE([ES_HEADER_READLINE])
  AS_VAR_IF([with_readline],
    [no], [],
    [AS_VAR_IF([_es_cv_rlheader],
      [no], [AC_DEFINE([HAVE_READLINE_H])],
      [AC_DEFINE([HAVE_READLINE_READLINE_H])]
    )
    AS_VAR_IF([_es_cv_rlhistory],
      [no],
        [AS_VAR_IF([_es_cv_rlhistory_bare],
          [no], [],
          [AC_DEFINE([HAVE_HISTORY_H])
          AC_DEFINE([HAVE_READLINE_HISTORY])]
        )],
      [AC_DEFINE([HAVE_READLINE_HISTORY_H])
      AC_DEFINE([HAVE_READLINE_HISTORY])]
    )
    AS_VAR_APPEND([LIBS], [" $[]_es_cv_rllib[]"])
    AS_VAR_IF([_es_cv_rltermlib],
      ["none required"], [],
      [AS_VAR_APPEND([LIBS], [" $[]_es_cv_rltermlib[]"])]
    )
    AC_DEFINE([HAVE_READLINE])]
  )]
)# ES_READLINE
