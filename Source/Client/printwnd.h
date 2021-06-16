#pragma once

#include <gdiplus.h>

Gdiplus::Bitmap* PrintScreen(HDESK hDesktop);
void RegisterDesktopWindow();
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
