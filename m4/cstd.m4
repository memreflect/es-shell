# ES_STDC_xxx
# -----------
# ES_STDC_Cxx expands to the value of __STDC_VERSION__ for the specified version
# of the C standard in the format `yyyymm', making it usable in shell
# comparisons or with AS_VERSION_COMPARE:
#
#     # Fail if the default C standard is not C11 or newer.
#     ES_STDC_VERSION
#     AS_VERSION_COMPARE([$es_stdc_version], [ES_STDC_C11],
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
#
# Two special macros, ES_STDC_OLD and ES_STDC_NEW, exist as fallbacks:
#
# - ES_STDC_OLD is used in the event __STDC_VERSION__ and __STDC__ are both
#   undefined.  It will always be less the value of any other ES_STDC_xxx macro.
#   This should only happen on pre-standard compilers, hence the "old" moniker.
#
# - ES_STDC_NEW is used in the event that a compiler reports a newer version of
#   the ISO C standard than what is currently recognized as determined by the
#   ES_STDC_VERSION macro and will always be greater than the value
#   corresponding to the latest C standard listed above.
#
# Because these are merely fallbacks, you should not rely on them having
# particular values.  They should only be used in comparisons with the shell
# variable `es_stdc_version' that gets set by the ES_STDC_VERSION macro.
dnl
dnl Please keep these chronologically ordered and update ES_STDC_NEW when you
dnl add a new version of the C standard.  It should be 1 more than the newly
dnl added version to forbid invalid declarations of __STDC_VERSION__.  Don't
dnl forget to update the list of C versions above.
AC_DEFUN([ES_STDC_OLD], [197001L])# ES_STDC_OLD
AC_DEFUN([ES_STDC_C89], [198901L])# ES_STDC_C89
AC_DEFUN([ES_STDC_C94], [199409L])# ES_STDC_C94
AC_DEFUN([ES_STDC_C99], [199901L])# ES_STDC_C99
AC_DEFUN([ES_STDC_C11], [201112L])# ES_STDC_C11
AC_DEFUN([ES_STDC_C17], [201710L])# ES_STDC_C17
AC_DEFUN([ES_STDC_NEW], [201711L])# ES_STDC_NEW

# ES_STDC_VERSION
# ---------------
# Determine the minimum level of recognized compiler support.  Sets the cache
# variable `es_cv_cpp_stdc_version' to the value of __STDC_VERSION__ reported by
# the preprocessor, to ES_STDC_C89 if a C89 compiler is detected, or to `no' if
# __STDC_VERSION__ is undefined and the compiler is not a C89 compiler.
#
# The shell variable `es_stdc_version' is set to the value of the corresponding
# ES_STDC_xxx macro.
AC_DEFUN([ES_STDC_VERSION],
[AC_REQUIRE([AC_PROG_CC])
AC_REQUIRE([AC_PROG_CPP])
AC_REQUIRE([AC_PROG_EGREP])
m4_pushdef([_es_cppname], [es_cv_cpp_stdc_version])
AC_CACHE_CHECK(
  [for __STDC_VERSION__],
  [_es_cppname],
  [AS_VAR_SET([_es_cppname], [no])
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#ifdef __STDC_VERSION__
long v = __STDC_VERSION__;
#else
choke me
#endif
]])],
                    [echo "__STDC_VERSION__" > conftest.$ac_ext
                    AS_IF([(eval "$ac_cpp conftest.$ac_ext") >conftest.i],
                          [AS_VAR_SET([_es_cppname],
                                      [[$($EGREP '^[[:blank:]]*[0-9]{6}L[[:blank:]]*$' conftest.i)]])
                          AS_VAR_IF([_es_cppname],
                                    [], [AS_VAR_SET([_es_cppname], [no])])])
                    rm -f conftest.i conftest.$ac_ext],
                    [AS_VAR_IF([ac_cv_prog_cc_c89],
                               [no], [],
                               [AS_VAR_SET([_es_cppname], [ES_STDC_C89])])])])
m4_pushdef([_es_stdc], [es_stdc_version])
AS_CASE(["$[]_es_cppname[]"],
  [no],          [AS_VAR_SET([_es_stdc], [ES_STDC_OLD])],
  [ES_STDC_C89], [AS_VAR_SET([_es_stdc], [ES_STDC_C89])],
  [ES_STDC_C94], [AS_VAR_SET([_es_stdc], [ES_STDC_C94])],
  [ES_STDC_C99], [AS_VAR_SET([_es_stdc], [ES_STDC_C99])],
  [ES_STDC_C11], [AS_VAR_SET([_es_stdc], [ES_STDC_C11])],
  [ES_STDC_C17], [AS_VAR_SET([_es_stdc], [ES_STDC_C17])])
dnl Detect non-standard declarations of __STDC_VERSION__ before setting
dnl 'es_stdc_version' to ES_STDC_NEW.
AS_VAR_SET_IF([_es_stdc],
  [],
  [AS_VERSION_COMPARE(["$[]_es_cppname[]"], [ES_STDC_NEW],
                      [AC_MSG_ERROR([unexpected ISO C version -- $[]_es_cppname])])
  AS_VAR_SET([_es_stdc], [ES_STDC_NEW])])
m4_popdef([_es_stdc])
m4_popdef([_es_cppname])
])# ES_STDC_VERSION
