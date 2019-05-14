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
#include "IDrawSurface.h"
#include "ExtraTypes.h"
#include "OpenDrawClipper.h"
#include "OpenDrawPalette.h"

class OpenDraw;

class OpenDrawSurface : public IDrawSurface
{
public:
	OpenDraw* ddraw;
	DisplayMode mode;

	OpenDrawClipper* attachedClipper;
	OpenDrawPalette* attachedPalette;
	OpenDrawSurface* attachedSurface;

	VOID* indexBuffer;

	SurfaceType type;

	OpenDrawSurface(IDrawUnknown**, OpenDraw*, SurfaceType);
	~OpenDrawSurface();

	VOID CreateBuffer(DWORD, DWORD, DWORD);
	VOID ReleaseBuffer();

	// Inherited via IDrawSurface
	ULONG __stdcall Release() override;
	HRESULT __stdcall Blt(LPRECT, IDrawSurface*, LPRECT, DWORD, LPDDBLTFX) override;
	HRESULT __stdcall GetPixelFormat(LPDDPIXELFORMAT) override;
	HRESULT __stdcall GetSurfaceDesc(LPDDSURFACEDESC) override;
	HRESULT __stdcall Lock(LPRECT, LPDDSURFACEDESC, DWORD, HANDLE) override;
	HRESULT __stdcall SetClipper(IDrawClipper*) override;
	HRESULT __stdcall SetPalette(IDrawPalette*) override;
};

