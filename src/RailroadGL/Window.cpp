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
#include "Window.h"
#include "CommCtrl.h"
#include "Windowsx.h"
#include "Shellapi.h"
#include "Mmsystem.h"
#include "Resource.h"
#include "Main.h"
#include "Config.h"
#include "Hooks.h"
#include "FpsCounter.h"
#include "OpenDraw.h"

namespace Window
{
	HHOOK OldKeysHook;
	WNDPROC OldWindowProc, OldPanelProc;

	BOOL __fastcall FindMenuByChildId(HMENU hMenu, DWORD childId, HMENU* hParent, DWORD* pos)
	{
		DWORD count = GetMenuItemCount(hMenu);
		while (count--)
		{
			HMENU hSub = GetSubMenu(hMenu, count);
			if (hSub)
			{
				DWORD subCount = GetMenuItemCount(hSub);
				while (subCount--)
				{
					if (GetMenuItemID(hSub, subCount) == childId)
					{
						*hParent = hMenu;
						*pos = count;
						return TRUE;
					}

					if (FindMenuByChildId(hSub, childId, hParent, pos))
						return TRUE;
				}
			}
		}

		return FALSE;
	}

	VOID __fastcall CheckMenu(MenuType type)
	{
		switch (type)
		{
		case MenuAspect:
		{
			EnableMenuItem(config.menu, IDM_ASPECT_RATIO, MF_BYCOMMAND | (glVersion ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			CheckMenuItem(config.menu, IDM_ASPECT_RATIO, MF_BYCOMMAND | (glVersion && config.image.aspect ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuVSync:
		{
			EnableMenuItem(config.menu, IDM_VSYNC, MF_BYCOMMAND | (glVersion && WGLSwapInterval ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			CheckMenuItem(config.menu, IDM_VSYNC, MF_BYCOMMAND | (glVersion && WGLSwapInterval && config.image.vSync ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuFiltering:
		{
			CheckMenuItem(config.menu, IDM_FILT_OFF, MF_BYCOMMAND | MF_UNCHECKED);

			EnableMenuItem(config.menu, IDM_FILT_LINEAR, MF_BYCOMMAND | (glVersion ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			CheckMenuItem(config.menu, IDM_FILT_LINEAR, MF_BYCOMMAND | MF_UNCHECKED);

			DWORD isFilters = glVersion >= GL_VER_2_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED);

			EnableMenuItem(config.menu, IDM_FILT_HERMITE, MF_BYCOMMAND | isFilters);
			CheckMenuItem(config.menu, IDM_FILT_HERMITE, MF_BYCOMMAND | MF_UNCHECKED);

			EnableMenuItem(config.menu, IDM_FILT_CUBIC, MF_BYCOMMAND | isFilters);
			CheckMenuItem(config.menu, IDM_FILT_CUBIC, MF_BYCOMMAND | MF_UNCHECKED);

			switch (config.image.filter)
			{
			case FilterLinear:
				CheckMenuItem(config.menu, IDM_FILT_LINEAR, MF_BYCOMMAND | MF_CHECKED);
				break;

			case FilterHermite:
				CheckMenuItem(config.menu, isFilters == MF_ENABLED ? IDM_FILT_HERMITE : IDM_FILT_LINEAR, MF_BYCOMMAND | MF_CHECKED);
				break;

			case FilterCubic:
				CheckMenuItem(config.menu, isFilters == MF_ENABLED ? IDM_FILT_CUBIC : IDM_FILT_LINEAR, MF_BYCOMMAND | MF_CHECKED);
				break;

			default:
				CheckMenuItem(config.menu, IDM_FILT_OFF, MF_BYCOMMAND | MF_CHECKED);
				break;
			}
		}
		break;

		case MenuWindowMode:
		{
			CheckMenuItem(config.menu, IDM_RES_FULL_SCREEN, MF_BYCOMMAND | (config.windowState != StateWindowed ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		default:
			break;
		}
	}

	VOID __fastcall CheckMenu()
	{
		CheckMenu(MenuAspect);
		CheckMenu(MenuVSync);
		CheckMenu(MenuFiltering);
		CheckMenu(MenuWindowMode);
	}

	VOID __fastcall FilterChanged(HWND hWnd)
	{
		Config::Set(CONFIG_WRAPPER, "ImageFilter", *(INT*)&config.image.filter);
		CheckMenu(MenuFiltering);

		OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
		if (ddraw)
		{
			ddraw->isStateChanged = TRUE;
			SetEvent(ddraw->hDrawEvent);
		}
	}

	BOOL __stdcall EnumChildProc(HWND hDlg, LPARAM lParam)
	{
		if ((GetWindowLong(hDlg, GWL_STYLE) & SS_ICON) == SS_ICON)
			SendMessage(hDlg, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)config.icon);
		else
			SendMessage(hDlg, WM_SETFONT, (WPARAM)config.font, TRUE);

		return TRUE;
	}

	LRESULT __stdcall KeysHook(INT nCode, WPARAM wParam, LPARAM lParam)
	{
		if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
		{
			KBDLLHOOKSTRUCT* phs = (KBDLLHOOKSTRUCT*)lParam;
			if (phs->vkCode == VK_SNAPSHOT)
			{
				HWND hWnd = GetActiveWindow();
				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw && config.windowState == StateFullsreen)
				{
					ddraw->isTakeSnapshot = TRUE;
					if (ddraw)
						SetEvent(ddraw->hDrawEvent);
					MessageBeep(0);
					return TRUE;
				}
			}
		}

		return CallNextHookEx(OldKeysHook, nCode, wParam, lParam);
	}

	LRESULT __stdcall AboutApplicationProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_INITDIALOG:
		{
			SetWindowLong(hDlg, GWL_EXSTYLE, NULL);
			EnumChildWindows(hDlg, EnumChildProc, NULL);

			CHAR path[MAX_PATH];
			CHAR temp[100];

			GetModuleFileName(NULL, path, sizeof(path));

			DWORD hSize;
			DWORD verSize = GetFileVersionInfoSize(path, &hSize);

			if (verSize)
			{
				CHAR* verData = (CHAR*)MemoryAlloc(verSize);
				{
					if (GetFileVersionInfo(path, hSize, verSize, verData))
					{
						VOID* buffer;
						UINT size;
						if (VerQueryValue(verData, "\\", &buffer, &size) && size)
						{
							VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)buffer;

							GetDlgItemText(hDlg, IDC_VERSION, temp, sizeof(temp));
							StrPrint(path, temp, HIWORD(verInfo->dwProductVersionMS), LOWORD(verInfo->dwProductVersionMS), HIWORD(verInfo->dwProductVersionLS), LOWORD(verInfo->dwProductVersionLS));
							SetDlgItemText(hDlg, IDC_VERSION, path);
						}

						CHAR* copy = NULL;
						CHAR* company = NULL;

						DWORD* lpTranslate;
						if (VerQueryValue(verData, "\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &size) && size)
						{
							BOOL isTitle = FALSE;
							BOOL isCopyright = FALSE;
							BOOL isCompany = FALSE;

							DWORD count = size / sizeof(UINT);
							{
								DWORD* translate = lpTranslate;
								while (count && (!isTitle || !isCopyright))
								{
									if (!isTitle && StrPrint(path, "\\StringFileInfo\\%04x%04x\\%s", LOWORD(*translate), HIWORD(*translate), "FileDescription") &&
										VerQueryValue(verData, path, &buffer, &size) && size)
									{
										SetDlgItemText(hDlg, IDC_TITLE, (CHAR*)buffer);
										isTitle = TRUE;
									}

									if (!isCopyright && StrPrint(path, "\\StringFileInfo\\%04x%04x\\%s", LOWORD(*translate), HIWORD(*translate), "LegalCopyright") &&
										VerQueryValue(verData, path, &buffer, &size) && size)
									{
										copy = (CHAR*)buffer;
										isCopyright = TRUE;
									}

									if (!isCompany && StrPrint(path, "\\StringFileInfo\\%04x%04x\\%s", LOWORD(*translate), HIWORD(*translate), "CompanyName") &&
										VerQueryValue(verData, path, &buffer, &size) && size)
									{
										company = (CHAR*)buffer;
										isCompany = TRUE;
									}

									++translate;
									--count;
								}
							}

							if (isCopyright && isCompany)
							{
								CHAR text[100];
								StrPrint(text, "%s  %s", copy, company);
								SetDlgItemText(hDlg, IDC_COPYRIGHT, text);
							}
						}
					}
				}
				MemoryFree(verData);
			}

			GetDlgItemText(hDlg, IDC_LNK_EMAIL, temp, sizeof(temp));
			StrPrint(path, "<A HREF=\"mailto:%s\">%s</A>", temp, temp);
			SetDlgItemText(hDlg, IDC_LNK_EMAIL, path);

			break;
		}

		case WM_COMMAND:
		{
			if (wParam == IDC_BTN_OK)
				EndDialog(hDlg, TRUE);
			break;
		}

		default:
			break;
		}

		return DefWindowProc(hDlg, uMsg, wParam, lParam);
	}

	LRESULT __stdcall AboutWrapperProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_INITDIALOG:
		{
			SetWindowLong(hDlg, GWL_EXSTYLE, NULL);
			EnumChildWindows(hDlg, EnumChildProc, NULL);

			CHAR path[MAX_PATH];
			CHAR temp[100];

			GetModuleFileName(hDllModule, path, sizeof(path));

			DWORD hSize;
			DWORD verSize = GetFileVersionInfoSize(path, &hSize);

			if (verSize)
			{
				CHAR* verData = (CHAR*)MemoryAlloc(verSize);
				{
					if (GetFileVersionInfo(path, hSize, verSize, verData))
					{
						VOID* buffer;
						UINT size;
						if (VerQueryValue(verData, "\\", &buffer, &size) && size)
						{
							VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)buffer;
							
							GetDlgItemText(hDlg, IDC_VERSION, temp, sizeof(temp));
							StrPrint(path, temp, HIWORD(verInfo->dwProductVersionMS), LOWORD(verInfo->dwProductVersionMS), HIWORD(verInfo->dwProductVersionLS), LOWORD(verInfo->dwFileVersionLS));
							SetDlgItemText(hDlg, IDC_VERSION, path);
						}
					}
				}
				MemoryFree(verData);
			}

			if (GetDlgItemText(hDlg, IDC_LNK_EMAIL, temp, sizeof(temp)))
			{
				StrPrint(path, "<A HREF=\"mailto:%s\">%s</A>", temp, temp);
				SetDlgItemText(hDlg, IDC_LNK_EMAIL, path);
			}

			break;
		}

		case WM_NOTIFY:
		{
			if (((NMHDR*)lParam)->code == NM_CLICK && wParam == IDC_LNK_EMAIL)
			{
				NMLINK* pNMLink = (NMLINK*)lParam;
				LITEM iItem = pNMLink->item;

				CHAR url[MAX_PATH];
				StrToAnsi(url, pNMLink->item.szUrl, sizeof(url) - 1);

				SHELLEXECUTEINFO shExecInfo;
				MemoryZero(&shExecInfo, sizeof(SHELLEXECUTEINFO));
				shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
				shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
				shExecInfo.lpFile = url;
				shExecInfo.nShow = SW_SHOW;

				ShellExecuteEx(&shExecInfo);
			}

			break;
		}

		case WM_COMMAND:
		{
			if (wParam == IDC_BTN_OK)
				EndDialog(hDlg, TRUE);
			break;
		}

		default:
			break;
		}

		return DefWindowProc(hDlg, uMsg, wParam, lParam);
	}

	LRESULT __stdcall WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			if (config.isPlatinum || config.windowState == StateFullsreen)
			{
				PAINTSTRUCT paint;
				HDC hDc = BeginPaint(hWnd, &paint);
				if (hDc)
				{
					EndPaint(hWnd, &paint);
					return NULL;
				}

				return NULL;
			}
			else
				return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_ERASEBKGND:
		{
			if (config.windowState == StateFullsreen)
			{
				RECT rc;
				GetClientRect(hWnd, &rc);
				FillRect((HDC)wParam, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
				return TRUE;
			}
			return NULL;
		}

		case WM_WINDOWPOSCHANGED:
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				RECT rect;
				GetClientRect(hWnd, &rect);
				ddraw->viewport.width = rect.right;
				ddraw->viewport.height = rect.bottom;
				ddraw->viewport.refresh = TRUE;
				SetEvent(ddraw->hDrawEvent);
			}

			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		case WM_MOVE:
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
				SetEvent(ddraw->hDrawEvent);

			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		case WM_SIZE:
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				if (ddraw->hDraw && ddraw->hDraw != hWnd)
					SetWindowPos(ddraw->hDraw, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);

				ddraw->viewport.width = LOWORD(lParam);
				ddraw->viewport.height = HIWORD(lParam);
				ddraw->viewport.refresh = TRUE;
				SetEvent(ddraw->hDrawEvent);
			}

			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		case WM_GETMINMAXINFO:
		{
			DWORD style = GetWindowLong(hWnd, GWL_STYLE);

			RECT min = { 0, 0, MIN_WIDTH, MIN_HEIGHT };
			AdjustWindowRect(&min, style, config.windowState == StateWindowed);

			RECT max = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
			AdjustWindowRect(&max, style, config.windowState == StateWindowed);

			MINMAXINFO* mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = min.right - min.left;
			mmi->ptMinTrackSize.y = min.bottom - min.top;
			mmi->ptMaxTrackSize.x = max.right - max.left;
			mmi->ptMaxTrackSize.y = max.bottom - max.top;

			return NULL;
		}

		case WM_ACTIVATEAPP:
		{
			if (config.windowState == StateFullsreen)
			{
				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw)
				{
					if ((BOOL)wParam)
						ddraw->RenderStart();
					else
						ddraw->RenderStop();
				}
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			if (!(HIWORD(lParam) & KF_ALTDOWN))
			{
				if (config.keys.fpsCounter && config.keys.fpsCounter + VK_F1 - 1 == wParam)
				{
					switch (fpsState)
					{
					case FpsNormal:
						fpsState = FpsBenchmark;
						break;
					case FpsBenchmark:
						fpsState = FpsDisabled;
						break;
					default:
						fpsState = FpsNormal;
						break;
					}

					isFpsChanged = TRUE;
					OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
					if (ddraw)
						SetEvent(ddraw->hDrawEvent);
					return NULL;
				}
				else if (config.keys.windowedMode && config.keys.windowedMode + VK_F1 - 1 == wParam)
				{
					return WindowProc(hWnd, WM_COMMAND, IDM_RES_FULL_SCREEN, NULL);
					return NULL;
				}
				else if (config.keys.imageFilter && config.keys.imageFilter + VK_F1 - 1 == wParam)
				{
					switch (config.image.filter)
					{
					case FilterNearest:
						config.image.filter = FilterLinear;
						break;

					case FilterLinear:
						config.image.filter = glVersion >= GL_VER_2_0 ? FilterHermite : FilterNearest;
						break;

					case FilterHermite:
						config.image.filter = glVersion >= GL_VER_2_0 ? FilterCubic : FilterNearest;
						break;

					default:
						config.image.filter = FilterNearest;
						break;
					}

					FilterChanged(hWnd);

					return NULL;
				}
				else if (config.keys.aspectRatio && config.keys.aspectRatio + VK_F1 - 1 == wParam)
				{
					WindowProc(hWnd, WM_COMMAND, IDM_ASPECT_RATIO, NULL);
					return NULL;
				}
				else if (config.keys.vSync && config.keys.vSync + VK_F1 - 1 == wParam)
				{
					return WindowProc(hWnd, WM_COMMAND, IDM_VSYNC, NULL);
					return NULL;
				}
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_MOUSEMOVE:
		case WM_MOUSEWHEEL:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				ddraw->ScaleMouse(&p);
				lParam = MAKELONG(p.x, p.y);
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_SYSCOMMAND:
		{
			if (config.isPlatinum)
				return DefWindowProc(hWnd, uMsg, wParam, lParam);
			else
				return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_COMMAND:
		{
			switch (wParam)
			{
			case IDM_FILE_QUIT:
			{
				SendMessage(hWnd, WM_CLOSE, NULL, NULL);
				return NULL;
			}

			case IDM_HELP_ABOUT_APPLICATION:
			case IDM_HELP_ABOUT_WRAPPER:
			{
				ULONG_PTR cookie = NULL;
				if (hActCtx && hActCtx != INVALID_HANDLE_VALUE && !ActivateActCtxC(hActCtx, &cookie))
					cookie = NULL;

				LPCSTR id;
				DLGPROC proc;
				if (wParam == IDM_HELP_ABOUT_APPLICATION)
				{
					id = MAKEINTRESOURCE(IDD_ABOUT_APPLICATION);
					proc = (DLGPROC)AboutApplicationProc;
				}
				else
				{
					id = MAKEINTRESOURCE(cookie ? IDD_ABOUT_WRAPPER : IDD_ABOUT_WRAPPER_OLD);
					proc = (DLGPROC)AboutWrapperProc;
				}

				DialogBoxParam(hDllModule, id, hWnd, proc, NULL);

				if (cookie)
					DeactivateActCtxC(0, cookie);

				SetForegroundWindow(hWnd);
				return NULL;
			}

			case IDM_RES_FULL_SCREEN:
			{
				CallWindowProc(OldWindowProc, hWnd, WM_KEYUP, VK_F4, NULL);
				return NULL;
			}

			case IDM_ASPECT_RATIO:
			{
				config.image.aspect = !config.image.aspect;
				Config::Set(CONFIG_WRAPPER, "ImageAspect", config.image.aspect);
				CheckMenu(MenuAspect);

				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw)
				{
					ddraw->viewport.refresh = TRUE;
					SetEvent(ddraw->hDrawEvent);
				}

				return NULL;
			}

			case IDM_VSYNC:
			{
				config.image.vSync = !config.image.vSync;
				Config::Set(CONFIG_WRAPPER, "ImageVSync", config.image.vSync);
				CheckMenu(MenuVSync);

				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw)
					SetEvent(ddraw->hDrawEvent);

				return NULL;
			}

			case IDM_FILT_OFF:
			{
				config.image.filter = FilterNearest;
				FilterChanged(hWnd);
				return NULL;
			}

			case IDM_FILT_LINEAR:
			{
				config.image.filter = FilterLinear;
				FilterChanged(hWnd);
				return NULL;
			}

			case IDM_FILT_HERMITE:
			{
				config.image.filter = glVersion >= GL_VER_2_0 ? FilterHermite : FilterLinear;
				FilterChanged(hWnd);
				return NULL;
			}

			case IDM_FILT_CUBIC:
			{
				config.image.filter = glVersion >= GL_VER_2_0 ? FilterCubic : FilterLinear;
				FilterChanged(hWnd);
				return NULL;
			}

			default:
				return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
			}
		}

		default:
			if (uMsg == config.msgMenu)
			{
				CheckMenu();
				return NULL;
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}
	}

	LRESULT __stdcall PanelProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_NCHITTEST:
		case WM_SETCURSOR:

		case WM_MOUSEMOVE:
		case WM_MOUSEWHEEL:

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:

		case WM_SYSCOMMAND:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
			return WindowProc(GetParent(hWnd), uMsg, wParam, lParam);

		default:
			return CallWindowProc(OldPanelProc, hWnd, uMsg, wParam, lParam);
		}
	}

	VOID __fastcall SetCaptureKeys(BOOL state)
	{
		if (state)
		{
			if (!OldKeysHook)
				OldKeysHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeysHook, hDllModule, NULL);
		}
		else
		{
			if (OldKeysHook && UnhookWindowsHookEx(OldKeysHook))
				OldKeysHook = NULL;
		}
	}

	VOID __fastcall SetCaptureWindow(HWND hWnd)
	{
		OldWindowProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (LPARAM)WindowProc);
	}

	VOID __fastcall SetCapturePanel(HWND hWnd)
	{
		OldPanelProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (LPARAM)PanelProc);
	}
}