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

#include "stdafx.h"
#include "OpenDrawSurface.h"
#include "OpenDraw.h"
#include "Main.h"
#include "Config.h"
#include "Resource.h"

OpenDrawSurface::OpenDrawSurface(IDrawUnknown** list, OpenDraw* lpDD, SurfaceType type)
	: IDrawSurface(list)
{
	this->ddraw = lpDD;

	this->attachedPalette = NULL;
	this->attachedClipper = NULL;
	this->attachedSurface = NULL;

	this->indexBuffer = NULL;

	this->mode.width = 0;
	this->mode.height = 0;
	this->mode.bpp = 0;

	this->type = type;
}

OpenDrawSurface::~OpenDrawSurface()
{
	if (this->ddraw->attachedSurface == this)
		this->ddraw->attachedSurface = NULL;

	if (this->attachedPalette)
		this->attachedPalette->Release();

	if (this->attachedClipper)
		this->attachedClipper->Release();

	if (this->attachedSurface)
		this->attachedSurface->Release();

	this->ReleaseBuffer();
}

VOID OpenDrawSurface::ReleaseBuffer()
{
	if (this->indexBuffer)
	{
		AlignedFree(this->indexBuffer);
		this->indexBuffer = NULL;
	}
}

VOID OpenDrawSurface::CreateBuffer(DWORD width, DWORD height, DWORD bpp)
{
	this->ReleaseBuffer();

	this->mode.width = width;
	this->mode.height = height;
	this->mode.bpp = bpp;

	bpp >>= 3;
	if (bpp == sizeof(BYTE) && this->type == SurfacePrimary)
		bpp = sizeof(DWORD);

	DWORD size = width * height * bpp;
	this->indexBuffer = AlignedAlloc(size);
	MemoryZero(this->indexBuffer, size);
}

ULONG __stdcall OpenDrawSurface::Release()
{
	if (this->type == SurfacePrimary)
		this->ddraw->RenderStop();

	if (--this->refCount)
		return this->refCount;

	delete this;
	return 0;
}

HRESULT __stdcall OpenDrawSurface::GetPixelFormat(LPDDPIXELFORMAT lpDDPixelFormat)
{
	lpDDPixelFormat->dwRGBBitCount = this->mode.bpp;

	if (this->mode.bpp == 8)
		lpDDPixelFormat->dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;
	else
	{
		lpDDPixelFormat->dwFlags = DDPF_RGB;
		lpDDPixelFormat->dwRBitMask = 0xF800;
		lpDDPixelFormat->dwGBitMask = 0x07E0;
		lpDDPixelFormat->dwBBitMask = 0x001F;
		lpDDPixelFormat->dwRGBAlphaBitMask = 0x0000;
	}

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::GetSurfaceDesc(LPDDSURFACEDESC lpDDSurfaceDesc)
{
	lpDDSurfaceDesc->dwWidth = this->mode.width;
	lpDDSurfaceDesc->dwHeight = this->mode.height;
	lpDDSurfaceDesc->lPitch = this->mode.width * (this->mode.bpp >> 3);
	lpDDSurfaceDesc->lpSurface = this->indexBuffer;

	this->GetPixelFormat(&lpDDSurfaceDesc->ddpfPixelFormat);

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::Lock(LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent)
{
	this->GetSurfaceDesc(lpDDSurfaceDesc);
	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::SetClipper(IDrawClipper* lpDDClipper)
{
	OpenDrawClipper* old = this->attachedClipper;
	this->attachedClipper = (OpenDrawClipper*)lpDDClipper;

	if (this->attachedClipper)
	{
		if (old != this->attachedClipper)
		{
			if (old)
				old->Release();

			this->attachedClipper->AddRef();
		}
	}
	else if (old)
		old->Release();

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::SetPalette(IDrawPalette* lpDDPalette)
{
	OpenDrawPalette* old = this->attachedPalette;
	this->attachedPalette = (OpenDrawPalette*)lpDDPalette;

	if (this->attachedPalette)
	{
		if (old != this->attachedPalette)
		{
			if (old)
				old->Release();

			this->attachedPalette->AddRef();
		}
	}
	else if (old)
		old->Release();

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::Blt(LPRECT lpDestRect, IDrawSurface* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
	OpenDrawSurface* surface = (OpenDrawSurface*)lpDDSrcSurface;

	DWORD sWidth = surface->mode.width;

	DWORD dWidth;
	if (this->attachedClipper)
	{
		POINT offset = { 0, 0 };
		ClientToScreen(this->attachedClipper->hWnd, &offset);
		OffsetRect(lpDestRect, -offset.x, -offset.y);

		dWidth = config.mode->width;
	}
	else
		dWidth = this->mode.width;

	if (lpDestRect->left < 0)
	{
		lpSrcRect->left -= lpDestRect->left;
		lpDestRect->left = 0;
	}

	if (lpDestRect->top < 0)
	{
		lpSrcRect->top -= lpDestRect->top;
		lpDestRect->top = 0;
	}

	if (lpDestRect->right > this->mode.width)
		lpDestRect->right = this->mode.width;

	if (lpDestRect->bottom > this->mode.height)
		lpDestRect->bottom = this->mode.height;

	INT width = lpDestRect->right - lpDestRect->left;
	INT height = lpDestRect->bottom - lpDestRect->top;

	if (width > 0 && height > 0)
	{
		if (this->mode.bpp == 16)
		{
			WORD* source = (WORD*)surface->indexBuffer + lpSrcRect->top * sWidth + lpSrcRect->left;
			WORD* destination = (WORD*)this->indexBuffer + lpDestRect->top * dWidth + lpDestRect->left;

			if (width == this->mode.width && width == surface->mode.width)
				MemoryCopy(destination, source, width * height * sizeof(WORD));
			else
				do
				{
					__asm
					{
						MOV EDI, destination
						MOV ESI, source
						MOV ECX, width
						REP MOVSW
					}

					source += sWidth;
					destination += dWidth;
				} while (--height);
		}
		else if (this->attachedPalette)
		{
			BYTE* source = (BYTE*)surface->indexBuffer + lpSrcRect->top * sWidth + lpSrcRect->left;
			DWORD* destination = (DWORD*)this->indexBuffer + lpDestRect->top * dWidth + lpDestRect->left;

			do
			{
				BYTE* src = source;
				DWORD* dest = destination;

				DWORD count = width;
				do
					*dest++ = this->attachedPalette->entries[*src++];
				while (--count);

				source += sWidth;
				destination += dWidth;
			} while (--height);
		}

		SetEvent(this->ddraw->hDrawEvent);
		Sleep(0);
	}

	return DD_OK;
}