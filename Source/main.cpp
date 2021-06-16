#include <WinSock2.h>
#include <iphlpapi.h>
#include <Windows.h>
#include <Psapi.h>
#include <stdio.h>
#include <assert.h>
#include <Shlwapi.h>
#include <string>
#include "Server/Server.h"
#include "Client/Client.h"

#include <iostream>

using namespace std;

std::wstring GetDockerGatewayIP()
{
	PIP_ADAPTER_ADDRESSES pAdapterAddresses = NULL;
	ULONG Size = 0;

	ULONG Status = GetAdaptersAddresses(
		AF_INET,
		GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_UNICAST | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST,
		NULL,
		NULL,
		&Size
	);

	if (Status == ERROR_BUFFER_OVERFLOW)
	{
		pAdapterAddresses = (PIP_ADAPTER_ADDRESSES)malloc(Size);

		Status = GetAdaptersAddresses(
			AF_INET,
			GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_UNICAST | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_INCLUDE_PREFIX,
			NULL,
			pAdapterAddresses,
			&Size
		);
	}

	if (Status == ERROR_SUCCESS)
	{
		for (PIP_ADAPTER_ADDRESSES pCurAddress = pAdapterAddresses; pCurAddress; pCurAddress = pCurAddress->Next)
		{
			if (!_wcsicmp(pCurAddress->FriendlyName, L"vEthernet (nat)"))
			{
				sockaddr_in* addr = (sockaddr_in*)pCurAddress->FirstPrefix->Next->Address.lpSockaddr;

				return std::to_wstring(addr->sin_addr.S_un.S_un_b.s_b1)
					+ L"." + std::to_wstring(addr->sin_addr.S_un.S_un_b.s_b2)
					+ L"." + std::to_wstring(addr->sin_addr.S_un.S_un_b.s_b3)
					+ L"." + std::to_wstring(addr->sin_addr.S_un.S_un_b.s_b4);
			}
		}
	}

	return std::wstring();
}

void StartSelfClient()
{
	TCHAR Filename[MAX_PATH];

	GetModuleFileNameEx(
		GetCurrentProcess(),
		NULL,
		Filename,
		sizeof(Filename)
	);

	PathRemoveFileSpec(Filename);

	std::wstring CmdLine;
	CmdLine = L"docker run --rm -ti --isolation process -v \"";
	CmdLine += Filename;
	CmdLine += L"\":C:\\Sandbox:ro ";
	CmdLine += L"mcr.microsoft.com/windows:20H2";
	CmdLine += L" C:\\Sandbox\\Sandbox client";
	CmdLine += L" " + GetDockerGatewayIP();

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));

	si.cb = sizeof(si);

	if (!CreateProcess(
		NULL,
		(LPWSTR)CmdLine.c_str(),
		NULL,
		NULL,
		FALSE,
		CREATE_NO_WINDOW,
		NULL,
		NULL,
		&si,
		&pi
	))
	{
		throw GetLastError();
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

extern "C" int main(int argc, char *argv[])
{
	int nArgs = 0;

	LPWSTR* lpArgs = CommandLineToArgvW(
		GetCommandLineW(),
		&nArgs
	);

	cout << "Init" << endl;

	if (nArgs > 2 && lpArgs && !_wcsicmp(lpArgs[1], L"client"))
	{
		try
		{
			cout << "Initializing client..." << endl;
			SandboxInitClient(lpArgs[2]);
			cout << "Exit" << endl;
		}
		catch (long ErrorCode)
		{
			cout << "Failed to init client (error " << ErrorCode << ")" << endl;
			return 0;
		}
	}
	else
	{
		try
		{
			SandboxInitServer();
			//StartSelfClient();
			SandboxInitVideo();
		}
		catch (long ErrorCode)
		{
			return 0;
		}
	}

	if (lpArgs)
	{
		LocalFree(lpArgs);
	}

	return 0;
}
