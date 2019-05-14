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

HMODULE hDllModule;
HANDLE hActCtx;

CREATEACTCTXA CreateActCtxC;
RELEASEACTCTX ReleaseActCtxC;
ACTIVATEACTCTX ActivateActCtxC;
DEACTIVATEACTCTX DeactivateActCtxC;

FARPROC _accept;
FARPROC _bind;
FARPROC _closesocket;
FARPROC _connect;
FARPROC _getpeername;
FARPROC _getsockname;
FARPROC _getsockopt;
FARPROC _htonl;
FARPROC _htons;
FARPROC _inet_addr;
FARPROC _inet_ntoa;
FARPROC _ioctlsocket;
FARPROC _listen;
FARPROC _ntohl;
FARPROC _ntohs;
FARPROC _recv;
FARPROC _recvfrom;
FARPROC _select;
FARPROC _send;
FARPROC _sendto;
FARPROC _setsockopt;
FARPROC _shutdown;
FARPROC _socket;
FARPROC _MigrateWinsockConfiguration;
FARPROC _gethostbyaddr;
FARPROC _gethostbyname;
FARPROC _getprotobyname;
FARPROC _getprotobynumber;
FARPROC _getservbyname;
FARPROC _getservbyport;
FARPROC _gethostname;
FARPROC _WSAAsyncSelect;
FARPROC _WSAAsyncGetHostByAddr;
FARPROC _WSAAsyncGetHostByName;
FARPROC _WSAAsyncGetProtoByNumber;
FARPROC _WSAAsyncGetProtoByName;
FARPROC _WSAAsyncGetServByPort;
FARPROC _WSAAsyncGetServByName;
FARPROC _WSACancelAsyncRequest;
FARPROC _WSASetBlockingHook;
FARPROC _WSAUnhookBlockingHook;
FARPROC _WSAGetLastError;
FARPROC _WSASetLastError;
FARPROC _WSACancelBlockingCall;
FARPROC _WSAIsBlocking;
FARPROC _WSAStartup;
FARPROC _WSACleanup;
FARPROC _WSAFDIsSet;
FARPROC _WEP;
FARPROC _WSApSetPostRoutine;
FARPROC _inet_network;
FARPROC _getnetbyname;
FARPROC _rcmd;
FARPROC _rexec;
FARPROC _rresvport;
FARPROC _sethostname;
FARPROC _dn_expand;
FARPROC _WSARecvEx;
FARPROC _s_perror;
FARPROC _GetAddressByNameA;
FARPROC _GetAddressByNameW;
FARPROC _EnumProtocolsA;
FARPROC _EnumProtocolsW;
FARPROC _GetTypeByNameA;
FARPROC _GetTypeByNameW;
FARPROC _GetNameByTypeA;
FARPROC _GetNameByTypeW;
FARPROC _SetServiceA;
FARPROC _SetServiceW;
FARPROC _GetServiceA;
FARPROC _GetServiceW;
FARPROC _NPLoadNameSpaces;
FARPROC _TransmitFile;
FARPROC _AcceptEx;
FARPROC _GetAcceptExSockaddrs;

#define LIBEXP VOID __declspec(naked,nothrow) __stdcall

LIBEXP ex_accept() { _asm { JMP _accept } }
LIBEXP ex_bind() { _asm { JMP _bind } }
LIBEXP ex_closesocket() { _asm { JMP _closesocket } }
LIBEXP ex_connect() { _asm { JMP _connect } }
LIBEXP ex_getpeername() { _asm { JMP _getpeername } }
LIBEXP ex_getsockname() { _asm { JMP _getsockname } }
LIBEXP ex_getsockopt() { _asm { JMP _getsockopt } }
LIBEXP ex_htonl() { _asm { JMP _htonl } }
LIBEXP ex_htons() { _asm { JMP _htons } }
LIBEXP ex_inet_addr() { _asm { JMP _inet_addr } }
LIBEXP ex_inet_ntoa() { _asm { JMP _inet_ntoa } }
LIBEXP ex_ioctlsocket() { _asm { JMP _ioctlsocket } }
LIBEXP ex_listen() { _asm { JMP _listen } }
LIBEXP ex_ntohl() { _asm { JMP _ntohl } }
LIBEXP ex_ntohs() { _asm { JMP _ntohs } }
LIBEXP ex_recv() { _asm { JMP _recv } }
LIBEXP ex_recvfrom() { _asm { JMP _recvfrom } }
LIBEXP ex_select() { _asm { JMP _select } }
LIBEXP ex_send() { _asm { JMP _send } }
LIBEXP ex_sendto() { _asm { JMP _sendto } }
LIBEXP ex_setsockopt() { _asm { JMP _setsockopt } }
LIBEXP ex_shutdown() { _asm { JMP _shutdown } }
LIBEXP ex_socket() { _asm { JMP _socket } }
LIBEXP ex_MigrateWinsockConfiguration() { _asm { JMP _MigrateWinsockConfiguration } }
LIBEXP ex_gethostbyaddr() { _asm { JMP _gethostbyaddr } }
LIBEXP ex_gethostbyname() { _asm { JMP _gethostbyname } }
LIBEXP ex_getprotobyname() { _asm { JMP _getprotobyname } }
LIBEXP ex_getprotobynumber() { _asm { JMP _getprotobynumber } }
LIBEXP ex_getservbyname() { _asm { JMP _getservbyname } }
LIBEXP ex_getservbyport() { _asm { JMP _getservbyport } }
LIBEXP ex_gethostname() { _asm { JMP _gethostname } }
LIBEXP ex_WSAAsyncSelect() { _asm { JMP _WSAAsyncSelect } }
LIBEXP ex_WSAAsyncGetHostByAddr() { _asm { JMP _WSAAsyncGetHostByAddr } }
LIBEXP ex_WSAAsyncGetHostByName() { _asm { JMP _WSAAsyncGetHostByName } }
LIBEXP ex_WSAAsyncGetProtoByNumber() { _asm { JMP _WSAAsyncGetProtoByNumber } }
LIBEXP ex_WSAAsyncGetProtoByName() { _asm { JMP _WSAAsyncGetProtoByName } }
LIBEXP ex_WSAAsyncGetServByPort() { _asm { JMP _WSAAsyncGetServByPort } }
LIBEXP ex_WSAAsyncGetServByName() { _asm { JMP _WSAAsyncGetServByName } }
LIBEXP ex_WSACancelAsyncRequest() { _asm { JMP _WSACancelAsyncRequest } }
LIBEXP ex_WSASetBlockingHook() { _asm { JMP _WSASetBlockingHook } }
LIBEXP ex_WSAUnhookBlockingHook() { _asm { JMP _WSAUnhookBlockingHook } }
LIBEXP ex_WSAGetLastError() { _asm { JMP _WSAGetLastError } }
LIBEXP ex_WSASetLastError() { _asm { JMP _WSASetLastError } }
LIBEXP ex_WSACancelBlockingCall() { _asm { JMP _WSACancelBlockingCall } }
LIBEXP ex_WSAIsBlocking() { _asm { JMP _WSAIsBlocking } }
LIBEXP ex_WSAStartup() { _asm { JMP _WSAStartup } }
LIBEXP ex_WSACleanup() { _asm { JMP _WSACleanup } }
LIBEXP ex_WSAFDIsSet() { _asm { JMP _WSAFDIsSet } }
LIBEXP ex_WEP() { _asm { JMP _WEP } }
LIBEXP ex_WSApSetPostRoutine() { _asm { JMP _WSApSetPostRoutine } }
LIBEXP ex_inet_network() { _asm { JMP _inet_network } }
LIBEXP ex_getnetbyname() { _asm { JMP _getnetbyname } }
LIBEXP ex_rcmd() { _asm { JMP _rcmd } }
LIBEXP ex_rexec() { _asm { JMP _rexec } }
LIBEXP ex_rresvport() { _asm { JMP _rresvport } }
LIBEXP ex_sethostname() { _asm { JMP _sethostname } }
LIBEXP ex_dn_expand() { _asm { JMP _dn_expand } }
LIBEXP ex_WSARecvEx() { _asm { JMP _WSARecvEx } }
LIBEXP ex_s_perror() { _asm { JMP _s_perror } }
LIBEXP ex_GetAddressByNameA() { _asm { JMP _GetAddressByNameA } }
LIBEXP ex_GetAddressByNameW() { _asm { JMP _GetAddressByNameW } }
LIBEXP ex_EnumProtocolsA() { _asm { JMP _EnumProtocolsA } }
LIBEXP ex_EnumProtocolsW() { _asm { JMP _EnumProtocolsW } }
LIBEXP ex_GetTypeByNameA() { _asm { JMP _GetTypeByNameA } }
LIBEXP ex_GetTypeByNameW() { _asm { JMP _GetTypeByNameW } }
LIBEXP ex_GetNameByTypeA() { _asm { JMP _GetNameByTypeA } }
LIBEXP ex_GetNameByTypeW() { _asm { JMP _GetNameByTypeW } }
LIBEXP ex_SetServiceA() { _asm { JMP _SetServiceA } }
LIBEXP ex_SetServiceW() { _asm { JMP _SetServiceW } }
LIBEXP ex_GetServiceA() { _asm { JMP _GetServiceA } }
LIBEXP ex_GetServiceW() { _asm { JMP _GetServiceW } }
LIBEXP ex_NPLoadNameSpaces() { _asm { JMP _NPLoadNameSpaces } }
LIBEXP ex_TransmitFile() { _asm { JMP _TransmitFile } }
LIBEXP ex_AcceptEx() { _asm { JMP _AcceptEx } }
LIBEXP ex_GetAcceptExSockaddrs() { _asm { JMP _GetAcceptExSockaddrs } }

DOUBLE __fastcall MathRound(DOUBLE number)
{
	DOUBLE floorVal = MathFloor(number);
	return floorVal + 0.5f > number ? floorVal : MathCeil(number);
}

typedef VOID*(__cdecl *MALLOC)(size_t);
typedef VOID(__cdecl *FREE)(VOID*);

struct Aligned
{
	Aligned* last;
	VOID* data;
	VOID* block;
} *alignedList;

VOID* __fastcall AlignedAlloc(size_t size)
{
	Aligned* entry = (Aligned*)MemoryAlloc(sizeof(Aligned));
	entry->last = alignedList;
	alignedList = entry;

	entry->data = MemoryAlloc(size + 16);
	entry->block = (VOID*)(((DWORD)entry->data + 16) & 0xFFFFFFF0);

	return entry->block;
}

VOID __fastcall AlignedFree(VOID* block)
{
	Aligned* entry = alignedList;
	if (entry)
	{
		if (entry->block == block)
		{
			Aligned* last = entry->last;
			MemoryFree(entry->data);
			MemoryFree(entry);
			alignedList = last;
			return;
		}
		else while (entry->last)
		{
			if (entry->last->block == block)
			{
				Aligned* last = entry->last->last;
				MemoryFree(entry->last->data);
				MemoryFree(entry->last);
				entry->last = last;
				return;
			}

			entry = entry->last;
		}
	}
}

VOID LoadKernel32()
{
	HMODULE hLib = GetModuleHandle("KERNEL32.dll");
	if (hLib)
	{
		CreateActCtxC = (CREATEACTCTXA)GetProcAddress(hLib, "CreateActCtxA");
		ReleaseActCtxC = (RELEASEACTCTX)GetProcAddress(hLib, "ReleaseActCtx");
		ActivateActCtxC = (ACTIVATEACTCTX)GetProcAddress(hLib, "ActivateActCtx");
		DeactivateActCtxC = (DEACTIVATEACTCTX)GetProcAddress(hLib, "DeactivateActCtx");
	}
}

VOID LoadWSock32()
{
	CHAR dir[MAX_PATH];
	if (GetSystemDirectory(dir, MAX_PATH))
	{
		StrCat(dir, "\\WSOCK32.dll");
		HMODULE hLib = LoadLibrary(dir);
		if (hLib)
		{
			_accept = GetProcAddress(hLib, "accept");
			_bind = GetProcAddress(hLib, "bind");
			_closesocket = GetProcAddress(hLib, "closesocket");
			_connect = GetProcAddress(hLib, "connect");
			_getpeername = GetProcAddress(hLib, "getpeername");
			_getsockname = GetProcAddress(hLib, "getsockname");
			_getsockopt = GetProcAddress(hLib, "getsockopt");
			_htonl = GetProcAddress(hLib, "htonl");
			_htons = GetProcAddress(hLib, "htons");
			_inet_addr = GetProcAddress(hLib, "inet_addr");
			_inet_ntoa = GetProcAddress(hLib, "inet_ntoa");
			_ioctlsocket = GetProcAddress(hLib, "ioctlsocket");
			_listen = GetProcAddress(hLib, "listen");
			_ntohl = GetProcAddress(hLib, "ntohl");
			_ntohs = GetProcAddress(hLib, "ntohs");
			_recv = GetProcAddress(hLib, "recv");
			_recvfrom = GetProcAddress(hLib, "recvfrom");
			_select = GetProcAddress(hLib, "select");
			_send = GetProcAddress(hLib, "send");
			_sendto = GetProcAddress(hLib, "sendto");
			_setsockopt = GetProcAddress(hLib, "setsockopt");
			_shutdown = GetProcAddress(hLib, "shutdown");
			_socket = GetProcAddress(hLib, "socket");
			_MigrateWinsockConfiguration = GetProcAddress(hLib, "MigrateWinsockConfiguration");
			_gethostbyaddr = GetProcAddress(hLib, "gethostbyaddr");
			_gethostbyname = GetProcAddress(hLib, "gethostbyname");
			_getprotobyname = GetProcAddress(hLib, "getprotobyname");
			_getprotobynumber = GetProcAddress(hLib, "getprotobynumber");
			_getservbyname = GetProcAddress(hLib, "getservbyname");
			_getservbyport = GetProcAddress(hLib, "getservbyport");
			_gethostname = GetProcAddress(hLib, "gethostname");
			_WSAAsyncSelect = GetProcAddress(hLib, "WSAAsyncSelect");
			_WSAAsyncGetHostByAddr = GetProcAddress(hLib, "WSAAsyncGetHostByAddr");
			_WSAAsyncGetHostByName = GetProcAddress(hLib, "WSAAsyncGetHostByName");
			_WSAAsyncGetProtoByNumber = GetProcAddress(hLib, "WSAAsyncGetProtoByNumber");
			_WSAAsyncGetProtoByName = GetProcAddress(hLib, "WSAAsyncGetProtoByName");
			_WSAAsyncGetServByPort = GetProcAddress(hLib, "WSAAsyncGetServByPort");
			_WSAAsyncGetServByName = GetProcAddress(hLib, "WSAAsyncGetServByName");
			_WSACancelAsyncRequest = GetProcAddress(hLib, "WSACancelAsyncRequest");
			_WSASetBlockingHook = GetProcAddress(hLib, "WSASetBlockingHook");
			_WSAUnhookBlockingHook = GetProcAddress(hLib, "WSAUnhookBlockingHook");
			_WSAGetLastError = GetProcAddress(hLib, "WSAGetLastError");
			_WSASetLastError = GetProcAddress(hLib, "WSASetLastError");
			_WSACancelBlockingCall = GetProcAddress(hLib, "WSACancelBlockingCall");
			_WSAIsBlocking = GetProcAddress(hLib, "WSAIsBlocking");
			_WSAStartup = GetProcAddress(hLib, "WSAStartup");
			_WSACleanup = GetProcAddress(hLib, "WSACleanup");
			_WSAFDIsSet = GetProcAddress(hLib, "__WSAFDIsSet");
			_WEP = GetProcAddress(hLib, "WEP");
			_WSApSetPostRoutine = GetProcAddress(hLib, "WSApSetPostRoutine");
			_inet_network = GetProcAddress(hLib, "inet_network");
			_getnetbyname = GetProcAddress(hLib, "getnetbyname");
			_rcmd = GetProcAddress(hLib, "rcmd");
			_rexec = GetProcAddress(hLib, "rexec");
			_rresvport = GetProcAddress(hLib, "rresvport");
			_sethostname = GetProcAddress(hLib, "sethostname");
			_dn_expand = GetProcAddress(hLib, "dn_expand");
			_WSARecvEx = GetProcAddress(hLib, "WSARecvEx");
			_s_perror = GetProcAddress(hLib, "s_perror");
			_GetAddressByNameA = GetProcAddress(hLib, "GetAddressByNameA");
			_GetAddressByNameW = GetProcAddress(hLib, "GetAddressByNameW");
			_EnumProtocolsA = GetProcAddress(hLib, "EnumProtocolsA");
			_EnumProtocolsW = GetProcAddress(hLib, "EnumProtocolsW");
			_GetTypeByNameA = GetProcAddress(hLib, "GetTypeByNameA");
			_GetTypeByNameW = GetProcAddress(hLib, "GetTypeByNameW");
			_GetNameByTypeA = GetProcAddress(hLib, "GetNameByTypeA");
			_GetNameByTypeW = GetProcAddress(hLib, "GetNameByTypeW");
			_SetServiceA = GetProcAddress(hLib, "SetServiceA");
			_SetServiceW = GetProcAddress(hLib, "SetServiceW");
			_GetServiceA = GetProcAddress(hLib, "GetServiceA");
			_GetServiceW = GetProcAddress(hLib, "GetServiceW");
			_NPLoadNameSpaces = GetProcAddress(hLib, "NPLoadNameSpaces");
			_TransmitFile = GetProcAddress(hLib, "TransmitFile");
			_AcceptEx = GetProcAddress(hLib, "AcceptEx");
			_GetAcceptExSockaddrs = GetProcAddress(hLib, "GetAcceptExSockaddrs");
		}
	}
}