/*
 * timeval.h    1.0 01/12/19
x *
 * Defines gettimeofday, timeval, etc. for Win32
 *
 * By Wu Yongwei
 *
 */

#ifndef _TIMEVAL_H
#define _TIMEVAL_H

#ifndef _WINSOCKAPI_
#ifndef _WINSOCK2API_

/* mingw32 libs (-mno-cygwin) */
/* gcc test */
#ifndef __GNUC__
#define EPOCHFILETIME (116444736000000000i64)
#else
#define EPOCHFILETIME (116444736000000000LL)
#endif

/* needed by cygwin,mingw wms */
#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <time.h>

/* mingw has timeval already defined */
#ifndef _TIMEVAL_DEFINED
struct timeval {
    long tv_sec;        /* seconds */
    long tv_usec;  /* microseconds */
};
#endif

struct timezone {
    int tz_minuteswest; /* minutes W of Greenwich */
    int tz_dsttime;     /* type of dst correction */
};
/* WMS doesnt allow inline export (6.0) */
#ifdef WMS
__inline int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    FILETIME        ft;
    LARGE_INTEGER   li;
    __int64         t;
    static int      tzflag;

    if (tv)
    {
        GetSystemTimeAsFileTime(&ft);
        li.LowPart  = ft.dwLowDateTime;
        li.HighPart = ft.dwHighDateTime;
        t  = li.QuadPart;       /* In 100-nanosecond intervals */
        t -= EPOCHFILETIME;     /* Offset to the Epoch time */
        t /= 10;                /* In microseconds */
        tv->tv_sec  = (long)(t / 1000000);
        tv->tv_usec = (long)(t % 1000000);
    }

    if (tz)
    {
        if (!tzflag)
        {
            _tzset();
            tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }

    return 0;
}
#else  /* __MINGW32__ ,cygwin */

#include <sys/time.h>
extern __inline int gettimeofday(struct timeval *tv, struct timezone *tz);
#endif 

#endif /* _WIN32 */

#include <sys/time.h>

#endif // winsock2

#endif // winsock

#endif /* _TIMEVAL_H */
