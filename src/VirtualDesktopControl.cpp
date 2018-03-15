
#include "VirtualDesktopControl.h"


VirtualDesktopControl::VirtualDesktopControl() {
}

HRESULT VirtualDesktopControl::Initialize() {
	//::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	//if (!IsWindows10OrGreater()) {
	//	return E_FAIL;
	//}

	if (FAILED(::CoCreateInstance(CLSID_ImmersiveShell, NULL, CLSCTX_LOCAL_SERVER, __uuidof(IServiceProvider), (PVOID*)&pServiceProvider))) {
		return E_FAIL;
	}
	if (FAILED(pServiceProvider->QueryService(CLSID_VirtualDesktopAPI_Unknown, &pDesktopManagerInternal))) {
		return E_FAIL;
	}
	if (FAILED(pServiceProvider->QueryService(CLASID_ApplicationViewCollection, &pApplicationViewCollection))) {
		return E_FAIL;
	}
	if (FAILED(::CoCreateInstance(CLSID_VirtualDesktopManager, NULL, CLSCTX_INPROC, __uuidof(IVirtualDesktopManager), (PVOID*)&pDesktopManager))) {
		return E_FAIL;
	}

	_initialized = true;
	return S_OK;
}

void VirtualDesktopControl::Release() {
	pServiceProvider->Release();
	pDesktopManagerInternal->Release();
	pDesktopManager->Release();
	pApplicationViewCollection->Release();
	//CoUninitialize();
}


HRESULT SwitchVirtualDesktop(IVirtualDesktopManagerInternal *pDesktopManagerInternal, UINT num) {
	IObjectArray *pDesktops;
	if (pDesktopManagerInternal == nullptr || FAILED(pDesktopManagerInternal->GetDesktops(&pDesktops))) {
		return E_FAIL;
	}

	HRESULT hres = S_OK;
	IVirtualDesktop *pDesktop;
	if (SUCCEEDED(pDesktops->GetAt(num, __uuidof(IVirtualDesktop), (void**)&pDesktop))) {
		hres = pDesktopManagerInternal->SwitchDesktop(pDesktop);
	}
	else {
		hres = E_FAIL;
	}

	return hres;
}

HRESULT VirtualDesktopControl::SwitchToDesktop(UINT num) {
	if (!_initialized) return E_FAIL;

	if (FAILED(SwitchVirtualDesktop(pDesktopManagerInternal, num))) {
		return E_FAIL;
	}
	return S_OK;
}

HRESULT VirtualDesktopControl::MoveWindowToDesktop(HWND wnd, UINT num) {
	if (!_initialized) return E_FAIL;

	IObjectArray *pDesktops;
	if (pDesktopManagerInternal == nullptr || FAILED(pDesktopManagerInternal->GetDesktops(&pDesktops))) {
		return E_FAIL;
	}

	HRESULT hres = S_OK;
	IVirtualDesktop *pDesktop;
	if (SUCCEEDED(pDesktops->GetAt(num, __uuidof(IVirtualDesktop), (void**)&pDesktop))) {
		IApplicationView *pView;
		pApplicationViewCollection->GetViewForHwnd(wnd, &pView);
		pDesktopManagerInternal->MoveViewToDesktop(pView, pDesktop);

		//IObjectArray *pViews;
		//pApplicationViewCollection->GetViews(&pViews);

		// This method doesn't work for other processes, 
		//GUID pGuid;
		//pDesktop->GetID(&pGuid);
		//pDesktopManager->MoveWindowToDesktop(wnd, pGuid);
	}
	return hres;
}