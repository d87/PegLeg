
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
	if (FAILED(pServiceProvider->QueryService(CLASID_VirtualDesktopPinnedApps, &pVirtualDesktopPinnedApps))) {
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


HRESULT VirtualDesktopControl::TogglePinWindow(HWND wnd) {
	if (!_initialized) return E_FAIL;

	IObjectArray *pDesktops;
	if (pDesktopManagerInternal == nullptr || FAILED(pDesktopManagerInternal->GetDesktops(&pDesktops))) {
		return E_FAIL;
	}


	IApplicationView *pView;
	if (SUCCEEDED(pApplicationViewCollection->GetViewForHwnd(wnd, &pView))) {
		BOOL isViewPinned;
		if (SUCCEEDED(pVirtualDesktopPinnedApps->IsViewPinned(pView, &isViewPinned))) {
			if (isViewPinned) {
				return pVirtualDesktopPinnedApps->UnpinView(pView);
			}
			else {
				return pVirtualDesktopPinnedApps->PinView(pView);
			}
		}
	}

	return E_FAIL;
}


int VirtualDesktopControl::GetDesktopCount() {

	IObjectArray *pObjectArray = nullptr;
	HRESULT hr = pDesktopManagerInternal->GetDesktops(&pObjectArray);

	if (SUCCEEDED(hr))
	{
		UINT count;
		hr = pObjectArray->GetCount(&count);
		pObjectArray->Release();
		return count;
	}

	return -1;
}



int VirtualDesktopControl::GetDesktopNumberById(GUID desktopId) {
	IObjectArray *pObjectArray = nullptr;
	HRESULT hr = pDesktopManagerInternal->GetDesktops(&pObjectArray);
	int found = -1;

	if (SUCCEEDED(hr))
	{
		UINT count;
		hr = pObjectArray->GetCount(&count);

		if (SUCCEEDED(hr))
		{
			for (UINT i = 0; i < count; i++)
			{
				IVirtualDesktop *pDesktop = nullptr;

				if (FAILED(pObjectArray->GetAt(i, __uuidof(IVirtualDesktop), (void**)&pDesktop)))
					continue;

				GUID id = { 0 };
				if (SUCCEEDED(pDesktop->GetID(&id)) && id == desktopId)
				{
					found = i;
					pDesktop->Release();
					break;
				}

				pDesktop->Release();
			}
		}

		pObjectArray->Release();
	}

	return found;
}

int VirtualDesktopControl::GetDesktopNumber(IVirtualDesktop *pDesktop) {
	if (pDesktop == nullptr) {
		return -1;
	}

	GUID guid;

	if (SUCCEEDED(pDesktop->GetID(&guid))) {
		return GetDesktopNumberById(guid);
	}

	return -1;
}


IVirtualDesktop* VirtualDesktopControl::GetCurrentDesktop() {
	if (pDesktopManagerInternal == nullptr) {
		return nullptr;
	}
	IVirtualDesktop* found = nullptr;
	pDesktopManagerInternal->GetCurrentDesktop(&found);
	return found;
}

int VirtualDesktopControl::GetCurrentDesktopNumber() {
	IVirtualDesktop* virtualDesktop = GetCurrentDesktop();
	int number = GetDesktopNumber(virtualDesktop);
	virtualDesktop->Release();
	return number;
}