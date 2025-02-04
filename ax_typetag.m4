AC_ARG_WITH([typetag-libdir],
    [AS_HELP_STRING([--with-typetag-libdir=LIB_DIR],
    [Force given directory for libtypetag.])],
    [
        AS_IF([test -d "$withval"],
            [_AX_TYPETAG_libdir="$withval"],
            [AC_MSG_ERROR([--with-typetag-libdir expected directory name])])
    ],
    [_AX_TYPETAG_libdir=""]
)

AC_ARG_WITH([typetag-includedir],
    [AS_HELP_STRING([--with-typetag-includedir=INCLUDE_DIR],
    [Force given directory for libtypetag.])],
    [
        AS_IF([test -d "$withval"],
            [_AX_TYPETAG_includedir="$withval"],
            [AC_MSG_ERROR([--with-typetag-includedir expected directory name])])
    ],
    [_AX_TYPETAG_includedir=""]
)

AS_IF([test -n "$_AX_TYPETAG_includedir"],
    [AC_CHECK_HEADERS(
        [$_AX_TYPETAG_includedir/typetag/typetag.h], 
        [
            AC_DEFINE([HAVE_TYPETAG_H], [1], [found typetag.h])
            CFLAGS="$CFLAGS -I$_AX_TYPETAG_includedir"
        ],
        [AC_MSG_ERROR([typetag.h not found.])]
    )],
    [AC_CHECK_HEADERS(
        [typetag/typetag.h], 
        [AC_DEFINE([HAVE_TYPETAG_H], [1], [found typetag.h])],
        [AC_MSG_ERROR([typetag.h not found.])]
    )]
)

AS_IF([test -n "$_AX_TYPETAG_libdir"],
    [AC_CHECK_LIB([$_AX_TYPETAG_libdir/libtypetag.so], [tt_get_tag_type],
        [
            AC_DEFINE(HAVE_TYPETAG, [1], [Define if you have libtypetag])
            TYPETAG_LDFLAGS="-L$_AX_TYPETAG_libdir"
            TYPETAG_LIB="-ltypetag"
            AC_SUBST(TYPETAG_LDFLAGS)
            AC_SUBST(TYPETAG_LIB)
        ],
        [AC_MSG_ERROR([libtypetag not found.])]
    )],
    [AC_CHECK_LIB([typetag], [tt_get_tag_type],
        [
            AC_DEFINE(HAVE_TYPETAG, [1], [Define if you have libtypetag])
            TYPETAG_LDFLAGS=""
            TYPETAG_LIB="-ltypetag"
            AC_SUBST(TYPETAG_LDFLAGS)
            AC_SUBST(TYPETAG_LIB)
        ],
        [AC_MSG_ERROR([libtypetag not found.])]
    )]
)

AC_DEFINE([TYPE_TAGGING_ENABLED], [], ["Enable experimental type tagging"])