#include <Windows.h>
#include <AclAPI.h>
#include <shellapi.h>
#include <Psapi.h>
#include <stdio.h>

#include <iostream>

using namespace std;

static bool AdjustSingleTokenPrivilege(HANDLE TokenHandle, LPCTSTR lpName, DWORD dwAttributes)
{
	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = dwAttributes;
	if (!LookupPrivilegeValue(NULL, lpName, &(tp.Privileges[0].Luid)))
		return false;
	if (!AdjustTokenPrivileges(TokenHandle, FALSE, &tp, 0, NULL, NULL))
		return false;
	return true;
}

void TweakWindowStation()
{
	HWINSTA hWinSta = OpenWindowStation(TEXT("WinSta0"), FALSE, GENERIC_EXECUTE | WRITE_DAC);
	if (!hWinSta)
	{
		cout << "Opening WinSta0 failed (err 0x" << hex << GetLastError() << ")" << endl;
		return;
	}

	HWINSTA hOldWinsta = GetProcessWindowStation();
	SetProcessWindowStation(hWinSta);

	HDESK hDesktop = OpenDesktop(TEXT("Default"), 0, FALSE, GENERIC_READ | WRITE_DAC);
	if (!hDesktop)
	{
		cout << "Opening Default desktop failed (err 0x" << hex << GetLastError() << ")" << endl;
		return;
	}

	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;

	PSID pEveryoneSID = NULL;
	AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSID);

	EXPLICIT_ACCESS ExplicitAccess;
	memset(&ExplicitAccess, 0, sizeof(ExplicitAccess));
	ExplicitAccess.grfAccessPermissions = GENERIC_ALL;
	ExplicitAccess.grfAccessMode = GRANT_ACCESS;
	ExplicitAccess.grfInheritance = NO_INHERITANCE;
	ExplicitAccess.Trustee.pMultipleTrustee = NULL;
	ExplicitAccess.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
	ExplicitAccess.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ExplicitAccess.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ExplicitAccess.Trustee.ptstrName = (TCHAR*)pEveryoneSID;

	PACL Dacl;
	DWORD Status = SetEntriesInAcl(
		1,
		&ExplicitAccess,
		NULL,
		&Dacl
	);

	if (Status == ERROR_SUCCESS)
	{
		HANDLE hToken;
		OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

		if (AdjustSingleTokenPrivilege(hToken, SE_SECURITY_NAME, SE_PRIVILEGE_ENABLED))
		{
			// Set all permissions for the window station
			Status = SetSecurityInfo(
				hWinSta,
				SE_KERNEL_OBJECT,
				DACL_SECURITY_INFORMATION,
				NULL,
				NULL,
				Dacl,
				NULL
			);

			if (Status)
			{
				cout << "SetSecurityInfo returned " << hex << Status << endl;
			}

			// Set all permissions for the desktop object
			Status = SetSecurityInfo(
				hDesktop,
				SE_KERNEL_OBJECT,
				DACL_SECURITY_INFORMATION,
				NULL,
				NULL,
				Dacl,
				NULL
			);

			if (Status)
			{
				cout << "SetSecurityInfo returned " << hex << Status << endl;
			}
		}
		else
		{
			cout << "AdjustTokenPrivileges failed (err 0x" << hex << GetLastError() << ")" << endl;
		}
	}
	else
	{
		cout << "SetEntriesInAcl returned " << hex << Status << endl;
	}

	LocalFree(Dacl);

	SetProcessWindowStation(hOldWinsta);
	CloseHandle(hWinSta);
	CloseHandle(hDesktop);
}

void AssignWindowStation()
{
	HWINSTA hWinSta = CreateWindowStation(TEXT("WinSta0"), 0, GENERIC_ALL, NULL); // OpenWindowStation(TEXT("WinSta0"), FALSE, GENERIC_ALL);
	if (!hWinSta)
	{
		cout << "Opening WinSta0 failed (err 0x" << hex << GetLastError() << ")" << endl;
		return;
	}

	HWINSTA hOldWinsta = GetProcessWindowStation();
	if (!SetProcessWindowStation(hWinSta))
	{
		cout << "SetProcessWindowStation failed (err 0x" << hex << GetLastError() << ")" << endl;
		return;
	}

	HDESK hDesktop = CreateDesktop(TEXT("Default"), NULL, NULL, 0, GENERIC_ALL, NULL);
	if (!hDesktop)
	{
		cout << "Opening hDesktop failed (err 0x" << hex << GetLastError() << ")" << endl;
		return;
	}

	HDESK hOldDesktop = GetThreadDesktop(GetCurrentThreadId());
	if (!SetThreadDesktop(hDesktop))
	{
		cout << "SetThreadDesktop failed (err 0x" << hex << GetLastError() << ")" << endl;
		return;
	}

	if (hOldWinsta)
	{
		// Close previous window station
		CloseHandle(hOldWinsta);
	}

	if (hOldDesktop)
	{
		// Close previous desktop
		CloseHandle(hOldDesktop);
	}

	USEROBJECTFLAGS Flags{ 0 };
	Flags.dwFlags = WSF_VISIBLE;
	if (!SetUserObjectInformation(hWinSta, UOI_FLAGS, &Flags, sizeof(Flags)))
	{
		cout << "SetUserObjectInformation failed (err 0x" << hex << GetLastError() << ")" << endl;
	}

	// Switch to the Default desktop
	if (!SwitchDesktop(hDesktop))
	{
		cout << "SwitchDesktop failed (err 0x" << hex << GetLastError() << ")" << endl;
	}
}

void StartProgram()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));

	si.cb = sizeof(si);

	HANDLE hDesktop = GetThreadDesktop(GetCurrentThreadId());
	HANDLE hWinSta = GetProcessWindowStation();

	DWORD TotalStringSize = 0;

	DWORD nWinStaNameSize = 0;
	GetUserObjectInformation(hWinSta, UOI_NAME, NULL, 0, &nWinStaNameSize);
	TotalStringSize += nWinStaNameSize;

	DWORD nDesktopNameSize = 0;
	GetUserObjectInformation(hDesktop, UOI_NAME, NULL, 0, &nDesktopNameSize);
	TotalStringSize += nDesktopNameSize;

	TCHAR* lpDesktop = new TCHAR[TotalStringSize / sizeof(TCHAR)];

	const DWORD nWinStaNameLength = nWinStaNameSize / sizeof(TCHAR);
	GetUserObjectInformation(hWinSta, UOI_NAME, lpDesktop, nWinStaNameSize, &nWinStaNameSize);
	lpDesktop[nWinStaNameLength - 1] = '\\';

	GetUserObjectInformation(hDesktop, UOI_NAME, lpDesktop + nWinStaNameLength, nDesktopNameSize, &nDesktopNameSize);

	wcout << L"Desktop: " << lpDesktop << endl;

	si.lpDesktop = lpDesktop;

	TCHAR szCommand[] = TEXT("cmd.exe");

	BOOL Result = CreateProcess(
		NULL,
		szCommand,
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&si,
		&pi
	);

	delete[] lpDesktop;

	if (!Result)
	{
		cout << "Process creation failed: error " << GetLastError() << endl;
		return;
	}

	cout << "Waiting for input..." << endl;
	WaitForInputIdle(pi.hProcess, INFINITE);

	// Wait until a window is drawn
	DWORD dwCount = 0;
	while (!GetTopWindow(NULL) && dwCount < 10000)
	{
		dwCount += 100;
		Sleep(100);
	}

	cout << "Done" << endl;
}
