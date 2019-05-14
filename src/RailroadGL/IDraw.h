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

#include "IDrawUnknown.h"
#include "IDrawPalette.h"
#include "IDrawClipper.h"
#include "IDrawSurface.h"

class IDraw : public IDrawUnknown
{
	// Inherited via IDirectDraw7
	virtual HRESULT __stdcall Compact();
	virtual HRESULT __stdcall CreateClipper(DWORD, IDrawClipper**, IDrawUnknown*);
	virtual HRESULT __stdcall CreatePalette(DWORD, LPPALETTEENTRY, IDrawPalette**, IDrawUnknown*);
	virtual HRESULT __stdcall CreateSurface(LPDDSURFACEDESC, IDrawSurface**, IDrawUnknown*);
	virtual HRESULT __stdcall DuplicateSurface(IDrawSurface*, IDrawSurface**);
	virtual HRESULT __stdcall EnumDisplayModes(DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMMODESCALLBACK);
	virtual HRESULT __stdcall EnumSurfaces(DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMSURFACESCALLBACK);
	virtual HRESULT __stdcall FlipToGDISurface();
	virtual HRESULT __stdcall GetCaps(LPDDCAPS, LPDDCAPS);
	virtual HRESULT __stdcall GetDisplayMode(LPDDSURFACEDESC);
	virtual HRESULT __stdcall GetFourCCCodes(LPDWORD, LPDWORD);
	virtual HRESULT __stdcall GetGDISurface(IDrawSurface**);
	virtual HRESULT __stdcall GetMonitorFrequency(LPDWORD);
	virtual HRESULT __stdcall GetScanLine(LPDWORD);
	virtual HRESULT __stdcall GetVerticalBlankStatus(LPBOOL);
	virtual HRESULT __stdcall Initialize(GUID*);
	virtual HRESULT __stdcall RestoreDisplayMode();
	virtual HRESULT __stdcall SetCooperativeLevel(HWND, DWORD);
	virtual HRESULT __stdcall SetDisplayMode(DWORD, DWORD, DWORD);
	virtual HRESULT __stdcall WaitForVerticalBlank(DWORD, HANDLE);
};

