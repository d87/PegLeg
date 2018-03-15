#pragma once

#include <objbase.h>
#include <ObjectArray.h>

const CLSID CLSID_ImmersiveShell = {
	0xC2F03A33, 0x21F5, 0x47FA, 0xB4, 0xBB, 0x15, 0x63, 0x62, 0xA2, 0xF2, 0x39 };

//= new Guid("aa509086-5ca9-4c25-8f95-589d3c07b48a");
const CLSID CLSID_VirtualDesktopManager = {
	0xAA509086, 0x5CA9, 0x4C25, 0x8F, 0x95, 0x58, 0x9D, 0x3C, 0x07, 0xB4, 0x8A };

//CLSID_VirtualDesktopManager

const CLSID CLSID_VirtualDesktopAPI_Unknown = {
	0xC5E0CDCA, 0x7B6E, 0x41B2, 0x9F, 0xC4, 0xD9, 0x39, 0x75, 0xCC, 0x46, 0x7B };

const IID IID_IVirtualDesktopManagerInternal = {
	0xEF9F1A6C, 0xD3CC, 0x4358, 0xB7, 0x12, 0xF8, 0x4B, 0x63, 0x5B, 0xEB, 0xE7 };

//2c08adf0 - a386 - 4b35 - 9250 - 0fe183476fcc
const CLSID CLASID_ApplicationViewCollection = {
	0x2C08ADF0, 0xA386, 0x4B35, 0x92, 0x50, 0x0F, 0xE1, 0x83, 0x47, 0x6F, 0xCC };

//4ce81583-1e4c-4632-a621-07a53543148f
//b5a399e7 - 1c87 - 46b8 - 88e9 - fc5747b171bd
const CLSID CLASID_VirtualDesktopPinnedApps = {
	0xB5A399E7, 0x1C87, 0x46B8, 0x88, 0xE9, 0xFC, 0x57, 0x47, 0xB1, 0x71, 0xBD };

EXTERN_C const IID IID_IApplicationView;

MIDL_INTERFACE("9ac0b5c8-1484-4c5b-9533-4134a0f97cea")
IApplicationView : public IUnknown
{
public:
	//int SetFocus();
	virtual HRESULT STDMETHODCALLTYPE SetFocus() = 0;

	//int SwitchTo();
	virtual HRESULT STDMETHODCALLTYPE SwitchTo() = 0;

	virtual HRESULT STDMETHODCALLTYPE GetVirtualDesktopId(
		GUID **pGUID) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetVirtualDesktopId(
		GUID *pGUID) = 0;
};

EXTERN_C const IID IID_IVirtualDesktop;

MIDL_INTERFACE("ff72ffdd-be7e-43fc-9c03-ad81681e88e4")
IVirtualDesktop : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE IsViewVisible(
		IApplicationView *pView,
		int *pfVisible) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetID(
		GUID *pGuid) = 0;
};

enum AdjacentDesktop
{
	LeftDirection = 3,
	RightDirection = 4
};

EXTERN_C const IID IID_IVirtualDesktopManagerInternal;


// check https://github.com/Grabacr07/VirtualDesktop/blob/master/source/VirtualDesktop/Interop/IVirtualDesktopManagerInternal.cs if it becomes outdated
MIDL_INTERFACE("f31574d6-b682-4cdc-bd56-1827860abec6")
IVirtualDesktopManagerInternal : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetCount(
		UINT *pCount) = 0;

	virtual HRESULT STDMETHODCALLTYPE MoveViewToDesktop(
		IApplicationView *pView,
		IVirtualDesktop *pDesktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE CanViewMoveDesktops(
		IApplicationView *pView,
		int *pfCanViewMoveDesktops) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetCurrentDesktop(
		IVirtualDesktop** desktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetDesktops(
		IObjectArray **ppDesktops) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetAdjacentDesktop(
		IVirtualDesktop *pDesktopReference,
		AdjacentDesktop uDirection,
		IVirtualDesktop **ppAdjacentDesktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE SwitchDesktop(
		IVirtualDesktop *pDesktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE CreateDesktopW(
		IVirtualDesktop **ppNewDesktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE RemoveDesktop(
		IVirtualDesktop *pRemove,
		IVirtualDesktop *pFallbackDesktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE FindDesktop(
		GUID *desktopId,
		IVirtualDesktop **ppDesktop) = 0;
};

EXTERN_C const IID IID_IVirtualDesktopManager;

MIDL_INTERFACE("a5cd92ff-29be-454c-8d04-d82879fb3f1b")
IVirtualDesktopManager : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE IsWindowOnCurrentVirtualDesktop(
		/* [in] */ __RPC__in HWND topLevelWindow,
		/* [out] */ __RPC__out BOOL *onCurrentDesktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetWindowDesktopId(
		/* [in] */ __RPC__in HWND topLevelWindow,
		/* [out] */ __RPC__out GUID *desktopId) = 0;

	virtual HRESULT STDMETHODCALLTYPE MoveWindowToDesktop(
		/* [in] */ __RPC__in HWND topLevelWindow,
		/* [in] */ __RPC__in REFGUID desktopId) = 0;
};

EXTERN_C const IID IID_IVirtualDesktopNotification;

MIDL_INTERFACE("c179334c-4295-40d3-bea1-c654d965605a")
IVirtualDesktopNotification : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopCreated(
		IVirtualDesktop *pDesktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyBegin(
		IVirtualDesktop *pDesktopDestroyed,
		IVirtualDesktop *pDesktopFallback) = 0;

	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyFailed(
		IVirtualDesktop *pDesktopDestroyed,
		IVirtualDesktop *pDesktopFallback) = 0;

	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyed(
		IVirtualDesktop *pDesktopDestroyed,
		IVirtualDesktop *pDesktopFallback) = 0;

	virtual HRESULT STDMETHODCALLTYPE ViewVirtualDesktopChanged(
		IApplicationView *pView) = 0;

	virtual HRESULT STDMETHODCALLTYPE CurrentVirtualDesktopChanged(
		IVirtualDesktop *pDesktopOld,
		IVirtualDesktop *pDesktopNew) = 0;

};

EXTERN_C const IID IID_IVirtualDesktopNotificationService;

MIDL_INTERFACE("0cd45e71-d927-4f15-8b0a-8fef525337bf")
IVirtualDesktopNotificationService : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE Register(
		IVirtualDesktopNotification *pNotification,
		DWORD *pdwCookie) = 0;

	virtual HRESULT STDMETHODCALLTYPE Unregister(
		DWORD dwCookie) = 0;
};



EXTERN_C const IID IID_IApplicationViewCollection;

MIDL_INTERFACE("2c08adf0-a386-4b35-9250-0fe183476fcc")
IApplicationViewCollection : public IUnknown
{
public:
	//int GetViews(out IObjectArray array);
	virtual HRESULT STDMETHODCALLTYPE GetViews(
		IObjectArray **ppObjectArray) = 0;

	//int GetViewsByZOrder(out IObjectArray array);
	virtual HRESULT STDMETHODCALLTYPE GetViewsByZOrder(
		IObjectArray **ppObjectArray) = 0;

	//int GetViewsByAppUserModelId(string id, out IObjectArray array);
	virtual HRESULT STDMETHODCALLTYPE GetViewsByAppUserModelId(
		WCHAR *id,
		IObjectArray **ppObjectArray) = 0;

	//int GetViewForHwnd(IntPtr hwnd, out IApplicationView view);
	virtual HRESULT STDMETHODCALLTYPE GetViewForHwnd(
			HWND hWnd,
			IApplicationView **ppApplicationView) = 0;
};


EXTERN_C const IID IID_IVirtualDesktopPinnedApps;

MIDL_INTERFACE("4ce81583-1e4c-4632-a621-07a53543148f")
IVirtualDesktopPinnedApps : public IUnknown
{
public:
	//bool IsAppIdPinned(string appId);
	virtual HRESULT STDMETHODCALLTYPE IsAppIdPinned(
		LPCTSTR *appId, BOOL *bResult) = 0;

	//void PinAppID(string appId);
	virtual HRESULT STDMETHODCALLTYPE PinAppID(
		LPCTSTR *appId) = 0;

	//void UnpinAppID(string appId);
	virtual HRESULT STDMETHODCALLTYPE UnpinAppID(
		LPCTSTR *appId) = 0;

	//bool IsViewPinned(IApplicationView applicationView);
	virtual HRESULT STDMETHODCALLTYPE IsViewPinned(
		IApplicationView *pView, BOOL *bResult) = 0;

	//void PinView(IApplicationView applicationView);
	virtual HRESULT STDMETHODCALLTYPE PinView(
		IApplicationView *pView) = 0;

	//void UnpinView(IApplicationView applicationView);
	virtual HRESULT STDMETHODCALLTYPE UnpinView(
		IApplicationView *pView) = 0;
};