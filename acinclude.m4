AC_DEFUN([MY_EXPAND_DIR], [
	        $1=$2
	        $1=`(
	             test "x$prefix" = xNONE && prefix="$ac_default_prefix"
	             test "x$exec_prefix" = xNONE && exec_prefix="${prefix}"
	             eval echo \""[$]$1"/games/XBlast-TNT/\"
	            )`
	      ])

# From SANE (sane-project.org)
# XBLAST_CC_SET_CFLAGS()
# Set CFLAGS. Enable/disable compilation warnings if we gcc is used.
AC_DEFUN([XBLAST_CC_SET_CFLAGS],[
if test "${ac_cv_c_compiler_gnu}" = "yes"; then
  CFLAGS="$CFLAGS \
	-ggdb \
	-Wall \
	-Wcast-align \
	-Wcast-qual \
	-Wmissing-declarations \
	-Wmissing-prototypes \
	-Wpointer-arith \
	-Wreturn-type \
	-Wstrict-prototypes \
	"
fi # ac_cv_c_compiler_gnu
])
