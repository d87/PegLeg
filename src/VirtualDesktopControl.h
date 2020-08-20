#pragma once

#include <windows.h>
#include <VersionHelpers.h>
#include <Shlobj.h>
#include "VirtualDesktopComObjects.h"


class VirtualDesktopControl {
public:
	IVirtualDesktopManagerInternal *pDesktopManagerInternal = nullptr;
	IVirtualDesktopManager *pDesktopManager = nullptr;
	IServiceProvider* pServiceProvider = nullptr;
	IApplicationViewCollection *pApplicationViewCollection = nullptr;
	IVirtualDesktopPinnedApps *pVirtualDesktopPinnedApps = nullptr;

private:
	BOOL _initialized = false;

public:
	VirtualDesktopControl();
	HRESULT Initialize();
	HRESULT SwitchToDesktop(UINT num);
	HRESULT MoveWindowToDesktop(HWND wnd, UINT num);
	HRESULT TogglePinWindow(HWND wnd);
	int GetDesktopNumberById(GUID desktopId);
	int GetDesktopNumber(IVirtualDesktop *pDesktop);
	IVirtualDesktop* GetCurrentDesktop();
	int GetCurrentDesktopNumber();
	int GetDesktopCount();
	void Release();
};
