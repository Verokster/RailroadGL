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
#include "Config.h"
#include "Resource.h"

ConfigItems config;

const DisplayMode modesList[] = {
	1024, 768, 16,
	1024, 768, 8,
	800, 600, 16,
	800, 600, 8,
	640, 480, 16,
	640, 480, 8
};

namespace Config
{
	BOOL __fastcall GetMenuByChildID(HMENU hParent, INT index, MenuItemData* mData)
	{
		HMENU hMenu = GetSubMenu(hParent, index);

		INT count = GetMenuItemCount(hMenu);
		for (INT i = 0; i < count; ++i)
		{
			UINT id = GetMenuItemID(hMenu, i);
			if (id == mData->childId)
			{
				mData->hParent = hParent;
				mData->index = index;

				return TRUE;
			}
			else if (GetMenuByChildID(hMenu, i, mData))
				return TRUE;
		}

		return FALSE;
	}

	BOOL __fastcall GetMenuByChildID(HMENU hMenu, MenuItemData* mData)
	{
		INT count = GetMenuItemCount(hMenu);
		for (INT i = 0; i < count; ++i)
		{
			if (GetMenuByChildID(hMenu, i, mData))
				return TRUE;
		}

		return FALSE;
	}

	BOOL __fastcall Load()
	{
		HMODULE hModule = GetModuleHandle(NULL);

		GetModuleFileName(hModule, config.file, MAX_PATH - 1);
		CHAR * p = StrLastChar(config.file, '.');
		*p = NULL;
		StrCopy(p, ".ini");

		config.isExist = !(BOOL)Config::Get(CONFIG_WRAPPER, "ReInit", TRUE);
		Config::Set(CONFIG_WRAPPER, "ReInit", FALSE);

		if (!config.isExist)
		{
			config.image.aspect = TRUE;
			Config::Set(CONFIG_WRAPPER, "ImageAspect", config.image.aspect);

			config.image.vSync = TRUE;
			Config::Set(CONFIG_WRAPPER, "ImageVSync", config.image.vSync);

			config.image.filter = FilterCubic;
			Config::Set(CONFIG_WRAPPER, "ImageFilter", *(INT*)&config.image.filter);

			config.image.scaleNx.value = 2;
			Config::Set(CONFIG_WRAPPER, "ImageScaleNx", *(INT*)&config.image.scaleNx);

			config.image.xSal.value = 2;
			Config::Set(CONFIG_WRAPPER, "ImageXSal", *(INT*)&config.image.xSal);

			config.image.eagle.value = 2;
			Config::Set(CONFIG_WRAPPER, "ImageEagle", *(INT*)&config.image.eagle);

			config.image.scaleHQ.value = 2;
			Config::Set(CONFIG_WRAPPER, "ImageScaleHQ", *(INT*)&config.image.scaleHQ);

			config.image.xBRz.value = 2;
			Config::Set(CONFIG_WRAPPER, "ImageXBRZ", *(INT*)&config.image.xBRz);

			config.keys.fpsCounter = 1;
			Config::Set(CONFIG_KEYS, "FpsCounter", config.keys.fpsCounter);

			config.keys.imageFilter = 2;
			Config::Set(CONFIG_KEYS, "ImageFilter", config.keys.imageFilter);

			Config::Set(CONFIG_KEYS, "AspectRatio", "");
			Config::Set(CONFIG_KEYS, "VSync", "");

			Config::Set(CONFIG_KEYS, "AlwaysActive", "");
		}
		else
		{
			config.image.aspect = (BOOL)Config::Get(CONFIG_WRAPPER, "ImageAspect", TRUE);
			config.image.vSync = (BOOL)Config::Get(CONFIG_WRAPPER, "ImageVSync", TRUE);

			INT value = Config::Get(CONFIG_WRAPPER, "ImageFilter", FilterCubic);
			config.image.filter = *(ImageFilter*)&value;
			if (config.image.filter < FilterNearest || config.image.filter > FilterScaleNx)
				config.image.filter = FilterCubic;

			value = Config::Get(CONFIG_WRAPPER, "ImageScaleNx", 2);
			config.image.scaleNx = *(FilterType*)&value;
			if (config.image.scaleNx.value != 2 && config.image.scaleNx.value != 3)
				config.image.scaleNx.value = 2;
			if (config.image.scaleNx.type & 0xFE)
				config.image.scaleNx.type = 0;

			value = Config::Get(CONFIG_WRAPPER, "ImageXSal", 2);
			config.image.xSal = *(FilterType*)&value;
			if (config.image.xSal.value != 2)
				config.image.xSal.value = 2;
			if (config.image.xSal.type & 0xFE)
				config.image.xSal.type = 0;

			value = Config::Get(CONFIG_WRAPPER, "ImageEagle", 2);
			config.image.eagle = *(FilterType*)&value;
			if (config.image.eagle.value != 2)
				config.image.eagle.value = 2;
			if (config.image.eagle.type & 0xFE)
				config.image.eagle.type = 0;

			value = Config::Get(CONFIG_WRAPPER, "ImageScaleHQ", 2);
			config.image.scaleHQ = *(FilterType*)&value;
			if (config.image.scaleHQ.value != 2 && config.image.scaleHQ.value != 4)
				config.image.scaleHQ.value = 2;
			if (config.image.scaleHQ.type & 0xFE)
				config.image.scaleHQ.type = 0;

			value = Config::Get(CONFIG_WRAPPER, "ImageXBRZ", 2);
			config.image.xBRz = *(FilterType*)&value;
			if (config.image.xBRz.value < 2 || config.image.xBRz.value > 6)
				config.image.xBRz.value = 6;
			if (config.image.xBRz.type & 0xFE)
				config.image.xBRz.type = 0;

			CHAR buffer[20];
			if (Config::Get(CONFIG_KEYS, "FpsCounter", "", buffer, sizeof(buffer)))
			{
				value = Config::Get(CONFIG_KEYS, "FpsCounter", 0);
				config.keys.fpsCounter = LOBYTE(value);
				if (config.keys.fpsCounter > 1 && config.keys.fpsCounter > 24)
					config.keys.fpsCounter = 0;
			}

			if (Config::Get(CONFIG_KEYS, "ImageFilter", "", buffer, sizeof(buffer)))
			{
				value = Config::Get(CONFIG_KEYS, "ImageFilter", 0);
				config.keys.imageFilter = LOBYTE(value);
				if (config.keys.imageFilter > 1 && config.keys.imageFilter > 24)
					config.keys.imageFilter = 0;
			}

			if (Config::Get(CONFIG_KEYS, "AspectRatio", "", buffer, sizeof(buffer)))
			{
				value = Config::Get(CONFIG_KEYS, "AspectRatio", 0);
				config.keys.aspectRatio = LOBYTE(value);
				if (config.keys.aspectRatio > 1 && config.keys.aspectRatio > 24)
					config.keys.aspectRatio = 0;
			}

			if (Config::Get(CONFIG_KEYS, "VSync", "", buffer, sizeof(buffer)))
			{
				value = Config::Get(CONFIG_KEYS, "VSync", 0);
				config.keys.vSync = LOBYTE(value);
				if (config.keys.vSync > 1 && config.keys.vSync > 24)
					config.keys.vSync = 0;
			}
		}

		config.menu = LoadMenu(hDllModule, MAKEINTRESOURCE(IDR_MENU));
		config.font = (HFONT)CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
			OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, TEXT("MS Shell Dlg"));
		config.icon = LoadIcon(hModule, "IHS");

		HMODULE hLibrary = LoadLibrary("NTDLL.dll");
		if (hLibrary)
		{
			if (GetProcAddress(hLibrary, "wine_get_version"))
				config.singleWindow = TRUE;
			FreeLibrary(hLibrary);
		}

		if (!config.singleWindow)
			config.singleWindow = Config::Get(CONFIG_WRAPPER, "SingleWindow", FALSE);

		CHAR buffer[256];
		MENUITEMINFO info;
		MemoryZero(&info, sizeof(MENUITEMINFO));
		info.cbSize = sizeof(MENUITEMINFO);
		info.fMask = MIIM_TYPE;
		info.fType = MFT_STRING;
		info.dwTypeData = buffer;

		if (config.keys.aspectRatio && (info.cch = sizeof(buffer), GetMenuItemInfo(config.menu, IDM_ASPECT_RATIO, FALSE, &info)))
		{
			StrPrint(buffer, "%sF%d", buffer, config.keys.aspectRatio);
			SetMenuItemInfo(config.menu, IDM_ASPECT_RATIO, FALSE, &info);
		}

		if (config.keys.vSync && (info.cch = sizeof(buffer), GetMenuItemInfo(config.menu, IDM_VSYNC, FALSE, &info)))
		{
			StrPrint(buffer, "%sF%d", buffer, config.keys.vSync);
			SetMenuItemInfo(config.menu, IDM_VSYNC, FALSE, &info);
		}

		MenuItemData mData;
		if (config.keys.imageFilter)
		{
			mData.childId = IDM_FILT_OFF;
			if (GetMenuByChildID(config.menu, &mData) && (info.cch = sizeof(buffer), GetMenuItemInfo(mData.hParent, mData.index, TRUE, &info)))
			{
				StrPrint(buffer, "%sF%d", buffer, config.keys.imageFilter);
				SetMenuItemInfo(mData.hParent, mData.index, TRUE, &info);
			}
		}

		return TRUE;
	}

	INT __fastcall Get(const CHAR* app, const CHAR* key, INT default)
	{
		return GetPrivateProfileInt(app, key, (INT)default, config.file);
	}

	DWORD __fastcall Get(const CHAR* app, const CHAR* key, CHAR* default, CHAR* returnString, DWORD nSize)
	{
		return GetPrivateProfileString(app, key, default, returnString, nSize, config.file);
	}

	BOOL __fastcall Set(const CHAR* app, const CHAR* key, INT value)
	{
		CHAR res[20];
		StrPrint(res, "%d", value);
		return WritePrivateProfileString(app, key, res, config.file);
	}

	BOOL __fastcall Set(const CHAR* app, const CHAR* key, CHAR* value)
	{
		return WritePrivateProfileString(app, key, value, config.file);
	}
}