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
#include "FpsCounter.h"
#include "Config.h"

FpsState fpsState;
BOOL isFpsChanged;

const WORD counters[10][FPS_HEIGHT] = {
	{ // 0
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC
	}, { // 1
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000
	}, { // 2
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0x003C,
		0x003C,
		0x003C,
		0x003C,
		0x003C,
		0x003C,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC
	}, { // 3
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC
	}, { // 4
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000
	}, { // 5
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0x003C,
		0x003C,
		0x003C,
		0x003C,
		0x003C,
		0x003C,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC
	}, { // 6
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0x003C,
		0x003C,
		0x003C,
		0x003C,
		0x003C,
		0x003C,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC
	}, { // 7
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000
	}, { // 8
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC
	}, { // 9
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC
	}
};

VOID* FpsCounter::operator new(size_t size) { return MemoryAlloc(size); }

VOID FpsCounter::operator delete(VOID* p) { MemoryFree(p); }

FpsCounter::FpsCounter(DWORD accuracy)
{
	this->accuracy = accuracy;
	this->count = accuracy * 10;
	this->tickQueue = (FrameItem*)MemoryAlloc(this->count * sizeof(FrameItem));
	this->Reset();
}

FpsCounter::~FpsCounter()
{
	MemoryFree(this->tickQueue);
}

VOID FpsCounter::Reset()
{
	this->checkIndex = 0;
	this->currentIndex = 0;
	this->summary = 0;
	this->lastTick = 0;
	this->value = 0;
	MemoryZero(this->tickQueue, this->count * sizeof(FrameItem));
}

VOID FpsCounter::Calculate()
{
	if (isFpsChanged)
	{
		isFpsChanged = FALSE;
		this->Reset();
	}

	FrameItem* tickItem = &tickQueue[this->currentIndex];
	tickItem->tick = GetTickCount();

	if (this->lastTick)
	{
		tickItem->span = tickItem->tick - this->lastTick;
		this->summary += tickItem->span;
	}
	this->lastTick = tickItem->tick;

	DWORD check = tickItem->tick - accuracy;

	DWORD total = 0;
	if (this->checkIndex > this->currentIndex)
	{
		FrameItem* checkPtr = &this->tickQueue[this->checkIndex];
		while (this->checkIndex < this->count)
		{
			if (checkPtr->tick > check)
			{
				total = this->count - this->checkIndex + this->currentIndex + 1;
				break;
			}

			this->summary -= checkPtr->span;

			++checkPtr;
			++this->checkIndex;
		}

		if (this->checkIndex == this->count)
			this->checkIndex = 0;
	}

	if (!total)
	{
		FrameItem* checkPtr = &this->tickQueue[this->checkIndex];
		while (this->checkIndex <= this->currentIndex)
		{
			if (checkPtr->tick > check)
			{
				total = this->currentIndex - this->checkIndex + 1;
				break;
			}

			this->summary -= checkPtr->span;

			++checkPtr;
			++this->checkIndex;
		}
	}

	if (this->currentIndex != this->count - 1)
		++this->currentIndex;
	else
		this->currentIndex = 0;

	this->value = this->summary ? 1000 * total / this->summary : 0;
}

VOID FpsCounter::Draw(VOID* srcBuffer, VOID* dstBuffer, DWORD frameWidth, DWORD frameHeight)
{
	DWORD fps = this->value;
	DWORD digCount = 0;
	DWORD current = fps;
	do
	{
		++digCount;
		current = current / 10;
	} while (current);

	if (config.mode->bpp != 16)
	{
		DWORD fpsColor = fpsState == FpsBenchmark ? 0xFF00FFFF : 0xFFFFFFFF;
		DWORD dcount = digCount;
		do
		{
			WORD* lpDig = (WORD*)counters[fps % 10];

			for (DWORD y = 0; y < FPS_HEIGHT; ++y)
			{
				DWORD* idx = (DWORD*)srcBuffer + (FPS_Y + y) * frameWidth +
					FPS_X + FPS_WIDTH * (dcount - 1);

				DWORD* pix = (DWORD*)dstBuffer + y * FPS_WIDTH * 4 +
					FPS_WIDTH * (dcount - 1);

				WORD check = *lpDig++;
				DWORD width = FPS_WIDTH;
				do
				{
					*pix++ = (check & 1) ? fpsColor : *idx;
					++idx;
					check >>= 1;
				} while (--width);
			}

			fps = fps / 10;
		} while (--dcount);

		dcount = 4;
		while (dcount != digCount)
		{
			for (DWORD y = 0; y < FPS_HEIGHT; ++y)
			{
				DWORD* idx = (DWORD*)srcBuffer + (FPS_Y + y) * frameWidth +
					FPS_X + FPS_WIDTH * (dcount - 1);

				DWORD* pix = (DWORD*)dstBuffer + y * FPS_WIDTH * 4 +
					FPS_WIDTH * (dcount - 1);

				DWORD width = FPS_WIDTH;
				do
					*pix++ = *idx++;
				while (--width);
			}

			--dcount;
		}

		GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, FPS_WIDTH * 4, FPS_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, dstBuffer);
	}
	else
	{
		if (glVersion > GL_VER_1_1)
		{
			WORD fpsColor = fpsState == FpsBenchmark ? 0xFFE0 : 0xFFFF;
			DWORD dcount = digCount;
			do
			{
				WORD* lpDig = (WORD*)counters[fps % 10];

				for (DWORD y = 0; y < FPS_HEIGHT; ++y)
				{
					WORD* idx = (WORD*)srcBuffer + (FPS_Y + y) * frameWidth +
						FPS_X + FPS_WIDTH * (dcount - 1);

					WORD* pix = (WORD*)dstBuffer + y * FPS_WIDTH * 4 +
						FPS_WIDTH * (dcount - 1);

					WORD check = *lpDig++;
					DWORD width = FPS_WIDTH;
					do
					{
						*pix++ = (check & 1) ? fpsColor : *idx;
						++idx;
						check >>= 1;
					} while (--width);
				}

				fps = fps / 10;
			} while (--dcount);

			dcount = 4;
			while (dcount != digCount)
			{
				for (DWORD y = 0; y < FPS_HEIGHT; ++y)
				{
					WORD* idx = (WORD*)srcBuffer + (FPS_Y + y) * frameWidth +
						FPS_X + FPS_WIDTH * (dcount - 1);

					WORD* pix = (WORD*)dstBuffer + y * FPS_WIDTH * 4 +
						FPS_WIDTH * (dcount - 1);

					DWORD width = FPS_WIDTH;
					do
						*pix++ = *idx++;
					while (--width);
				}

				--dcount;
			}

			GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, FPS_WIDTH * 4, FPS_HEIGHT, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, dstBuffer);
		}
		else
		{
			DWORD fpsColor = fpsState == FpsBenchmark ? 0xFF00FFFF : 0xFFFFFFFF;
			DWORD dcount = digCount;
			do
			{
				WORD* lpDig = (WORD*)counters[fps % 10];

				for (DWORD y = 0; y < FPS_HEIGHT; ++y)
				{
					WORD* idx = (WORD*)srcBuffer + (FPS_Y + y) * frameWidth +
						FPS_X + FPS_WIDTH * (dcount - 1);

					DWORD* pix = (DWORD*)dstBuffer + y * FPS_WIDTH * 4 +
						FPS_WIDTH * (dcount - 1);

					WORD check = *lpDig++;
					DWORD width = FPS_WIDTH;
					do
					{
						if (check & 1)
							*pix = fpsColor;
						else
						{
							WORD px = *idx;
							*pix = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
						}

						++pix;
						++idx;
						check >>= 1;
					} while (--width);
				}

				fps = fps / 10;
			} while (--dcount);

			dcount = 4;
			while (dcount != digCount)
			{
				for (DWORD y = 0; y < FPS_HEIGHT; ++y)
				{
					WORD* idx = (WORD*)srcBuffer + (FPS_Y + y) * frameWidth +
						FPS_X + FPS_WIDTH * (dcount - 1);

					DWORD* pix = (DWORD*)dstBuffer + y * FPS_WIDTH * 4 +
						FPS_WIDTH * (dcount - 1);

					DWORD width = FPS_WIDTH;
					do
					{
						WORD px = *idx++;
						*pix++ = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
					} while (--width);
				}

				--dcount;
			}

			GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, FPS_WIDTH * 4, FPS_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, dstBuffer);
		}
	}
}