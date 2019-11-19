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
#include "IDraw.h"
#include "ExtraTypes.h"
#include "OpenDrawSurface.h"
#include "OpenDrawClipper.h"
#include "OpenDrawPalette.h"

class OpenDraw : public IDraw {
public:
	OpenDrawSurface* surfaceEntries;
	OpenDrawClipper* clipperEntries;
	OpenDrawPalette* paletteEntries;

	OpenDrawSurface* attachedSurface;

	HDC hDc;
	HWND hWnd;
	HWND hDraw;

	HANDLE hDrawThread;
	HANDLE hDrawEvent;

	Viewport viewport;
	DWORD clearStage;
	BOOL isFinish;
	BOOL isStateChanged;
	BOOL isTakeSnapshot;

	OpenDraw(IDrawUnknown**);
	~OpenDraw();

	VOID CalcView();
	BOOL CheckView(BOOL);
	VOID __fastcall ScaleMouse(LPPOINT);

	VOID RenderStart();
	VOID RenderStop();

	VOID RenderOld();
	VOID RenderNew();
	VOID TakeSnapshot(VOID*, Size*);

	// Inherited via  IDraw
	ULONG __stdcall Release() override;
	HRESULT __stdcall CreateClipper(DWORD, IDrawClipper**, IDrawUnknown*) override;
	HRESULT __stdcall CreateSurface(LPDDSURFACEDESC, IDrawSurface**, IDrawUnknown*) override;
	HRESULT __stdcall SetCooperativeLevel(HWND, DWORD) override;
	HRESULT __stdcall SetDisplayMode(DWORD, DWORD, DWORD) override;
	HRESULT __stdcall EnumDisplayModes(DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMMODESCALLBACK) override;
	HRESULT __stdcall GetDisplayMode(LPDDSURFACEDESC) override;
	HRESULT __stdcall CreatePalette(DWORD, LPPALETTEENTRY, IDrawPalette**, IDrawUnknown*) override;
};