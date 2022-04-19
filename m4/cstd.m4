# ES_STDC_xxx
# -----------
# Result: yyyymmL
#
# ES_STDC_Cxx expands to the value of __STDC_VERSION__ for the specified version
# of the C standard, making it usable in shell comparisons or with
# AS_VERSION_COMPARE:
#
#     # Fail if the default C standard is not C11 or newer.
#     ES_STDC_VERSION
#     AS_VERSION_COMPARE(["$es_cv_cpp_stdc_version"], [ES_STDC_C11],
#                        [AC_MSG_FAILURE([C11 is required])])
#
# The current supported versions of the standard are:
#
#   C89   198901L
#   C94   199409L
#   C99   199901L
#   C11   201112L
#   C17   201710L
#
# Note that while C89 did not actually define __STDC_VERSION__, it still defined
# __STDC__, distinguishing it from pre-standardization and non-standard
# compilers.  Its value 198901L was chosen arbitrarily.
dnl
dnl Please keep these chronologically ordered and update ES_STDC_NEW when you
dnl add a new version of the C standard.  It should be 1 more than the newly
dnl added version to forbid invalid declarations of __STDC_VERSION__.  Don't
dnl forget to update the list of C versions above.
AC_DEFUN([ES_STDC_C89], [198901L])# ES_STDC_C89
AC_DEFUN([ES_STDC_C94], [199409L])# ES_STDC_C94
AC_DEFUN([ES_STDC_C99], [199901L])# ES_STDC_C99
AC_DEFUN([ES_STDC_C11], [201112L])# ES_STDC_C11
AC_DEFUN([ES_STDC_C17], [201710L])# ES_STDC_C17
AC_DEFUN([ES_STDC_VERSIONS], [C89 C94 C99 C11 C17])# ES_STDC_VERSIONS

# ES_STDC_VERSION
# ---------------
# Result:
#   197001L       pre-C89
#   ES_STDC_Cnn   Cnn
#
# Cache the value of __STDC_VERSION__ reported by the preprocessor in
# es_cv_cpp_stdc_version.  If undefined, ac_cv_prog_cc_c89 is checked.  If it is
# set to 'no', then es_cv_cpp_stdc_version will be set to 197001L, else to
# ES_STDC_C89.
AC_DEFUN([ES_STDC_VERSION],
  [AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PROG_CPP])
  AC_REQUIRE([AC_PROG_EGREP])
  AS_VAR_PUSHDEF([_es_stdc_version], [es_cv_cpp_stdc_version])
  AC_CACHE_CHECK(
    [for __STDC_VERSION__],
    [_es_stdc_version],
    [AS_VAR_SET([_es_stdc_version], [no])
    AC_COMPILE_IFELSE(
      [AC_LANG_SOURCE(
        [[#ifdef __STDC_VERSION__
long v = __STDC_VERSION__;
#else
choke me
#endif  ]]
      )],
      [echo "__STDC_VERSION__" > conftest.$ac_ext
      AS_IF(
        [(eval "$ac_cpp conftest.$ac_ext") >conftest.i],
        [AS_VAR_SET(
          [_es_stdc_version],
          [[$($EGREP '^[[:blank:]]*[0-9]{6}L[[:blank:]]*$' conftest.i)]]
        )
        AS_VAR_IF([_es_stdc_version],
          [], [AS_VAR_SET([_es_stdc_version], [no])]
        )]
      )
      rm -f conftest.i conftest.$ac_ext],
      [AS_VAR_IF([ac_cv_prog_cc_c89],
        [no], [AS_VAR_SET([_es_stdc_version], [197001L])],
        [AS_VAR_SET([_es_stdc_version], [ES_STDC_C89])]
      )]
    )]
  )
  AS_VAR_POPDEF([_es_stdc_version])]
)# ES_STDC_VERSION

# _ES_STDC_ENABLE(VERSION)
# ------------------------
# Result: no/yes
#
# If VERSION is supported, cache 'yes' in the es_cv_stdc_enable_VERSION variable
# and define the ENABLE_<VERSION> preprocessor macro.
AC_DEFUN([_ES_STDC_ENABLE],
  [m4_ifblank(
    [$1],
    [m4_fatal([_ES_STDC_ENABLE: missing required argument VERSION])]
  )
  AH_TEMPLATE(
    [ENABLE_][$1],
    [Define to 1 if you are using a $1 compiler.]
  )
  AS_VAR_PUSHDEF([_es_stdc_enable], [m4_tolower([es_cv_stdc_enable_][$1])])
  AC_CACHE_VAL(
    [_es_stdc_enable],
    [AS_VAR_SET([_es_stdc_enable], [yes])
    AS_VERSION_COMPARE(["$es_cv_cpp_stdc_version"], m4_defn([ES_STDC_]$1),
      [AS_VAR_SET([_es_stdc_enable], [no])]
    )]
  )
  AS_VAR_IF([_es_stdc_enable],
    [yes], [AC_DEFINE([ENABLE_][$1])]
  )
  AS_VAR_POPDEF([_es_stdc_enable])]
)# _ES_STDC_ENABLE

# ES_STDC_ENABLE
# --------------
# Result: no/yes
#
# If a version of the C standard is supported, cache 'yes' in the
# es_cv_stdc_enable_VERSION variable and define the ENABLE_<VERSION>
# preprocessor macro.  If other versions are older than the given VERSION, they
# are also enabled in the same way.
#
#     ES_STDC_ENABLE
#     AS_VAR_IF([es_cv_std_enable_c99],
#       [yes], [AS_ECHO([C99 supported])]
#     )
#     AS_VAR_IF([es_cv_std_enable_c11],
#       [yes], [],
#       [AC_MSG_ERROR([a C11 compiler is required])]
#     )
AC_DEFUN([ES_STDC_ENABLE],
  [AC_REQUIRE([ES_STDC_VERSION])
  m4_map_args_w(ES_STDC_VERSIONS, [_ES_STDC_ENABLE(], [)])]
)# ES_STDC_ENABLE
