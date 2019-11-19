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
#include "OpenDraw.h"
#include "Resource.h"
#include "CommCtrl.h"
#include "Main.h"
#include "FpsCounter.h"
#include "Config.h"
#include "Window.h"
#include "Hooks.h"

DWORD __fastcall GetPow2(DWORD value)
{
	DWORD res = 1;
	while (res < value)
		res <<= 1;
	return res;
}

VOID __fastcall UseShaderProgram(ShaderProgram* program, DWORD texSize)
{
	if (!program->id)
	{
		program->id = GLCreateProgram();

		GLBindAttribLocation(program->id, 0, "vCoord");
		GLBindAttribLocation(program->id, 1, "vTex");

		GLuint vShader = GL::CompileShaderSource(program->vertexName, program->version, GL_VERTEX_SHADER);
		GLuint fShader = GL::CompileShaderSource(program->fragmentName, program->version, GL_FRAGMENT_SHADER);
		{
			GLAttachShader(program->id, vShader);
			GLAttachShader(program->id, fShader);
			{
				GLLinkProgram(program->id);
			}
			GLDetachShader(program->id, fShader);
			GLDetachShader(program->id, vShader);
		}
		GLDeleteShader(fShader);
		GLDeleteShader(vShader);

		GLUseProgram(program->id);

		GLUniform1i(GLGetUniformLocation(program->id, "tex01"), 0);
		GLint loc = GLGetUniformLocation(program->id, "tex02");
		if (loc >= 0)
			GLUniform1i(loc, 1);

		program->texSize.location = GLGetUniformLocation(program->id, "texSize");
		if (program->texSize.location >= 0)
		{
			program->texSize.value = texSize;
			GLUniform2f(program->texSize.location, (FLOAT)LOWORD(texSize), (FLOAT)HIWORD(texSize));
		}
	}
	else
	{
		GLUseProgram(program->id);

		if (program->texSize.location >= 0 && program->texSize.value != texSize)
		{
			program->texSize.value = texSize;
			GLUniform2f(program->texSize.location, (FLOAT)LOWORD(texSize), (FLOAT)HIWORD(texSize));
		}
	}
}

VOID __fastcall FindMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP)
{
	const DisplayMode* mode = modesList;
	DWORD count = sizeof(modesList) / sizeof(DisplayMode);
	do
	{
		if (mode->width == dwWidth && mode->height == dwHeight && mode->bpp == dwBPP)
		{
			config.mode = mode;
			break;
		}

		++mode;
	} while (--count);
}

VOID __fastcall PrepareDescription(DDSURFACEDESC* lpDDSurfaceDesc, const DisplayMode* mode)
{
	lpDDSurfaceDesc->dwFlags = DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT;

	lpDDSurfaceDesc->dwWidth = mode->width;
	lpDDSurfaceDesc->dwHeight = mode->height;
	lpDDSurfaceDesc->lPitch = mode->width * (mode->bpp >> 3);

	lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount = mode->bpp;

	if (mode->bpp == 8)
		lpDDSurfaceDesc->ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;
	else
	{
		lpDDSurfaceDesc->ddpfPixelFormat.dwFlags = DDPF_RGB;
		lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask = 0xF800;
		lpDDSurfaceDesc->ddpfPixelFormat.dwGBitMask = 0x07E0;
		lpDDSurfaceDesc->ddpfPixelFormat.dwBBitMask = 0x001F;
		lpDDSurfaceDesc->ddpfPixelFormat.dwRGBAlphaBitMask = 0x0000;
	}
}

VOID OpenDraw::TakeSnapshot(VOID* buffer, Size* frameSize)
{
	this->isTakeSnapshot = FALSE;

	if (OpenClipboard(NULL))
	{
		EmptyClipboard();

		BOOL isTrueColor = config.mode->bpp != 16;

		DWORD pitch = frameSize->width * (isTrueColor ? 3 : 2);
		if (pitch & 3)
			pitch = (pitch & 0xFFFFFFFC) + 4;

		DWORD infoSize = sizeof(BITMAPINFOHEADER);
		if (!isTrueColor)
			infoSize += sizeof(DWORD) * 3;

		DWORD dataSize = pitch * frameSize->height;
		HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, infoSize + dataSize);
		{
			VOID* data = GlobalLock(hMemory);
			{
				BITMAPV4HEADER* bmiHeader = (BITMAPV4HEADER*)data;
				bmiHeader->bV4Size = sizeof(BITMAPINFOHEADER);
				bmiHeader->bV4Width = frameSize->width;
				bmiHeader->bV4Height = frameSize->height;
				bmiHeader->bV4Planes = 1;
				bmiHeader->bV4SizeImage = dataSize;
				bmiHeader->bV4XPelsPerMeter = 1;
				bmiHeader->bV4YPelsPerMeter = 1;
				bmiHeader->bV4ClrUsed = 0;
				bmiHeader->bV4ClrImportant = 0;

				if (isTrueColor)
				{
					bmiHeader->bV4BitCount = 24;
					bmiHeader->bV4V4Compression = BI_RGB;

					DWORD* src = (DWORD*)buffer;
					BYTE* dstData = (BYTE*)data + infoSize + dataSize - pitch;

					DWORD height = frameSize->height;
					do
					{
						BYTE* dst = dstData;

						DWORD width = frameSize->width;
						do
						{
							DWORD color = _byteswap_ulong(*src++);
							BYTE* px = (BYTE*)&color;

							*dst++ = *++px;
							*dst++ = *++px;
							*dst++ = *++px;
						} while (--width);

						dstData -= pitch;
					} while (--height);
				}
				else
				{
					bmiHeader->bV4BitCount = 16;
					bmiHeader->bV4V4Compression = BI_BITFIELDS;
					bmiHeader->bV4RedMask = 0xF800;
					bmiHeader->bV4GreenMask = 0x07E0;
					bmiHeader->bV4BlueMask = 0x001F;

					WORD* src = (WORD*)buffer;
					BYTE* dstData = (BYTE*)data + infoSize + dataSize - pitch;

					DWORD height = frameSize->height;
					do
					{
						BYTE* dst = dstData;

						DWORD width = frameSize->width;
						do
						{
							*(WORD*)dst = *src++;
							dst += 2;
						} while (--width);

						dstData -= pitch;
					} while (--height);
				}
			}
			GlobalUnlock(hMemory);
			SetClipboardData(CF_DIB, hMemory);
		}
		GlobalFree(hMemory);

		CloseClipboard();
	}
}

DWORD __stdcall RenderThread(LPVOID lpParameter)
{
	OpenDraw* ddraw = (OpenDraw*)lpParameter;
	ddraw->hDc = ::GetDC(ddraw->hDraw);
	if (ddraw->hDc)
	{
		if (!::GetPixelFormat(ddraw->hDc))
		{
			PIXELFORMATDESCRIPTOR pfd;
			INT glPixelFormat = GL::PreparePixelFormat(&pfd);
			if (!glPixelFormat)
			{
				glPixelFormat = ::ChoosePixelFormat(ddraw->hDc, &pfd);
				if (!glPixelFormat)
					Main::ShowError(IDS_ERROR_CHOOSE_PF, "OpenDraw.cpp", __LINE__);
				else if (pfd.dwFlags & PFD_NEED_PALETTE)
					Main::ShowError(IDS_ERROR_NEED_PALETTE, "OpenDraw.cpp", __LINE__);
			}

			GL::ResetPixelFormatDescription(&pfd);
			if (::DescribePixelFormat(ddraw->hDc, glPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == NULL)
				Main::ShowError(IDS_ERROR_DESCRIBE_PF, "OpenDraw.cpp", __LINE__);

			if (!::SetPixelFormat(ddraw->hDc, glPixelFormat, &pfd))
				Main::ShowError(IDS_ERROR_SET_PF, "OpenDraw.cpp", __LINE__);

			if ((pfd.iPixelType != PFD_TYPE_RGBA) || (pfd.cRedBits < 5) || (pfd.cGreenBits < 6) || (pfd.cBlueBits < 5))
				Main::ShowError(IDS_ERROR_BAD_PF, "OpenDraw.cpp", __LINE__);
		}

		HGLRC hRc = wglCreateContext(ddraw->hDc);
		if (hRc)
		{
			if (wglMakeCurrent(ddraw->hDc, hRc))
			{
				GL::CreateContextAttribs(ddraw->hDc, &hRc);
				if (glVersion >= GL_VER_2_0)
				{
					DWORD maxSize = config.mode->width > config.mode->height ? config.mode->width : config.mode->height;

					DWORD maxTexSize = 1;
					while (maxTexSize < maxSize)
						maxTexSize <<= 1;

					DWORD glMaxTexSize;
					GLGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&glMaxTexSize);
					if (maxTexSize > glMaxTexSize)
						glVersion = GL_VER_1_1;
				}

				if (glVersion >= GL_VER_2_0)
					ddraw->RenderNew();
				else
					ddraw->RenderOld();

				wglMakeCurrent(ddraw->hDc, NULL);
			}

			wglDeleteContext(hRc);
		}

		::ReleaseDC(ddraw->hDraw, ddraw->hDc);
		ddraw->hDc = NULL;
	}

	return NULL;
}

VOID OpenDraw::RenderOld()
{
	BOOL isTrueColor = config.mode->bpp != 16;

	if (config.image.filter > FilterLinear)
		config.image.filter = FilterLinear;

	DWORD glMaxTexSize;
	GLGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&glMaxTexSize);
	if (glMaxTexSize < 256)
		glMaxTexSize = 256;

	DWORD maxAllow = GetPow2(config.mode->width > config.mode->height ? config.mode->width : config.mode->height);

	DWORD maxTexSize = maxAllow < glMaxTexSize ? maxAllow : glMaxTexSize;
	DWORD glFilter = config.image.filter == FilterNearest ? GL_NEAREST : GL_LINEAR;

	DWORD framePerWidth = config.mode->width / maxTexSize + (config.mode->width % maxTexSize ? 1 : 0);
	DWORD framePerHeight = config.mode->height / maxTexSize + (config.mode->height % maxTexSize ? 1 : 0);
	DWORD frameCount = framePerWidth * framePerHeight;

	PostMessage(this->hWnd, config.msgMenu, NULL, NULL);

	Frame* frames = (Frame*)MemoryAlloc(frameCount * sizeof(Frame));
	{
		Frame* frame = frames;
		for (DWORD y = 0; y < config.mode->height; y += maxTexSize)
		{
			DWORD height = config.mode->height - y;
			if (height > maxTexSize)
				height = maxTexSize;

			for (DWORD x = 0; x < config.mode->width; x += maxTexSize, ++frame)
			{
				DWORD width = config.mode->width - x;
				if (width > maxTexSize)
					width = maxTexSize;

				frame->point.x = x;
				frame->point.y = y;

				frame->rect.x = x;
				frame->rect.y = y;
				frame->rect.width = width;
				frame->rect.height = height;

				frame->vSize.width = x + width;
				frame->vSize.height = y + height;

				frame->tSize.width = width == maxTexSize ? 1.0f : (FLOAT)width / maxTexSize;
				frame->tSize.height = height == maxTexSize ? 1.0f : (FLOAT)height / maxTexSize;

				GLGenTextures(1, &frame->id);

				GLBindTexture(GL_TEXTURE_2D, frame->id);

				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glCapsClampToEdge);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter);


				GLTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

				if (!isTrueColor && glVersion > GL_VER_1_1)
					GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
				else
					GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			}
		}

		GLMatrixMode(GL_PROJECTION);
		GLLoadIdentity();
		GLOrtho(0.0, (GLdouble)config.mode->width, (GLdouble)config.mode->height, 0.0, 0.0, 1.0);
		GLMatrixMode(GL_MODELVIEW);
		GLLoadIdentity();

		GLEnable(GL_TEXTURE_2D);
		GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		this->viewport.refresh = TRUE;

		VOID* frameBuffer = AlignedAlloc(maxTexSize * maxTexSize * (!isTrueColor && glVersion > GL_VER_1_1 ? sizeof(WORD) : sizeof(DWORD)));
		{
			FpsCounter* fpsCounter = new FpsCounter(FPS_ACCURACY);
			{
				BOOL isVSync = config.image.vSync;
				if (WGLSwapInterval)
					WGLSwapInterval(isVSync);

				while (!this->isFinish)
				{
					OpenDrawSurface* surface = this->attachedSurface;
					if (surface)
					{
						VOID* currentBuffer = surface->indexBuffer;

						if (isVSync != config.image.vSync)
						{
							isVSync = config.image.vSync;
							if (WGLSwapInterval)
								WGLSwapInterval(isVSync);
						}

						if (fpsState)
							fpsCounter->Calculate();

						if (this->CheckView(TRUE))
							GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);

						glFilter = 0;
						if (this->isStateChanged)
						{
							this->isStateChanged = FALSE;
							glFilter = config.image.filter == FilterNearest ? GL_NEAREST : GL_LINEAR;
						}

						Size* frameSize = (Size*)config.mode;

						DWORD count = frameCount;
						frame = frames;
						while (count--)
						{
							if (frameCount == 1)
							{
								if (glFilter)
								{
									GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter);
									GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter);
								}

								if (isTrueColor)
									GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameSize->width, frameSize->height, GL_RGBA, GL_UNSIGNED_BYTE, currentBuffer);
								else
								{
									if (glVersion > GL_VER_1_1)
										GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameSize->width, frameSize->height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, currentBuffer);
									else
									{
										WORD* source = (WORD*)currentBuffer;
										DWORD* dest = (DWORD*)frameBuffer;

										DWORD copyHeight = frameSize->height;
										do
										{
											WORD* src = source;
											source += frameSize->width;

											DWORD copyWidth = frameSize->width;
											do
											{
												WORD px = *src++;
												*dest++ = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
											} while (--copyWidth);
										} while (--copyHeight);

										GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameSize->width, frameSize->height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
									}
								}
							}
							else
							{
								GLBindTexture(GL_TEXTURE_2D, frame->id);

								if (glFilter)
								{
									GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter);
									GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter);
								}

								if (isTrueColor)
								{
									DWORD* source = (DWORD*)currentBuffer + frame->rect.y * frameSize->width + frame->rect.x;
									DWORD* dest = (DWORD*)frameBuffer;
									DWORD copyHeight = frame->rect.height;
									do
									{
										MemoryCopy(dest, source, frame->rect.width << 2);
										source += frameSize->width;
										dest += frame->rect.width;
									} while (--copyHeight);

									GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
								}
								else
								{
									if (glVersion > GL_VER_1_1)
									{
										WORD* source = (WORD*)currentBuffer + frame->rect.y * frameSize->width + frame->rect.x;
										WORD* dest = (WORD*)frameBuffer;
										DWORD copyHeight = frame->rect.height;
										do
										{
											MemoryCopy(dest, source, frame->rect.width << 1);
											source += frameSize->width;
											dest += frame->rect.width;
										} while (--copyHeight);

										GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, frameBuffer);
									}
									else
									{
										WORD* source = (WORD*)currentBuffer + frame->rect.y * frameSize->width + frame->rect.x;
										DWORD* dest = (DWORD*)frameBuffer;
										DWORD copyHeight = frame->rect.height;
										do
										{
											WORD* src = source;
											source += frameSize->width;

											DWORD count = frame->rect.width;
											do
											{
												WORD px = *src++;
												*dest++ = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
											} while (--count);
										} while (--copyHeight);

										GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
									}
								}
							}

							if (fpsState && !this->isTakeSnapshot && frame == frames)
								fpsCounter->Draw(currentBuffer, frameBuffer, frameSize->width, frameSize->height);

							GLBegin(GL_TRIANGLE_FAN);
							{
								GLTexCoord2f(0.0f, 0.0f);
								GLVertex2s((SHORT)frame->point.x, (SHORT)frame->point.y);

								GLTexCoord2f(frame->tSize.width, 0.0f);
								GLVertex2s((SHORT)frame->vSize.width, (SHORT)frame->point.y);

								GLTexCoord2f(frame->tSize.width, frame->tSize.height);
								GLVertex2s((SHORT)frame->vSize.width, (SHORT)frame->vSize.height);

								GLTexCoord2f(0.0f, frame->tSize.height);
								GLVertex2s((SHORT)frame->point.x, (SHORT)frame->vSize.height);
							}
							GLEnd();
							++frame;
						}

						if (this->isTakeSnapshot)
							this->TakeSnapshot(currentBuffer, frameSize);

						SwapBuffers(this->hDc);
						GLFinish();

						if (fpsState != FpsBenchmark)
							WaitForSingleObject(this->hDrawEvent, INFINITE);
						else
							Sleep(0);
					}
				}
			}
			delete fpsCounter;
		}
		AlignedFree(frameBuffer);

		frame = frames;
		DWORD count = frameCount;
		while (count--)
		{
			GLDeleteTextures(1, &frame->id);
			++frame;
		}
	}
	MemoryFree(frames);
}

VOID OpenDraw::RenderNew()
{
	PostMessage(this->hWnd, config.msgMenu, NULL, NULL);

	BOOL isTrueColor = config.mode->bpp != 16;
	DWORD maxTexSize = GetPow2(config.mode->width > config.mode->height ? config.mode->width : config.mode->height);

	FLOAT texWidth = config.mode->width == maxTexSize ? 1.0f : (FLOAT)config.mode->width / maxTexSize;
	FLOAT texHeight = config.mode->height == maxTexSize ? 1.0f : (FLOAT)config.mode->height / maxTexSize;

	DWORD texSize = (maxTexSize & 0xFFFF) | (maxTexSize << 16);

	FLOAT buffer[4][4] = {
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ (FLOAT)config.mode->width, 0.0f, texWidth, 0.0f },
		{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, texWidth, texHeight },
		{ 0.0f, (FLOAT)config.mode->height, 0.0f, texHeight },
	};

	FLOAT mvpMatrix[4][4] = {
		{ FLOAT(2.0f / config.mode->width), 0.0f, 0.0f, 0.0f },
		{ 0.0f, FLOAT(-2.0f / config.mode->height), 0.0f, 0.0f },
		{ 0.0f, 0.0f, 2.0f, 0.0f },
		{ -1.0f, 1.0f, -1.0f, 1.0f }
	};

	const CHAR* glslVersion = glVersion >= GL_VER_3_0 ? GLSL_VER_1_30 : GLSL_VER_1_10;

	struct {
		ShaderProgram linear;
		ShaderProgram hermite;
		ShaderProgram cubic;
	} shaders = {
		{ 0, glslVersion, IDR_LINEAR_VERTEX, IDR_LINEAR_FRAGMENT },
		{ 0, glslVersion, IDR_HERMITE_VERTEX, IDR_HERMITE_FRAGMENT },
		{ 0, glslVersion, IDR_CUBIC_VERTEX, IDR_CUBIC_FRAGMENT }
	};

	ShaderProgram* filterProgram = &shaders.linear;
	{
		GLuint bufferName;
		GLGenBuffers(1, &bufferName);
		{
			GLBindBuffer(GL_ARRAY_BUFFER, bufferName);
			{
				{
					FLOAT mvp[4][4] = {
						{ FLOAT(2.0f / config.mode->width), 0.0f, 0.0f, 0.0f },
						{ 0.0f, FLOAT(-2.0f / config.mode->height), 0.0f, 0.0f },
						{ 0.0f, 0.0f, 2.0f, 0.0f },
						{ -1.0f, 1.0f, -1.0f, 1.0f }
					};

					FLOAT buffer[16][8] = {
						{ 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
						{ (FLOAT)config.mode->width, 0.0f, 0.0f, 1.0f, texWidth, 0.0f },
						{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, 0.0f, 1.0f, texWidth, texHeight },
						{ 0.0f, (FLOAT)config.mode->height, 0.0f, 1.0f, 0.0f, texHeight }
					};

					for (DWORD i = 0; i < 4; ++i)
					{
						FLOAT* vector = &buffer[i][0];
						for (DWORD j = 0; j < 4; ++j)
						{
							FLOAT sum = 0.0f;
							for (DWORD v = 0; v < 4; ++v)
								sum += mvp[v][j] * vector[v];

							vector[j] = sum;
						}
					}

					GLBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);
				}

				{
					GLEnableVertexAttribArray(0);
					GLVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 32, (GLvoid*)0);

					GLEnableVertexAttribArray(1);
					GLVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 32, (GLvoid*)16);
				}

				GLuint textureId;
				GLGenTextures(1, &textureId);
				{
					DWORD filter = config.image.filter == FilterLinear ? GL_LINEAR : GL_NEAREST;
					GLActiveTexture(GL_TEXTURE0);

					GLBindTexture(GL_TEXTURE_2D, textureId);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

					if (isTrueColor)
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
					else
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);

					GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);
					this->viewport.refresh = TRUE;

					VOID* frameBuffer = AlignedAlloc(config.mode->width * config.mode->height * (isTrueColor ? sizeof(DWORD) : sizeof(WORD)));
					{
						FpsCounter* fpsCounter = new FpsCounter(FPS_ACCURACY);
						{
							this->isStateChanged = TRUE;

							BOOL isVSync = config.image.vSync;
							if (WGLSwapInterval)
								WGLSwapInterval(isVSync);

							while (!this->isFinish)
							{
								OpenDrawSurface* surface = this->attachedSurface;
								if (surface)
								{
									VOID* currentBuffer = surface->indexBuffer;

									if (isVSync != config.image.vSync)
									{
										isVSync = config.image.vSync;
										if (WGLSwapInterval)
											WGLSwapInterval(isVSync);
									}

									if (fpsState)
										fpsCounter->Calculate();

									if (this->CheckView(TRUE))
										GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);

									if (this->isStateChanged)
									{
										this->isStateChanged = FALSE;

										ImageFilter frameFilter = config.image.filter;

										ShaderProgram* program;
										switch (frameFilter)
										{
										case FilterHermite:
											program = &shaders.hermite;
											break;
										case FilterCubic:
											program = &shaders.cubic;
											break;
										default:
											program = &shaders.linear;
											break;
										}
										UseShaderProgram(program, texSize);

										GLBindTexture(GL_TEXTURE_2D, textureId);
										filter = frameFilter == FilterLinear || frameFilter == FilterHermite ? GL_LINEAR : GL_NEAREST;
										GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
										GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
									}

									Size* frameSize = (Size*)config.mode;
									if (isTrueColor)
										GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameSize->width, frameSize->height, GL_RGBA, GL_UNSIGNED_BYTE, currentBuffer);
									else
										GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameSize->width, frameSize->height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, currentBuffer);

									// Update FPS
									if (fpsState && !this->isTakeSnapshot)
										fpsCounter->Draw(currentBuffer, frameBuffer, frameSize->width, frameSize->height);

									GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);

									if (this->isTakeSnapshot)
										this->TakeSnapshot(currentBuffer, frameSize);

									SwapBuffers(this->hDc);
									GLFinish();

									if (fpsState != FpsBenchmark)
										WaitForSingleObject(this->hDrawEvent, INFINITE);
									else
										Sleep(0);
								}
							}
						}
						delete fpsCounter;
					}
					AlignedFree(frameBuffer);
				}
				GLDeleteTextures(1, &textureId);
			}
			GLBindBuffer(GL_ARRAY_BUFFER, NULL);
		}
		GLDeleteBuffers(1, &bufferName);
	}
	GLUseProgram(NULL);

	ShaderProgram* shaderProgram = (ShaderProgram*)&shaders;
	DWORD count = sizeof(shaders) / sizeof(ShaderProgram);
	do
	{
		if (shaderProgram->id)
			GLDeleteProgram(shaderProgram->id);

		++shaderProgram;
	} while (--count);
}

VOID OpenDraw::RenderStart()
{
	if (!this->isFinish || !this->hWnd)
		return;

	this->isFinish = FALSE;

	RECT rect;
	GetClientRect(this->hWnd, &rect);

	if (config.singleWindow)
		this->hDraw = this->hWnd;
	else
	{
		if (config.windowState == StateFullsreen)
			this->hDraw = CreateWindowEx(
				WS_EX_CONTROLPARENT | WS_EX_TOPMOST,
				WC_DRAW,
				NULL,
				WS_VISIBLE | WS_POPUP | WS_MAXIMIZE,
				0, 0,
				rect.right, rect.bottom,
				this->hWnd,
				NULL,
				hDllModule,
				NULL);
		else
		{
			this->hDraw = CreateWindowEx(
				WS_EX_CONTROLPARENT,
				WC_DRAW,
				NULL,
				WS_VISIBLE | WS_CHILD,
				0, 0,
				rect.right, rect.bottom,
				this->hWnd,
				NULL,
				hDllModule,
				NULL);
		}
		Window::SetCapturePanel(this->hDraw);

		SetClassLongPtr(this->hDraw, GCLP_HBRBACKGROUND, NULL);
		RedrawWindow(this->hDraw, NULL, NULL, RDW_INVALIDATE);
	}

	SetClassLongPtr(this->hWnd, GCLP_HBRBACKGROUND, NULL);
	RedrawWindow(this->hWnd, NULL, NULL, RDW_INVALIDATE);

	this->viewport.width = rect.right;
	this->viewport.height = rect.bottom;
	this->viewport.refresh = TRUE;

	DWORD threadId;
	SECURITY_ATTRIBUTES sAttribs;
	MemoryZero(&sAttribs, sizeof(SECURITY_ATTRIBUTES));
	sAttribs.nLength = sizeof(SECURITY_ATTRIBUTES);
	this->hDrawThread = CreateThread(&sAttribs, NULL, RenderThread, this, HIGH_PRIORITY_CLASS, &threadId);
}

VOID OpenDraw::RenderStop()
{
	if (this->isFinish)
		return;

	this->isFinish = TRUE;
	SetEvent(this->hDrawEvent);
	WaitForSingleObject(this->hDrawThread, INFINITE);
	CloseHandle(this->hDrawThread);
	this->hDrawThread = NULL;

	if (this->hDraw != this->hWnd)
	{
		DestroyWindow(this->hDraw);
		GL::ResetPixelFormat();
	}

	this->hDraw = NULL;

	ClipCursor(NULL);

	glVersion = NULL;
	Window::CheckMenu();
}

VOID OpenDraw::CalcView()
{
	this->viewport.rectangle.x = this->viewport.rectangle.y = 0;
	this->viewport.rectangle.width = this->viewport.width;
	this->viewport.rectangle.height = this->viewport.height;

	this->viewport.clipFactor.x = this->viewport.viewFactor.x = (FLOAT)this->viewport.width / config.mode->width;
	this->viewport.clipFactor.y = this->viewport.viewFactor.y = (FLOAT)this->viewport.height / config.mode->height;

	if (config.image.aspect && this->viewport.viewFactor.x != this->viewport.viewFactor.y)
	{
		if (this->viewport.viewFactor.x > this->viewport.viewFactor.y)
		{
			FLOAT fw = this->viewport.viewFactor.y * config.mode->width;
			this->viewport.rectangle.width = (INT)MathRound(fw);
			this->viewport.rectangle.x = (INT)MathRound(((FLOAT)this->viewport.width - fw) / 2.0f);
			this->viewport.clipFactor.x = this->viewport.viewFactor.y;
		}
		else
		{
			FLOAT fh = this->viewport.viewFactor.x * config.mode->height;
			this->viewport.rectangle.height = (INT)MathRound(fh);
			this->viewport.rectangle.y = (INT)MathRound(((FLOAT)this->viewport.height - fh) / 2.0f);
			this->viewport.clipFactor.y = this->viewport.viewFactor.x;
		}
	}
}

BOOL OpenDraw::CheckView(BOOL isDouble)
{
	if (this->viewport.refresh)
	{
		this->CalcView();

		HWND hActive = GetForegroundWindow();
		if (config.image.aspect && config.windowState == StateFullsreen && (hActive == this->hWnd || hActive == this->hDraw))
		{
			RECT clipRect;
			GetClientRect(this->hWnd, &clipRect);

			clipRect.left = this->viewport.rectangle.x;
			clipRect.right = clipRect.left + this->viewport.rectangle.width;
			clipRect.bottom = clipRect.bottom - this->viewport.rectangle.y;
			clipRect.top = clipRect.bottom - this->viewport.rectangle.height;

			ClientToScreen(this->hWnd, (POINT*)&clipRect.left);
			ClientToScreen(this->hWnd, (POINT*)&clipRect.right);

			ClipCursor(&clipRect);
		}
		else
			ClipCursor(NULL);

		this->clearStage = 0;
	}

	if (this->clearStage++ <= *(DWORD*)&isDouble)
		GLClear(GL_COLOR_BUFFER_BIT);

	if (this->viewport.refresh)
	{
		this->viewport.refresh = FALSE;
		return TRUE;
	}

	return FALSE;
}

VOID OpenDraw::ScaleMouse(LPPOINT p)
{
	if (this->viewport.rectangle.width && this->viewport.rectangle.height)
	{
		p->x = LONG((FLOAT)((p->x - this->viewport.rectangle.x) * config.mode->width) / this->viewport.rectangle.width);
		p->y = LONG((FLOAT)((p->y - this->viewport.rectangle.y) * config.mode->height) / this->viewport.rectangle.height);
	}
}

OpenDraw::OpenDraw(IDrawUnknown** list)
	: IDraw(list)
{
	this->surfaceEntries = NULL;
	this->clipperEntries = NULL;
	this->paletteEntries = NULL;

	this->attachedSurface = NULL;

	this->hDc = NULL;
	this->hWnd = NULL;
	this->hDraw = NULL;

	this->isTakeSnapshot = FALSE;
	this->isFinish = TRUE;

	this->hDrawEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

OpenDraw::~OpenDraw()
{
	this->RenderStop();
	CloseHandle(this->hDrawEvent);
}

ULONG __stdcall OpenDraw::Release()
{
	if (--this->refCount)
		return this->refCount;

	delete this;
	return 0;
}

HRESULT __stdcall OpenDraw::SetCooperativeLevel(HWND hWnd, DWORD dwFlags)
{
	this->hWnd = hWnd;

	if (dwFlags == (DDSCL_ALLOWREBOOT | DDSCL_NORMAL))
	{
		FindMode(1024, 768, 16);
		this->CalcView();
	}

	return DD_OK;
}

HRESULT __stdcall OpenDraw::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP)
{
	FindMode(dwWidth, dwHeight, dwBPP);
	this->CalcView();

	return DD_OK;
}

HRESULT __stdcall OpenDraw::CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc, IDrawSurface** lplpDDSurface, IDrawUnknown* pUnkOuter)
{
	// 1 - DDSD_CAPS // 512 - DDSCAPS_PRIMARYSURFACE
	// 7 - DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS // 2112 - DDSCAPS_SYSTEMMEMORY | DDSCAPS_OFFSCREENPLAIN

	OpenDrawSurface* surface;

	if (lpDDSurfaceDesc->dwFlags == (DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS))
	{
		surface = new OpenDrawSurface((IDrawUnknown**)&this->surfaceEntries, this, SurfaceSecondary);
		surface->AddRef();
		surface->CreateBuffer(lpDDSurfaceDesc->dwWidth, lpDDSurfaceDesc->dwHeight, config.mode->bpp);
		this->attachedSurface->attachedSurface = surface;
	}
	else
	{
		surface = new OpenDrawSurface((IDrawUnknown**)&this->surfaceEntries, this, SurfacePrimary);
		surface->CreateBuffer(config.mode->width, config.mode->height, config.mode->bpp);
		this->attachedSurface = surface;

		this->RenderStart();
	}

	*lplpDDSurface = surface;

	return DD_OK;
}

HRESULT __stdcall OpenDraw::CreateClipper(DWORD dwFlags, IDrawClipper** lplpDDClipper, IDrawUnknown* pUnkOuter)
{
	OpenDrawClipper* clipper = new OpenDrawClipper((IDrawUnknown**)&this->clipperEntries, this);
	*lplpDDClipper = clipper;

	return DD_OK;
}

HRESULT __stdcall OpenDraw::EnumDisplayModes(DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK lpEnumModesCallback)
{
	DDSURFACEDESC ddSurfaceDesc;
	MemoryZero(&ddSurfaceDesc, sizeof(DDSURFACEDESC));

	const DisplayMode* mode = modesList;
	DWORD count = sizeof(modesList) / sizeof(DisplayMode);
	do
	{
		PrepareDescription(&ddSurfaceDesc, mode);
		if (!lpEnumModesCallback(&ddSurfaceDesc, lpContext))
			break;

		++mode;
	} while (--count);

	return DD_OK;
}

HRESULT __stdcall OpenDraw::GetDisplayMode(LPDDSURFACEDESC lpDDSurfaceDesc)
{
	PrepareDescription(lpDDSurfaceDesc, config.mode);
	return DD_OK;
}

HRESULT __stdcall OpenDraw::CreatePalette(DWORD dwFlags, LPPALETTEENTRY lpDDColorArray, IDrawPalette** lplpDDPalette, IDrawUnknown* pUnkOuter)
{
	OpenDrawPalette* palette = new OpenDrawPalette((IDrawUnknown**)&this->paletteEntries, this);
	*lplpDDPalette = palette;
	MemoryCopy(palette->entries, lpDDColorArray, 256 * sizeof(PALETTEENTRY));

	return DD_OK;
}