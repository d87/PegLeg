#pragma once

#include <windows.h>
#include <exdisp.h>
#include <Shobjidl.h>
#include <Shlobj.h>
#include <SHLGUID.h>
#include <shlwapi.h>
#include <string>

#pragma comment( lib, "Shlwapi.lib" )

std::string GetExplorerWindowPathByHWND(HWND hWnd);