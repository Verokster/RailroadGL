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
#include "Shellapi.h"
#include "Objbase.h"
#include "Mmsystem.h"
#include "Commdlg.h"
#include "Hooks.h"
#include "Main.h"
#include "Config.h"
#include "Resource.h"
#include "Window.h"

#define CHECKVALUE (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)

namespace Hooks
{
	HMODULE hModule;
	INT baseOffset;

#pragma region Hook helpers
#pragma optimize("s", on)
	BOOL __fastcall PatchBlock(DWORD addr, VOID* block, DWORD size)
	{
		DWORD address = addr + baseOffset;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, size, PAGE_EXECUTE_READWRITE, &old_prot))
		{
			switch (size)
			{
			case 4:
				*(DWORD*)address = *(DWORD*)block;
				break;
			case 2:
				*(WORD*)address = *(WORD*)block;
				break;
			case 1:
				*(BYTE*)address = *(BYTE*)block;
				break;
			default:
				MemoryCopy((VOID*)address, block, size);
				break;
			}

			VirtualProtect((VOID*)address, size, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall ReadBlock(DWORD addr, VOID* block, DWORD size)
	{
		DWORD address = addr + baseOffset;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, size, PAGE_READONLY, &old_prot))
		{
			switch (size)
			{
			case 4:
				*(DWORD*)block = *(DWORD*)address;
				break;
			case 2:
				*(WORD*)block = *(WORD*)address;
				break;
			case 1:
				*(BYTE*)block = *(BYTE*)address;
				break;
			default:
				MemoryCopy(block, (VOID*)address, size);
				break;
			}

			VirtualProtect((VOID*)address, size, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall PatchDWord(DWORD addr, DWORD value)
	{
		return PatchBlock(addr, &value, sizeof(value));
	}

	BOOL __fastcall ReadWord(DWORD addr, WORD* value)
	{
		return ReadBlock(addr, value, sizeof(*value));
	}

	BOOL __fastcall ReadDWord(DWORD addr, DWORD* value)
	{
		return ReadBlock(addr, value, sizeof(*value));
	}

	BOOL __fastcall ReadRedirect(DWORD addr, DWORD* value)
	{
		if (ReadDWord(addr + 1, value))
		{
			*value += addr + baseOffset + 5;
			return TRUE;
		}
		else
			return FALSE;
	}

	DWORD __fastcall PatchFunction(MappedFile* file, const CHAR* function, VOID* addr)
	{
		DWORD res = NULL;

		DWORD base = (DWORD)file->hModule;
		PIMAGE_NT_HEADERS headNT = (PIMAGE_NT_HEADERS)((BYTE*)base + ((PIMAGE_DOS_HEADER)file->hModule)->e_lfanew);

		PIMAGE_DATA_DIRECTORY dataDir = &headNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
		if (dataDir->Size)
		{
			PIMAGE_IMPORT_DESCRIPTOR imports = (PIMAGE_IMPORT_DESCRIPTOR)(base + dataDir->VirtualAddress);
			for (DWORD idx = 0; imports->Name; ++idx, ++imports)
			{
				CHAR* libraryName = (CHAR*)(base + imports->Name);

				PIMAGE_THUNK_DATA addressThunk = (PIMAGE_THUNK_DATA)(base + imports->FirstThunk);
				PIMAGE_THUNK_DATA nameThunk;
				if (imports->OriginalFirstThunk)
					nameThunk = (PIMAGE_THUNK_DATA)(base + imports->OriginalFirstThunk);
				else
				{
					if (!file->hFile)
					{
						CHAR filePath[MAX_PATH];
						GetModuleFileName(file->hModule, filePath, MAX_PATH);
						file->hFile = CreateFile(filePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
						if (!file->hFile)
							return res;
					}

					if (!file->hMap)
					{
						file->hMap = CreateFileMapping(file->hFile, NULL, PAGE_READONLY, 0, 0, NULL);
						if (!file->hMap)
							return res;
					}

					if (!file->address)
					{
						file->address = MapViewOfFile(file->hMap, FILE_MAP_READ, 0, 0, 0);;
						if (!file->address)
							return res;
					}

					headNT = (PIMAGE_NT_HEADERS)((BYTE*)file->address + ((PIMAGE_DOS_HEADER)file->address)->e_lfanew);
					PIMAGE_SECTION_HEADER sh = (PIMAGE_SECTION_HEADER)((DWORD)& headNT->OptionalHeader + headNT->FileHeader.SizeOfOptionalHeader);

					nameThunk = NULL;
					DWORD sCount = headNT->FileHeader.NumberOfSections;
					while (sCount--)
					{
						if (imports->FirstThunk >= sh->VirtualAddress && imports->FirstThunk < sh->VirtualAddress + sh->Misc.VirtualSize)
						{
							nameThunk = PIMAGE_THUNK_DATA((DWORD)file->address + sh->PointerToRawData + imports->FirstThunk - sh->VirtualAddress);
							break;
						}

						++sh;
					}

					if (!nameThunk)
						return res;
				}

				for (; nameThunk->u1.AddressOfData; ++nameThunk, ++addressThunk)
				{
					PIMAGE_IMPORT_BY_NAME name = PIMAGE_IMPORT_BY_NAME(base + nameThunk->u1.AddressOfData);

					WORD hint;
					if (ReadWord((INT)name - baseOffset, &hint) && !StrCompare((CHAR*)name->Name, function))
					{
						if (ReadDWord((INT)& addressThunk->u1.AddressOfData - baseOffset, &res))
							PatchDWord((INT)& addressThunk->u1.AddressOfData - baseOffset, (DWORD)addr);

						return res;
					}
				}
			}
		}

		return res;
	}
#pragma optimize("", on)
#pragma endregion

	// ===============================================================
	INT __stdcall MessageBoxHook(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
	{
		INT res;
		ULONG_PTR cookie = NULL;
		if (hActCtx && hActCtx != INVALID_HANDLE_VALUE && !ActivateActCtxC(hActCtx, &cookie))
			cookie = NULL;

		res = MessageBox(hWnd, lpText, lpCaption, uType);

		if (cookie)
			DeactivateActCtxC(0, cookie);

		return res;
	}

	INT __stdcall GetDeviceCapsHook(HDC hdc, INT index)
	{
		switch (index)
		{
			//case HORZRES:
			//	return 1152;
			//case VERTRES:
			//	return 864;
		case BITSPIXEL:
			return 16;
		default:
			return GetDeviceCaps(hdc, index);
		}
	}

	ATOM __stdcall RegisterClassHook(WNDCLASSA* lpWndClass)
	{
		lpWndClass->hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		lpWndClass->hIcon = config.icon;

		return RegisterClass(lpWndClass);
	}

	RECT winLoc;
	BOOL __stdcall AdjustWindowRectExHook(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle)
	{
		dwStyle = config.isPlatinum ? WS_WINDOWED_DDRAW : WS_WINDOWED_GDI;

		if (AdjustWindowRectEx(lpRect, dwStyle, TRUE, dwExStyle))
		{
			LONG nWidth = lpRect->right - lpRect->left + 1;
			LONG nHeight = lpRect->bottom - lpRect->top + 1;

			OffsetRect(lpRect, ((GetSystemMetrics(SM_CXSCREEN) - nWidth) >> 1) - lpRect->left, ((GetSystemMetrics(SM_CYSCREEN) - nHeight) >> 1) - lpRect->top);

			winLoc.left = lpRect->left;
			winLoc.top = lpRect->top;

			return TRUE;
		}

		return FALSE;
	}

	HWND __stdcall CreateWindowExHook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, INT X, INT Y, INT nWidth, INT nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
	{
		dwStyle = config.isPlatinum ? WS_WINDOWED_DDRAW : WS_WINDOWED_GDI;

		HWND hWnd = CreateWindowEx(dwExStyle, lpClassName, lpWindowName, dwStyle, winLoc.left, winLoc.top, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		Window::SetCaptureWindow(hWnd);
		return hWnd;
	}

	LONG __stdcall SetWindowLongHook(HWND hWnd, INT nIndex, LONG dwNewLong)
	{
		if (nIndex != GWL_STYLE)
			return SetWindowLong(hWnd, nIndex, dwNewLong);
		else
		{
			BOOL isWindowed = dwNewLong != WS_CLIPSIBLINGS && dwNewLong != (WS_CLIPSIBLINGS | WS_VISIBLE);
			if (isWindowed)
			{
				DWORD dwStyle = config.isPlatinum ? ((dwNewLong | WS_VISIBLE) ? (WS_WINDOWED_DDRAW | WS_VISIBLE) : WS_WINDOWED_DDRAW) : dwNewLong;

				config.windowState = StateWindowed;

				if (!config.windowPlacement.length)
				{
					config.windowPlacement.length = sizeof(WINDOWPLACEMENT);
					GetWindowPlacement(hWnd, &config.windowPlacement);

					config.windowPlacement.ptMinPosition.x = config.windowPlacement.ptMinPosition.y = -1;
					config.windowPlacement.ptMaxPosition.x = config.windowPlacement.ptMaxPosition.y = -1;

					config.windowPlacement.flags = NULL;
					config.windowPlacement.showCmd = SW_SHOWNORMAL;
				}

				SetMenu(hWnd, config.menu);

				SetWindowLong(hWnd, GWL_STYLE, dwStyle);
				SetWindowPlacement(hWnd, &config.windowPlacement);
			}
			else
			{
				if (config.windowState != StateFullsreen)
				{
					config.windowPlacement.length = sizeof(WINDOWPLACEMENT);
					config.windowPlacement.flags = NULL;
					config.windowPlacement.showCmd = SW_SHOWNORMAL;

					GetWindowPlacement(hWnd, &config.windowPlacement);
				}

				config.windowState = StateFullsreen;

				SetMenu(hWnd, NULL);

				SetWindowLong(hWnd, GWL_STYLE, (dwNewLong | WS_VISIBLE) ? (WS_FULLSCREEN | WS_VISIBLE) : WS_FULLSCREEN);
				SetWindowPos(hWnd, NULL, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL);
			}

			SetForegroundWindow(hWnd);
			return TRUE;
		}
	}

	BOOL __stdcall SetWindowPosHook(HWND hWnd, HWND hWndInsertAfter, INT X, INT Y, INT cx, INT cy, UINT uFlags)
	{
		return TRUE;
	}

	BOOL __stdcall ScreenToClientHook(HWND hWnd, LPPOINT lpPoint)
	{
		if (ScreenToClient(hWnd, lpPoint))
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
				ddraw->ScaleMouse(lpPoint);

			return TRUE;
		}
		return FALSE;
	}

	HMENU __stdcall LoadMenuHook(HINSTANCE hInstance, LPCSTR lpMenuName)
	{
		return config.menu;
	}

	BOOL __stdcall ShowWindowHook(HWND hWnd, INT nCmdShow)
	{
		if (config.windowState == StateWindowed)
			SetMenu(hWnd, config.menu);

		return ShowWindow(hWnd, nCmdShow);
	}

	RECT paint;
	BOOL __stdcall InvalidateRectHook(HWND hWnd, RECT* lpRect, BOOL bErase)
	{
		if (config.isPlatinum || config.windowState == StateFullsreen)
		{
			paint = *lpRect;
			return TRUE;
		}
		else
			return InvalidateRect(hWnd, lpRect, bErase);
	}

	BOOL __stdcall UpdateWindowHook(HWND hWnd)
	{
		if (config.isPlatinum || config.windowState == StateFullsreen)
		{
			CallWindowProc(Window::OldWindowProc, hWnd, WM_PAINT, NULL, NULL);
			return TRUE;
		}
		else
			return UpdateWindow(hWnd);
	}

	HDC __stdcall BeginPaintHook(HWND hWnd, LPPAINTSTRUCT lpPaint)
	{
		if (config.isPlatinum || config.windowState == StateFullsreen)
		{
			lpPaint->rcPaint = paint;
			return NULL;
		}
		else
			return BeginPaint(hWnd, lpPaint);
	}

	BOOL __stdcall EndPaintHook(HWND hWnd, PAINTSTRUCT* lpPaint)
	{
		if (config.isPlatinum || config.windowState == StateFullsreen)
			return TRUE;
		else
			return EndPaint(hWnd, lpPaint);
	}

	DECLARE_HANDLE(HSMACK);
	DWORD _SmackOpen;
	HSMACK __stdcall SmackOpenHook(CHAR* hSrcFile, DWORD uFlags, DWORD uExtraBuffers)
	{
		if (uFlags == 0xFE000)
		{
			CHAR* p = StrLastChar(hSrcFile, '\\');
			if (!p)
				p = hSrcFile;
			else
				++p;

			if (StrCompareInsensitive(p, "intro_01.smk"))
				uFlags |= 0x200100;
		}

		return ((HSMACK(__stdcall*)(CHAR*, DWORD, DWORD))_SmackOpen)(hSrcFile, uFlags, uExtraBuffers);
	}

	HMODULE __stdcall LoadLibraryHook(LPCSTR lpLibFileName)
	{
		if (!StrCompareInsensitive(lpLibFileName, "DDRAW.dll"))
			return hDllModule;
		return LoadLibrary(lpLibFileName);
	}

	BOOL __stdcall FreeLibraryHook(HMODULE hLibModule)
	{
		if (hLibModule == hDllModule)
			return TRUE;
		return FreeLibrary(hLibModule);
	}

	FARPROC __stdcall GetProcAddressHook(HMODULE hModule, LPCSTR lpProcName)
	{
		if (hModule == hDllModule)
		{
			if (!StrCompareInsensitive(lpProcName, "DirectDrawCreate"))
				return (FARPROC)Main::DrawCreate;
			else
				return NULL;
		}
		else
			return GetProcAddress(hModule, lpProcName);
	}

#pragma optimize("s", on)
	VOID Load()
	{
		hModule = GetModuleHandle(NULL);
		PIMAGE_NT_HEADERS headNT = (PIMAGE_NT_HEADERS)((BYTE*)hModule + ((PIMAGE_DOS_HEADER)hModule)->e_lfanew);
		baseOffset = (INT)hModule - (INT)headNT->OptionalHeader.ImageBase;

		{
			MappedFile file = { hModule, NULL, NULL, NULL };
			{
				PatchFunction(&file, "MessageBoxA", MessageBoxHook);
				PatchFunction(&file, "GetDeviceCaps", GetDeviceCapsHook);
				PatchFunction(&file, "RegisterClassA", RegisterClassHook);
				PatchFunction(&file, "AdjustWindowRectEx", AdjustWindowRectExHook);
				PatchFunction(&file, "CreateWindowExA", CreateWindowExHook);
				PatchFunction(&file, "SetWindowLongA", SetWindowLongHook);
				PatchFunction(&file, "ShowWindow", ShowWindowHook);

				PatchFunction(&file, "SetWindowPos", SetWindowPosHook);
				PatchFunction(&file, "ScreenToClient", ScreenToClientHook);
				PatchFunction(&file, "LoadMenuA", LoadMenuHook);

				PatchFunction(&file, "InvalidateRect", InvalidateRectHook);
				PatchFunction(&file, "UpdateWindow", UpdateWindowHook);
				PatchFunction(&file, "BeginPaint", BeginPaintHook);
				PatchFunction(&file, "EndPaint", EndPaintHook);

				_SmackOpen = PatchFunction(&file, "_SmackOpen@12", SmackOpenHook);

				PatchFunction(&file, "LoadLibraryA", LoadLibraryHook);
				PatchFunction(&file, "FreeLibrary", FreeLibraryHook);
				PatchFunction(&file, "GetProcAddress", GetProcAddressHook);

				config.isPlatinum = PatchFunction(&file, "DirectDrawCreate", Main::DrawCreate);
			}

			if (file.address)
				UnmapViewOfFile(file.address);

			if (file.hMap)
				CloseHandle(file.hMap);

			if (file.hFile)
				CloseHandle(file.hFile);
		}
	}
#pragma optimize("", on)
}