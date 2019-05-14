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
#include "ExtraTypes.h"

#define FPS_X 3
#define FPS_Y 5
#define FPS_WIDTH 16
#define FPS_HEIGHT 24
#define FPS_COUNT 120
#define FPS_ACCURACY 2000

extern FpsState fpsState;
extern BOOL isFpsChanged;

extern const WORD counters[10][FPS_HEIGHT];

struct FrameItem
{
	DWORD tick;
	DWORD span;
};

class FpsCounter
{
private:
	DWORD accuracy;
	DWORD count;
	DWORD checkIndex;
	DWORD currentIndex;
	DWORD summary;
	DWORD lastTick;
	FrameItem* tickQueue;

public:
	DWORD value;

	VOID* operator new(size_t);
	VOID operator delete(VOID*);

	FpsCounter(DWORD accuracy);
	~FpsCounter();

	VOID Reset();
	VOID Calculate();
	VOID Draw(VOID*, VOID*, DWORD, DWORD);
};