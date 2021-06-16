#define OEMRESOURCE
#include <Windows.h>

#include <gdiplus.h>
#include <ole2.h>
#include <olectl.h>
#include <vector>

#include <iostream>

using namespace std;

HWND hDesktopWnd = NULL;
HDC hDesktopDC = NULL;

using namespace Gdiplus;

BOOL CALLBACK EnumWindowsReverseProc(
	_In_ HWND   hwnd,
	_In_ LPARAM lParam
)
{
	std::vector<HWND>* hwndList = (std::vector<HWND>*)lParam;
	hwndList->push_back(hwnd);

	return TRUE;
}

#if 0
void DrawWindowInternal(HWND hwnd, HDC hdc)
{
	WINDOWINFO WindowInfo;
	GetWindowInfo(hwnd, &WindowInfo);

	HWND hwndParent = GetAncestor(hwnd, GA_ROOT);

	WINDOWINFO OwnerWindowInfo;
	GetWindowInfo(hwndParent, &OwnerWindowInfo);

	if (WindowInfo.dwStyle & WS_VISIBLE)
	{
		BOOL b = FALSE;
		RECT rect;

		rect.left = WindowInfo.rcWindow.left - OwnerWindowInfo.rcWindow.left;
		rect.top = WindowInfo.rcWindow.top - OwnerWindowInfo.rcWindow.top;
		rect.right = WindowInfo.rcWindow.right - OwnerWindowInfo.rcWindow.left;
		rect.bottom = WindowInfo.rcWindow.bottom - OwnerWindowInfo.rcWindow.top;

		//FillRect( hdc, &rect, ( HBRUSH )( COLOR_WINDOW + 1 ) );
		//DrawFrameControl( hdc, &rect, DFC_BUTTON, DFCS_BUTTON3STATE );

		if (WindowInfo.dwStyle & WS_CAPTION)
		{
			RECT frame;

			frame.top = rect.top;
			frame.right = rect.right;
			frame.bottom = frame.top + 16;
			frame.left = frame.right - 32;
			DrawFrameControl(hdc, &frame, DFC_CAPTION, DFCS_CAPTIONCLOSE);

			frame.left -= 32;
			frame.right -= 32;
			DrawFrameControl(hdc, &frame, DFC_CAPTION, DFCS_CAPTIONMAX);

			frame.left -= 32;
			frame.right -= 32;
			DrawFrameControl(hdc, &frame, DFC_CAPTION, DFCS_CAPTIONMIN);

			frame.top = rect.top;
			frame.left = rect.left;
			frame.right = rect.right;
			frame.bottom = 50;
			//b = DrawCaption( hwnd, hdc, &frame, DC_BUTTONS | DC_ICON | DC_TEXT );

			//UserDrawCaption( hwnd, hdc, &frame, NULL, NULL, DC_BUTTONS | DC_ICON | DC_TEXT );
			UserDrawCaptionBar(hwnd, hdc, 0);
		}
		else
		{
			Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
		}

		TCHAR WindowText[256];
		GetWindowText(hwnd, WindowText, sizeof(WindowText));

		SetTextColor(hdc, RGB(0, 0, 0));
		SetBkMode(hdc, TRANSPARENT);
		rect.top += 4;
		rect.left += 4;

		//DrawText( hdc, WindowText, -1, &rect, DT_SINGLELINE );
	}
}

void DrawWindow(HWND hwnd, HDC hdc)
{
	std::vector<HWND> hwndList;

	RECT rect;
	GetWindowRect(hwnd, &rect);

	EnumChildWindows(hwnd, EnumWindowsReverseProc, (LPARAM)& hwndList);
	hwndList.push_back(hwnd);

	for (vector<HWND>::reverse_iterator rit = hwndList.rbegin(); rit != hwndList.rend(); rit++)
	{
		DrawWindowInternal(*rit, hdc);
	}

	/*
	TCHAR WindowText[ 256 ];
	GetWindowText( WindowText, sizeof( WindowText ) );

	SetTextColor( hdc, RGB( 0, 0, 0 ) );
	SetBkMode( hdc, TRANSPARENT );
	rect.top += 4;
	rect.left += 4;

	DrawText( hdc, WindowText, -1, &rect, DT_SINGLELINE );
	*/
}
#endif

typedef struct {
	int ScreenWidth;
	int ScreenHeight;
	HDC hdcMemory;
	HDC hdcSource;
} EnumWindowParam;

HBITMAP ghTempBitmap = NULL;

BOOL CALLBACK EnumWindowsProc(
	_In_ HWND   hwnd,
	_In_ LPARAM lParam
)
{
	EnumWindowParam* Param = (EnumWindowParam*)lParam;
	WINDOWINFO WindowInfo;

	GetWindowInfo(hwnd, &WindowInfo);

	if ((WindowInfo.dwStyle & WS_VISIBLE))
	{
		RECT r;
		GetWindowRect(hwnd, &r);
		int x = r.left;
		int y = r.top;
		int w = r.right - r.left;
		int h = r.bottom - r.top;

		HDC hdcWindow = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_WINDOW);
		HDC hdcWindowMemory = CreateCompatibleDC(hdcWindow);

		SelectObject(hdcWindowMemory, ghTempBitmap);

		// Write visuals into the DC
		PrintWindow(hwnd, hdcWindowMemory, 0);

		BitBlt(Param->hdcMemory, x, y, w, h, hdcWindowMemory, 0, 0, SRCCOPY | CAPTUREBLT);

		DeleteDC(hdcWindowMemory);
		ReleaseDC(hwnd, hdcWindow);
	}

	return TRUE;
}

void DrawCursor(HDC hDC)
{
	CURSORINFO cursor = { sizeof(cursor) };
	GetCursorInfo(&cursor);

	if (!cursor.hCursor)
	{
		// Use the default cursor image
		cursor.hCursor = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_NORMAL), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	}

	if (cursor.flags != CURSOR_SUPPRESSED)
	{
		ICONINFO info = { sizeof(info) };
		GetIconInfo(cursor.hCursor, &info);

		const int x = cursor.ptScreenPos.x - info.xHotspot;
		const int y = cursor.ptScreenPos.y - info.yHotspot;

		BITMAP bmpCursor = { 0 };
		GetObject(info.hbmColor, sizeof(bmpCursor), &bmpCursor);

		DrawIconEx(hDC, x, y, cursor.hCursor, 0, 0, 0, NULL, DI_NORMAL | DI_DEFAULTSIZE);

		DeleteObject(info.hbmMask);
		DeleteObject(info.hbmColor);
	}
}

Bitmap* PrintScreen(HDESK hDesktop)
{
	// Get the display DC
	HDC hdcSource = GetDC(NULL);
	if (!hdcSource)
	{
		cout << "DC could not be retrieved" << endl;
		return NULL;
	}

	// Display resolution
	int capX = GetDeviceCaps(hdcSource, HORZRES);
	int capY = GetDeviceCaps(hdcSource, VERTRES);

	HDC hdcMemory = CreateCompatibleDC(hdcSource);

	if(!ghTempBitmap)
	{
		// Create a single temp bitmap for all windows to draw on it
		// (avoids allocating a bitmap every time)
		ghTempBitmap = CreateCompatibleBitmap(hdcSource, capX, capY);
	}

	// Create a bitmap covering the whole screen
	HBITMAP hBitmap = CreateCompatibleBitmap(hdcSource, capX, capY);
	SelectObject(hdcMemory, hBitmap);

	EnumWindowParam EnumParam;
	EnumParam.ScreenWidth = capX;
	EnumParam.ScreenHeight = capY;
	EnumParam.hdcMemory = hdcMemory;
	EnumParam.hdcSource = hdcSource;

	// Fill background with the desktop color
	Rectangle(hdcMemory, 0, 0, capX, capY);

	RECT r{ 0, 0, capX, capY };
	FillRect(hdcMemory, &r, GetSysColorBrush(COLOR_DESKTOP));

	std::vector<HWND> hwndList;

	// Gather list of windows, so they can be drawn in reverse order
	// otherwise the back windows will appear first
	EnumDesktopWindows(hDesktop, EnumWindowsReverseProc, (LPARAM)& hwndList);

	for (std::vector<HWND>::reverse_iterator rit = hwndList.rbegin(); rit != hwndList.rend(); rit++)
	{
		EnumWindowsProc(*rit, (LPARAM)& EnumParam);
	}

	// If you uncomment this, it will draw the entire screen but it will look very messy, and there will be no console windows
	//BitBlt(hdcMemory, 0, 0, capX, capY, hdcSource, 0, 0, SRCCOPY);

	DrawCursor(hdcMemory);

	// Convert bitmap into a GDI+ Bitmap
	Bitmap* bmp = Bitmap::FromHBITMAP(hBitmap, NULL);

	DeleteObject(hBitmap);
	DeleteDC(hdcMemory);
	ReleaseDC(NULL, hdcSource);

	return bmp;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT dwMessage, WPARAM wParam, LPARAM lParam)
{
	switch (dwMessage)
	{
	case WM_ACTIVATE:
		if (wParam)
		{
			SetForegroundWindow((HWND)lParam);
		}
		return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT Paint;
		if (BeginPaint(hWnd, &Paint))
		{
			EndPaint(hWnd, &Paint);
		}
	}
		return 0;
	}

	return DefWindowProc(hWnd, dwMessage, wParam, lParam);
}

void RegisterDesktopWindow()
{
	INT Elements[] = { COLOR_DESKTOP };
	COLORREF RgbValues[] = { RGB(8, 110, 165) };
	SetSysColors(1, Elements, RgbValues);

	/*
	HBRUSH Brush = CreateSolidBrush(RGB(0, 255, 255));

	WNDCLASSEX wndClass;
	memset(&wndClass, 0, sizeof(wndClass));
	wndClass.style = CS_DBLCLKS;
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpszClassName = TEXT("DesktopWnd");
	wndClass.lpfnWndProc = WindowProc;
	wndClass.hbrBackground = Brush;

	RegisterClassEx(&wndClass);

	HDC hDeviceDC = GetDC(NULL);

	hDesktopWnd = CreateWindowEx(
		WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		TEXT("DesktopWnd"),
		NULL,
		WS_VISIBLE | WS_CLIPSIBLINGS,
		0,
		0,
		GetDeviceCaps(hDeviceDC, HORZRES),
		GetDeviceCaps(hDeviceDC, VERTRES),
		NULL,
		NULL,
		NULL,
		NULL
	);

	ReleaseDC(NULL, hDeviceDC);


	hDesktopDC = GetWindowDC(hDesktopWnd);
	*/
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num = 0;          // number of image encoders
	UINT size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
	{
		return -1;  // Failure
	}

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
	{
		return -1;  // Failure
	}

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (_wcsicmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}
