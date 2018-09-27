/*
 * file w32_mm.h - prototypes for winmm library
 *
 * $Id: w32_mm.h,v 1.2 2004/05/14 10:00:36 alfie Exp $
 *
 * Program XBLAST 
 * (C) by Oliver Vogel (e-mail: m.vogel@ndh.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2; or (at your option)
 * any later version
 *
 * This program is distributed in the hope that it will be entertaining,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILTY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef _W32_MM_H
#define _W32_MM_H

#include <windows.h>

#ifdef UNICODE
#define joyGetDevCaps joyGetDevCapsW
#else
#define joyGetDevCaps joyGetDevCapsA
#endif

#define MAXPNAMELEN 32

/*
 * type definitions
 */
DECLARE_HANDLE (HWAVE);
DECLARE_HANDLE (HWAVEOUT);

typedef UINT MMRESULT;

typedef struct {
  UINT wXpos;
  UINT wYpos;
  UINT wZpos;
  UINT wButtons;
} PACKED JOYINFO, *LPJOYINFO;

typedef struct tag_joycaps {
  UINT wMid;
  UINT wPid;
  char szPname[MAXPNAMELEN];
  UINT wXmin;
  UINT wXmax;
  UINT wYmin;
  UINT wYmax;
  UINT wZmin;
  UINT wZmax;
  UINT wNumButtons;
  UINT wPeriodMin;
  UINT wPeriodMax;
} PACKED JOYCAPS, *LPJOYCAPS;

typedef struct tag_waveformat {
  WORD  wFormatTag; 
  WORD  nChannels; 
  DWORD nSamplesPerSec;
  DWORD nAvgBytesPerSec;
  WORD  nBlockAlign;
} PACKED WAVEFORMAT, *LPWAVEFORMAT;

typedef struct { 
    WORD  wFormatTag; 
    WORD  nChannels; 
    DWORD nSamplesPerSec; 
    DWORD nAvgBytesPerSec; 
    WORD  nBlockAlign; 
    WORD  wBitsPerSample; 
    WORD  cbSize; 
} WAVEFORMATEX; 

typedef struct tag_pcmwaveformat {
  WAVEFORMAT wf;
  WORD       wBitsPerSample;
} PACKED PCMWAVEFORMAT, *LPPCMWAVEFORMAT;

typedef struct tag_wavehdr {
  LPSTR  lpData;
  DWORD  dwBufferLength;
  DWORD  dwBytesRecorded;
  DWORD  dwUser;
  DWORD  dwFlags;
  DWORD  dwLoops;
  struct tag_wavehdr *lpNext;
  DWORD  reserved;
} PACKED WAVEHDR;

/*
 * constants
 */
/* joystick ID */
#define JOYSTICKID1           0
#define JOYSTICKID2           1
/* error codes */   
#define JOYERR_NOERROR        0
#define JOYERR_PARMS        165
#define JOYERR_NOCANDO      166
#define JOYERR_UNPLUGGED    167
/* messages */
#define MM_JOY1MOVE       0x3A0
#define MM_JOY2MOVE       0x3A1
#define MM_JOY1ZMOVE      0x3A2
#define MM_JOY2ZMOVE      0x3A3
#define MM_JOY1BUTTONDOWN 0x3B5
#define MM_JOY2BUTTONDOWN 0x3B6
#define MM_JOY1BUTTONUP   0x3B7
#define MM_JOY2BUTTONUP   0x3B8
/* joystick button flags */
#define JOY_BUTTON1CHG    0x0100
#define JOY_BUTTON2CHG    0x0200
#define JOY_BUTTON3CHG    0x0400
#define JOY_BUTTON4CHG    0x0800

/* flags for dwFlags parameter in waveOutOpen() and waveInOpen() */
#define WAVE_FORMAT_QUERY 0x0001
#define WAVE_ALLOWSYNC    0x0002
#define CALLBACK_WINDOW   0x00010000l    /* dwCallback is a HWND */
#define CALLBACK_FUNCTION 0x00030000l    /* dwCallback is a FARPROC */
/* device ID for wave device mapper */
#define WAVE_MAPPER        (-1)
/* flags for wFormatTag field of WAVEFORMAT */
#define WAVE_FORMAT_PCM     1
/* messages */
#define MM_WOM_OPEN         0x3BB           /* waveform output */
#define MM_WOM_CLOSE        0x3BC
#define MM_WOM_DONE         0x3BD
/* flags for dwFlags field of WAVEHDR */
#define WHDR_DONE       0x00000001  /* done bit */
#define WHDR_PREPARED   0x00000002  /* set if this header has been prepared */
#define WHDR_BEGINLOOP  0x00000004  /* loop start block */
#define WHDR_ENDLOOP    0x00000008  /* loop end block */
#define WHDR_INQUEUE    0x00000010  /* reserved for driver */


/* KOEN SHIT */

#define	WINMMAPI 

#define TIMERR_NOERROR        (0)                  /* no error */
#define TIMERR_NOCANDO        (TIMERR_BASE+1)      /* request not completed */
#define TIMERR_STRUCT         (TIMERR_BASE+33)     /* time struct size */

/* timer data types */
typedef void (CALLBACK TIMECALLBACK)(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

typedef TIMECALLBACK FAR *LPTIMECALLBACK;

/* MMTIME data structure */
typedef struct mmtime_tag
{
    UINT            wType;      /* indicates the contents of the union */
    union
    {
	DWORD       ms;         /* milliseconds */
	DWORD       sample;     /* samples */
	DWORD       cb;         /* byte count */
	DWORD       ticks;      /* ticks in MIDI stream */

	/* SMPTE */
	struct
	{
	    BYTE    hour;       /* hours */
	    BYTE    min;        /* minutes */
	    BYTE    sec;        /* seconds */
	    BYTE    frame;      /* frames  */
	    BYTE    fps;        /* frames per second */
	    BYTE    dummy;      /* pad */
#ifdef _WIN32
	    BYTE    pad[2];
#endif
	} smpte;

	/* MIDI */
	struct
	{
	    DWORD songptrpos;   /* song pointer position */
	} midi;
    } u;
} MMTIME, *PMMTIME,  *NPMMTIME,  *LPMMTIME;

/* flags for fuEvent parameter of timeSetEvent() function */
#define TIME_ONESHOT    0x0000   /* program timer for single event */
#define TIME_PERIODIC   0x0001   /* program for continuous periodic event */

#ifdef _WIN32
#define TIME_CALLBACK_FUNCTION      0x0000  /* callback is function */
#define TIME_CALLBACK_EVENT_SET     0x0010  /* callback is event - use SetEvent */
#define TIME_CALLBACK_EVENT_PULSE   0x0020  /* callback is event - use PulseEvent */
#endif

/* timer device capabilities data structure */
typedef struct timecaps_tag {
    UINT    wPeriodMin;     /* minimum period supported  */
    UINT    wPeriodMax;     /* maximum period supported  */
} TIMECAPS, *PTIMECAPS,  *NPTIMECAPS,  *LPTIMECAPS;

/* timer function prototypes */
WINMMAPI MMRESULT WINAPI timeGetSystemTime(LPMMTIME pmmt, UINT cbmmt);
WINMMAPI DWORD WINAPI timeGetTime(void);
WINMMAPI MMRESULT WINAPI timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK fptc, DWORD dwUser, UINT fuEvent);
WINMMAPI MMRESULT WINAPI timeKillEvent(UINT uTimerID);
WINMMAPI MMRESULT WINAPI timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc);
WINMMAPI MMRESULT WINAPI timeBeginPeriod(UINT uPeriod);
WINMMAPI MMRESULT WINAPI timeEndPeriod(UINT uPeriod);



/*
 * function prototypes
 */
extern UINT STDCALL joyGetNumDevs (void);
extern MMRESULT STDCALL joyGetDevCaps (UINT, LPJOYCAPS, UINT);
extern MMRESULT STDCALL joyGetPos (UINT, LPJOYINFO);
extern MMRESULT STDCALL joySetCapture (HWND, UINT, UINT, BOOL);
extern MMRESULT STDCALL joySetThreshold (UINT, UINT);
extern MMRESULT STDCALL joyReleaseCapture(UINT uJoyID);

extern UINT STDCALL waveOutOpen (HWAVEOUT FAR* lphWaveOut, UINT uDeviceID, const WAVEFORMAT FAR* lpFormat, 
				 DWORD dwCallback, DWORD dwInstance, DWORD dwFlags);
extern UINT STDCALL waveOutClose (HWAVEOUT hWaveOut);
extern UINT STDCALL waveOutReset (HWAVEOUT hWaveOut);
extern UINT STDCALL waveOutPrepareHeader (HWAVEOUT hWaveOut, WAVEHDR FAR* lpWaveOutHdr, UINT uSize);
extern UINT STDCALL waveOutUnprepareHeader (HWAVEOUT hWaveOut, WAVEHDR FAR* lpWaveOutHdr, UINT uSize);
extern UINT STDCALL waveOutWrite (HWAVEOUT hWaveOut, WAVEHDR FAR* lpWaveOutHdr, UINT uSize);
#endif
/*
 * end of file w32_mm.h
 */
