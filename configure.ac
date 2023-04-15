dnl Process this file with autoconf to produce a configure script.
AC_INIT
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([access.c])

m4_pattern_forbid([^_?ES_])
m4_pattern_forbid([^_es_])

ES_WITH_READLINE
AC_ARG_ENABLE(
  [time],
  [AS_HELP_STRING([--enable-time], [enable 'time' builtin @<:@auto@:>@])]
)

AC_CANONICAL_HOST

AS_CASE(["$host"],
  [*sun5*|*solaris2*], [AC_DEFINE([SOLARIS], [1], [Are we Solaris?])]
)

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
  [AS_IF(
    [test -d /dev/fd], [AS_VAR_SET([es_cv_sys_dev_fd], [yes])],
    [AS_VAR_SET([es_cv_sys_dev_fd], [no])]
  )]
)
AS_VAR_IF([es_cv_sys_dev_fd],
  [yes], [AC_DEFINE([HAVE_DEV_FD], [1], [Do you have a /dev/fd/ directory?])])

AC_SYS_INTERPRETER
AS_VAR_IF([ac_cv_sys_interpreter],
  [yes], [AC_DEFINE([KERNEL_POUNDBANG], [1], [Does your kernel support @%:@!?])]
)

dnl Checks for libraries.
AC_SEARCH_LIBS([explicit_bzero], [bsd])
AS_VAR_IF([ac_cv_search_explicit_bzero],
  [no], [],
  [AC_DEFINE(
    [HAVE_EXPLICIT_BZERO],
    [1],
    [Define to 1 if you have explicit_bzero.]
  )]
)
ES_LIB_READLINE

dnl Checks for header files.
ES_HEADER_READLINE

dnl Checks for typedefs, structures, and compiler characteristics.
ES_STDC_ENABLE

# Check for _Bool
# If C99 or newer, we skip the check.  Otherwise, some compilers still make
# _Bool available even without C99, as seen with Clang 13.
AS_VAR_IF([es_cv_stdc_enable_c99],
  [no],
    [AH_TEMPLATE([HAVE__BOOL], [Define to 1 if you have the `_Bool' type.])
    AC_CACHE_CHECK(
      [for _Bool],
      [ac_cv_type__Bool],
      [AS_VAR_SET([ac_cv_type__Bool], [no])
      AC_COMPILE_IFELSE(
        [AC_LANG_SOURCE([[_Bool b = (_Bool)1;]])],
        [AS_VAR_SET([ac_cv_type__Bool], [yes])]
      )]
    )
    AS_VAR_IF([ac_cv_type__Bool],
      [yes], [AC_DEFINE([HAVE__BOOL])]
    )]
)

# Check for _Noreturn
# If C11 or newer, then we assume it is available.  Otherwise, we check for GCC
# __attribute__((noreturn)).
m4_define([_es_noreturn_attrib], [es_cv_c_attribute_noreturn])
AS_VAR_IF([es_cv_stdc_enable_c11],
  [no],
    [AH_TEMPLATE(
      [HAVE_NORETURN_ATTRIBUTE],
      [Define to 1 if you have the `noreturn' attribute.]
    )
    AC_CACHE_CHECK(
      [if the noreturn attribute is available],
      [_es_noreturn_attrib],
      [AS_VAR_SET([_es_noreturn_attrib], [no])
      AC_COMPILE_IFELSE(
        [AC_LANG_SOURCE([[__attribute__((noreturn)) void foo(void);]])],
        [AS_VAR_SET([_es_noreturn_attrib], [yes])]
      )]
    )
    AS_VAR_IF([_es_noreturn_attrib],
      [yes], [AC_DEFINE([HAVE_NORETURN_ATTRIBUTE])]
    )]
)

dnl Checks for library functions.
AC_CHECK_FUNCS([setrlimit])
AC_CHECK_FUNC(
  [mmap],
  [AC_CACHE_CHECK(
    [whether sys/mman.h defines MAP_PRIVATE],
    [ac_cv_ppdef_map_private],
    [AS_VAR_SET([ac_cv_ppdef_map_private], [no])
    AC_COMPILE_IFELSE(
      [AC_LANG_SOURCE(
        [[#include <sys/mman.h>
#ifndef MAP_PRIVATE
choke me
#endif  ]]
      )],
      [AS_VAR_SET([ac_cv_ppdef_map_private], [yes])]
    )]
  )
  AS_VAR_IF([ac_cv_ppdef_map_private],
    [no], [AC_MSG_FAILURE([MAP_PRIVATE must be defined])]
  )],
  [AC_MSG_FAILURE([mmap function with MAP_PRIVATE is required])]
)
AH_TEMPLATE([BUILTIN_TIME], [Define to 1 if you want the $&time primitive.])
AS_VAR_IF([enable_time],
  [no], [],
  [AC_CHECK_FUNC(
    [wait3],
    [AC_DEFINE([BUILTIN_TIME])],
    [AS_VAR_IF([enable_time],
      [yes], [AC_MSG_FAILURE([wait3 is required for 'time' builtin])]
    )]
  )]
)

# If C99 or newer, then we assume va_copy is available.  Otherwise, we test for
# __va_copy.
m4_define([_es_va_copy_alt], [ac_cv_macro___va_copy])
AS_VAR_IF([es_cv_stdc_enable_c99],
  [no],
    [AH_TEMPLATE(
      [HAVE___VA_COPY],
      [Define to 1 if you have the `__va_copy' macro.]
    )
    AC_CACHE_CHECK(
      [for __va_copy macro],
      [_es_va_copy_alt],
      [AS_VAR_SET([_es_va_copy_alt], [no])
      AC_COMPILE_IFELSE(
        [AC_LANG_SOURCE(
          [[#include <stdarg.h>
void foo(va_list ap) {
    va_list apsave;
    __va_copy(apsave, ap);
    va_end(apsave);
}         ]]
        )],
        [AS_VAR_SET([_es_va_copy_alt], [yes])]
      )]
    )
    AS_VAR_IF([_es_va_copy_alt],
      [yes], [AC_DEFINE([HAVE___VA_COPY])]
    )]
)

ES_READLINE
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
