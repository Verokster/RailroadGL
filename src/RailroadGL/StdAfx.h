/*
	MIT License

	Copyright (c) 2019 Oleksiy Ryabchun

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#pragma once
#define WIN32_LEAN_AND_MEAN
//#define WINVER 0x0400

#include "windows.h"
#include "mmreg.h"
#include "math.h"
#include "shellscalingapi.h"
#include "ddraw.h"
#include "ExtraTypes.h"

#define WC_DRAW "7903f211-51ca-4a51-9ec5-e1301db2d24d"
#define WS_WINDOWED_DDRAW (WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS)
#define WS_WINDOWED_GDI (WS_CLIPSIBLINGS | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
#define WS_FULLSCREEN (WS_POPUP | WS_SYSMENU | WS_CLIPSIBLINGS | WS_MAXIMIZE)

typedef HANDLE(__stdcall *CREATEACTCTXA)(ACTCTX* pActCtx);
typedef VOID(__stdcall *RELEASEACTCTX)(HANDLE hActCtx);
typedef BOOL(__stdcall *ACTIVATEACTCTX)(HANDLE hActCtx, ULONG_PTR* lpCookie);
typedef BOOL(__stdcall *DEACTIVATEACTCTX)(DWORD dwFlags, ULONG_PTR ulCookie);

extern CREATEACTCTXA CreateActCtxC;
extern RELEASEACTCTX ReleaseActCtxC;
extern ACTIVATEACTCTX ActivateActCtxC;
extern DEACTIVATEACTCTX DeactivateActCtxC;

typedef HRESULT(__stdcall* SETPROCESSDPIAWARENESS)(PROCESS_DPI_AWARENESS);

extern SETPROCESSDPIAWARENESS SetProcessDpiAwarenessC;

extern "C" _CRTIMP int __cdecl sprintf(char*, const char*, ...);

#define MemoryAlloc(size) malloc(size)
#define MemoryFree(block) free(block)
#define MemorySet(dst,val,size) memset(dst,val,size)
#define MemoryZero(dst,size) memset(dst,0,size)
#define MemoryCopy(dst,src,size) memcpy(dst,src,size)
#define MathCeil(x) ceil(x)
#define MathFloor(x) floor(x)
#define StrPrint(buf,fmt,...) sprintf(buf,fmt,__VA_ARGS__)
#define StrCompare(str1,str2) strcmp(str1,str2)
#define StrCompareInsensitive(str1,str2) _stricmp(str1,str2)
#define StrCopy(dst,src) strcpy(dst,src)
#define StrCat(dst,src) strcat(dst,src)
#define StrLastChar(str,ch) strrchr(str,ch)
#define StrStr(str,substr) strstr(str,substr)
#define StrToAnsi(dst,src,size) wcstombs(dst,src,size)
#define Exit(code) exit(code)

DOUBLE __fastcall MathRound(DOUBLE);
VOID* __fastcall AlignedAlloc(size_t size);
VOID __fastcall AlignedFree(VOID* block);

extern HMODULE hDllModule;
extern HANDLE hActCtx;

VOID LoadKernel32();
VOID LoadWSock32();
VOID LoadShcore();