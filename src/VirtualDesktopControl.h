#pragma once

#include <windows.h>
#include <VersionHelpers.h>
#include "VirtualDesktopComObjects.h"


class VirtualDesktopControl {
public:
	IVirtualDesktopManagerInternal *pDesktopManagerInternal = nullptr;
	IVirtualDesktopManager *pDesktopManager = nullptr;
	IServiceProvider* pServiceProvider = nullptr;
	IApplicationViewCollection *pApplicationViewCollection = nullptr;

private:
	BOOL _initialized = false;

public:
	VirtualDesktopControl();
	HRESULT Initialize();
	HRESULT SwitchToDesktop(UINT num);
	HRESULT MoveWindowToDesktop(HWND wnd, UINT num);
	void Release();
};
