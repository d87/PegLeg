#include "util.h"

std::string GetExplorerWindowPathByHWND(HWND hWnd) {

    std::string path;
    //LPWSTR pStrFileName = NULL;

    IShellWindows* psw;

    HRESULT hr = CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
        IID_IShellWindows, (void**)&psw);

    if (SUCCEEDED(hr)) {

        long numSHellWindows = 0;
        psw->get_Count(&numSHellWindows);

        for (long i = numSHellWindows - 1; i >= 0; i--) {
            IDispatch* dispatch;
            VARIANT index;
            index.vt = VT_I4;
            index.lVal = i;

            hr = psw->Item(index, &dispatch);

            if (SUCCEEDED(hr)) {
                // Getting IWB2 from item's dispatch seems to be the only way to get explorer window's HWND
                IWebBrowser2* iwb;
                hr = dispatch->QueryInterface(IID_IWebBrowser2, (void**)&iwb);
                if (SUCCEEDED(hr)) {
                    // Skip to the next item if HWNDs not matching
                    HWND explorer_hWnd;
                    SHANDLE_PTR as;
                    iwb->get_HWND((SHANDLE_PTR*)&explorer_hWnd);
                    if (explorer_hWnd == hWnd) {

                        // But for actually getting the shell view and the rest IServiceProvider is used
                        IServiceProvider* service_provider;
                        hr = dispatch->QueryInterface(IID_IServiceProvider, (void**)&service_provider);

                        if (SUCCEEDED(hr)) {

                            IShellBrowser* shell_browser;
                            hr = service_provider->QueryService(SID_STopLevelBrowser, IID_IShellBrowser, (void**)&shell_browser);

                            if (SUCCEEDED(hr)) {
                                IShellView* psv;
                                shell_browser->QueryActiveShellView(&psv);

                                IFolderView* pIFV;
                                if (SUCCEEDED(hr = psv->QueryInterface(IID_IFolderView, (void**)&pIFV)))
                                {
                                    IPersistFolder2* pIPF2;
                                    if (SUCCEEDED(hr = pIFV->GetFolder(IID_IPersistFolder2, (void**)&pIPF2)))
                                    {
                                        LPITEMIDLIST pidlFolder = NULL;
                                        if (SUCCEEDED(hr = pIPF2->GetCurFolder(&pidlFolder)))
                                        {
                                            LPCITEMIDLIST pidlChild = NULL;
                                            IShellFolder* pIShFldr;
                                            if (SUCCEEDED(::SHBindToParent(pidlFolder, IID_IShellFolder, (void**)&pIShFldr, &pidlChild)))
                                            {
                                                ULONG attrs = SFGAO_FILESYSTEM;
                                                if (SUCCEEDED(hr = pIShFldr->GetAttributesOf(1, &pidlChild, &attrs)))
                                                {
                                                    if (attrs & SFGAO_FILESYSTEM)
                                                    {
                                                        STRRET srt;
                                                        if (SUCCEEDED(hr = pIShFldr->GetDisplayNameOf(pidlChild, SHGDN_FORPARSING, &srt)))
                                                        {
                                                            LPSTR pStrFileName = NULL;
                                                            if (SUCCEEDED(hr = StrRetToStrA(&srt, pidlChild, &pStrFileName)))
                                                            {
                                                                path = pStrFileName;
                                                                //Compare to our path
                                                                //if (lstrlen(pStrFileName) >= nLnVolPath &&
                                                                //    ::CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                                                //        buffVolPath, nLnVolPath,
                                                                //        pStrFileName, nLnVolPath) == CSTR_EQUAL)
                                                                //{
                                                                //    //Close it
                                                                //    hr = pIWB2->Quit();
                                                                //}
                                                            }

                                                            if (pStrFileName)
                                                            {
                                                                CoTaskMemFree(pStrFileName);
                                                                pStrFileName = NULL;
                                                            }

                                                            //Free mem (if StrRetToStr() hasn't done it)
                                                            if (srt.pOleStr)
                                                            {
                                                                CoTaskMemFree(srt.pOleStr);
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            pIShFldr->Release();
                                        }
                                    }
                                    pIPF2->Release();
                                }
                                pIFV->Release();

                                psv->Release();
                            }
                            shell_browser->Release();
                        }
                        service_provider->Release();
                    }
                }
                iwb->Release();
            }
            dispatch->Release();
        }
    }
    psw->Release();
    return path;
}