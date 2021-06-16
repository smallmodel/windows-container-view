#include "Client.h"
#include "Init.h"
#include "../Interface/sandbox_h.h"
#include "printwnd.h"
#include <rpc.h>
#include <stdio.h>
#include <stdexcept>

#include <iostream>

using namespace std;

class RpcStream : public IStream
{

public:
	virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, _Out_opt_ ULARGE_INTEGER* plibNewPosition)
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize)
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual HRESULT STDMETHODCALLTYPE CopyTo(_In_ IStream* pstm, ULARGE_INTEGER cb, _Out_opt_ ULARGE_INTEGER* pcbRead, _Out_opt_ ULARGE_INTEGER* pcbWritten)
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags)
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual HRESULT STDMETHODCALLTYPE Revert(void)
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual HRESULT STDMETHODCALLTYPE Stat(__RPC__out STATSTG* pstatstg, DWORD grfStatFlag)
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual HRESULT STDMETHODCALLTYPE Clone(__RPC__deref_out_opt IStream** ppstm)
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual HRESULT STDMETHODCALLTYPE Read(_Out_writes_bytes_to_(cb, *pcbRead) void* pv, _In_ ULONG cb, _Out_opt_ ULONG* pcbRead)
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual HRESULT STDMETHODCALLTYPE Write(_In_reads_bytes_(cb) const void* pv, _In_ ULONG cb, _Out_opt_ ULONG* pcbWritten)
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual ULONG STDMETHODCALLTYPE AddRef()
	{
		return 0;
	}

	virtual ULONG STDMETHODCALLTYPE Release()
	{
		return 0;
	}
};

BOOL CALLBACK WndCountProc(
	HWND hwnd,
	LPARAM lParam
)
{
	DWORD* pdwCount = (DWORD*)lParam;

	if (IsWindowVisible(hwnd))
	{
		*pdwCount = *pdwCount + 1;
	}

	return TRUE;
}

Gdiplus::Bitmap* DrawFinishedText()
{
	RECT rect;
	GetWindowRect(GetDesktopWindow(), &rect);

	HDC hDC = GetDC(NULL);

	Gdiplus::Graphics* graphics = new Gdiplus::Graphics(hDC);

	Gdiplus::Font font(L"Arial.ttf", 1);
	Gdiplus::SolidBrush brush(Gdiplus::Color(255, 255, 255));

	graphics->DrawString(L"Container has exited.", 21, &font, Gdiplus::PointF(0, 0), &brush);
	Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(rect.right, rect.bottom, graphics);

	ReleaseDC(NULL, hDC);

	return bitmap;
}

void SANDBOXCLIENT_API SandboxInitClient(const wchar_t* IPAddress)
{
	RPC_WSTR StringBinding;

	RPC_BINDING_HANDLE BindingHandle;

	cout << "Configuring window stations..." << endl;
	// Set window station and desktop access rights
	TweakWindowStation();

	cout << "Assigning current process to the default window station..." << endl;
	// Assign this program to WinSta0 and the default desktop
	AssignWindowStation();

	cout << "Registering desktop window..." << endl;
	// Set some colors
	RegisterDesktopWindow();

	cout << "Starting CMD..." << endl;
	StartProgram();

	/*
	DEVMODE DevMode{ 0 };
	DevMode.dmSize = sizeof(DEVMODE);
	DevMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
	DevMode.dmPelsWidth = 1280;
	DevMode.dmPelsHeight = 1024;
	LONG Result = ChangeDisplaySettings(&DevMode, 0);

	cout << "ChangeDisplaySettings returned " << hex << Result << endl;
	*/

	cout << "Composing...";

	RPC_STATUS Status = RpcStringBindingCompose(
		NULL,
		(RPC_WSTR)TEXT("ncacn_ip_tcp"),
		(RPC_WSTR)IPAddress,
		NULL,
		NULL,
		&StringBinding
	);

	if (Status != RPC_S_OK)
	{
		throw Status;
	}

	cout << "Getting binding..." << endl;
	Status = RpcBindingFromStringBinding(StringBinding, &BindingHandle);
	RpcStringFree(&StringBinding);

	if (Status != RPC_S_OK)
	{
		throw Status;
	}

	cout << "Done." << endl;

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	bool bContinue = true;
	while(bContinue)
	{
		DWORD dwCount = 0;
		EnumWindows(WndCountProc, (LPARAM)&dwCount);

		Gdiplus::Bitmap* Bitmap = nullptr;

		if (!dwCount)
		{
			// Break when there are no more windows
			cout << "No more visible windows, exiting." << endl;
			bContinue = false;
		}

		Bitmap = PrintScreen(GetThreadDesktop(GetCurrentThreadId()));

		Gdiplus::Rect Rect(0, 0, Bitmap->GetWidth(), Bitmap->GetHeight());

		CLSID sid;
		GetEncoderClsid(L"image/png", &sid);

		IStream* Stream;
		CreateStreamOnHGlobal(NULL, TRUE, &Stream);

		Bitmap->Save(Stream, &sid, NULL);

		HGLOBAL hGlobalMem = NULL;
		GetHGlobalFromStream(Stream, &hGlobalMem);

		const SIZE_T Size = GlobalSize(hGlobalMem);
		const SIZE_T ChunkSize = 1 * 1024 * 1024;
		SIZE_T SizeRemaining = Size;

		LPVOID lpGlobalMem = GlobalLock(hGlobalMem);

		RpcTryExcept
		{
			RpcBeginImage(BindingHandle, Bitmap->GetWidth(), Bitmap->GetHeight(), (unsigned long)Size);

			while (SizeRemaining > 0)
			{
				const unsigned char* StartData = (unsigned char*)lpGlobalMem + (Size - SizeRemaining);
				const SIZE_T SizeToSend = min(SizeRemaining, ChunkSize);

				RpcSendImageChunkData(
					BindingHandle,
					(unsigned long)SizeToSend,
					StartData
				);

				SizeRemaining -= SizeToSend;
			}

			RpcEndImage(BindingHandle);

			SandboxEvent event;
			while (RpcPopEvent(BindingHandle, &event))
			{
				switch (event.eventType)
				{
				case SBET_MouseEvent:
					if(!SetCursorPos(event.mouseEvent.x, event.mouseEvent.y))
					{
						//cout << "SetCursorPos returned false with error " << hex << GetLastError() << endl;
					}

					if (event.mouseEvent.buttonIndex != -1)
					{
						INPUT input = { 0 };
						input.type = INPUT_MOUSE;
						input.mi.dx = event.mouseEvent.x;
						input.mi.dy = event.mouseEvent.y;

						switch (event.mouseEvent.buttonIndex)
						{
						case 1:
							input.mi.dwFlags = event.mouseEvent.bKeyDown ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
							break;
						case 2:
							input.mi.dwFlags = event.mouseEvent.bKeyDown ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
							break;
						case 3:
							input.mi.dwFlags = event.mouseEvent.bKeyDown ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
							break;
						}

						if (!SendInput(1, &input, sizeof(INPUT)))
						{
							cout << "SendInput for mouse returned false with error " << hex << GetLastError() << endl;
						}
					}
					break;
				case SBET_KeyEvent:
				{
					INPUT input = { 0 };
					input.type = INPUT_KEYBOARD;
					input.ki.wScan = event.keyboardEvent.keyIndex;
					input.ki.wVk = MapVirtualKey(input.ki.wScan, MAPVK_VSC_TO_VK_EX);
					input.ki.dwFlags = event.keyboardEvent.bKeyDown ? 0 : KEYEVENTF_KEYUP;

					input.ki.dwFlags |= KEYEVENTF_SCANCODE;

					cout << "scan:" << input.ki.wScan << " vk: " << input.ki.wVk << " received" << endl;

					if (!SendInput(1, &input, sizeof(INPUT)))
					{
						cout << "SendInput returned false with error " << hex << GetLastError() << endl;
					}

				}
					break;
				}
			}
		}
		RpcExcept(1)
		{
			cout << "Exception " << hex << RpcExceptionCode() << " encountered while sending data" << endl;

			if (RpcExceptionCode() == EPT_S_NOT_REGISTERED
				|| RpcExceptionCode() == RPC_S_SERVER_UNAVAILABLE)
			{
				bContinue = false;
			}
		}
		RpcEndExcept

		GlobalUnlock(hGlobalMem);

		Stream->Release();
		delete Bitmap;
	}
	//CLSID sid;
	//GetEncoderClsid(L"image/png", &sid);

	//Bitmap->Save(L"image.png", &sid, NULL);

	//RpcTryExcept
	//{
	//MyRemoteProc(BindingHandle);
	//}
	//RpcExcept(1)
	//{
	//	printf("Exception %d occured\n", RpcExceptionCode());
	//}
	//RpcEndExcept
}
