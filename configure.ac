AC_INIT([w_scan2], [1.0.1], [stefantalpalaru@yahoo.com], [w_scan2], [https://github.com/stefantalpalaru/w_scan2])
AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_PROG_CC

# define _GNU_SOURCE
AC_GNU_SOURCE

AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h strings.h sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([memset strcasecmp strdup strtoul])

dnl check for the presence of 'dvb/frontend.h'
AC_MSG_CHECKING([for linux/dvb/frontend.h presence])
    AC_LINK_IFELSE([
                    AC_LANG_PROGRAM([#include <linux/dvb/frontend.h>],
                                    [fe_caps_t fakeinstance;])
                    ],
                    [DVB_HEADER=1], [DVB_HEADER=0])

    if test "$DVB_HEADER" = "1"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
	echo "*******************************************************"
	echo "*** ERROR: $PACKAGE requires linux DVB headers,"
        echo "*** (usually /usr/include/linux/dvb)."
	echo "*** EXITING!"
	echo "*******************************************************"
        exit 1
    fi


dnl check for symbol 'dvb/frontend.h'
AC_MSG_CHECKING([for linux/dvb/frontend.h usability (FE_CAN_2G_MODULATION)])
    AC_LINK_IFELSE([
                    AC_LANG_PROGRAM([#include <linux/dvb/frontend.h>],
                                    [fe_caps_t fakeinstance = FE_CAN_2G_MODULATION;])
                    ],
                    [HAS_DVB_API5=1], [HAS_DVB_API5=0])

    if test "$HAS_DVB_API5" = "1"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
	echo "*******************************************************"
	echo "*** ERROR: $PACKAGE requires latest linux dvb headers."
	echo "*** missing symbol 'FE_CAN_2G_MODULATION' in <linux/dvb/frontend.h>"
	echo "*** Please update your linux dvb headers,"
        echo "*** (usually /usr/include/linux/dvb)."
	echo "*** EXITING!"
	echo "*******************************************************"
        exit 1
    fi

dnl check for the version of 'dvb/frontend.h'
AC_MSG_CHECKING([for linux/dvb/frontend.h usability (DVB-T2, DVB API >= v5.3)])
    AC_LINK_IFELSE([
                    AC_LANG_PROGRAM([#include <linux/dvb/frontend.h>],
                                    [fe_delivery_system_t fakeinstance = SYS_DVBT2;])
                    ],
                    [HAS_DVB_API5=1], [HAS_DVB_API5=0])

    if test "$HAS_DVB_API5" = "1"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
	echo "*******************************************************"
	echo "*** ERROR: $PACKAGE requires linux DVB API v5.3 or higher."
	echo "*** Please update your linux dvb headers,"
        echo "*** (usually /usr/include/linux/dvb)."
	echo "*** EXITING!"
	echo "*******************************************************"
        exit 1
    fi

AC_OUTPUT([Makefile])
