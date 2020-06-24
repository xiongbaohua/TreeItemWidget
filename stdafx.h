#ifndef AFX_STDAFX_H__INCLUDED_
#define AFX_STDAFX_H__INCLUDED_

#define SMALL_DOUBLE	0.0000001
#define EQUALDOUBLE(x,y)	(fabs((x)-(y)) < SMALL_DOUBLE)
#define GREATERDOUBLE(x,y)	(((x)-(y)) >SMALL_DOUBLE)
#define LESSDOUBLE(x,y)	(((x)-(y)) <-SMALL_DOUBLE)

#include <typeinfo>
#include <string.h>
#include <math.h>
#include <type_traits>
#include <qglobal.h>
#include "CodecClass.h"
#include <qsound.h>

#define STRING(a)	GBKCodec::fromGBK8Bit(a)
#define	BYTEARRAY(a)	GBKCodec::toGBK8Bit(a)

#define TRUE 1
#define FALSE 0
typedef char				CHAR;
typedef const CHAR			*LPCSTR;
typedef CHAR				*LPSTR;
typedef int					BOOL;
typedef unsigned int		uint;
typedef int					INT;
typedef unsigned int		UINT;
typedef unsigned int		*PUINT;
typedef long				LONG;
typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef FLOAT               *LPFLOAT;
typedef BOOL				*LPBOOL;
typedef int					INT;
typedef BYTE		        *LPBYTE;
typedef int		            *LPINT;
typedef WORD		        *LPWORD;
typedef long	            *LPLONG;
typedef DWORD		        *LPDWORD;
typedef void	            *LPVOID;
typedef const void			*LPCVOID;
typedef DWORD				COLORREF;
typedef DWORD				*LPCOLORREF;
typedef unsigned short		VARTYPE;
typedef qlonglong			LLONG;
typedef LLONG				LPARAM; //64bits, can used to as pointer addr and int64.
typedef LLONG				WPARAM;	//64bits, can used to as pointer addr and int64.
typedef LLONG				LRESULT;
typedef LLONG				INT_PTR;	//64bits, can used to as pointer addr and int64.
#define RGBA(r,g,b,a)		(COLORREF)(((DWORD)(BYTE)(a))<<24|((DWORD)(BYTE)(r))<<16|((DWORD)(BYTE)(g))<<8|(DWORD)(BYTE)b)
#define ARGB(a,r,g,b)		(COLORREF)(((DWORD)(BYTE)(a))<<24|((DWORD)(BYTE)(r))<<16|((DWORD)(BYTE)(g))<<8|(DWORD)(BYTE)b)
#define RGB(r,g,b)			(COLORREF)(((DWORD)255)<<24|((DWORD)(BYTE)(r))<<16|((DWORD)(BYTE)(g))<<8|((DWORD)(BYTE)(b)))

#endif // !defined(AFX_STDAFX_H__INCLUDED_)
