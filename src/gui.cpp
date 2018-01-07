#include <Windows.h>
#include <richedit.h>
#include <stdio.h>

#include "gui.h"
#include "res.h"
#include "pegleg.h"
#define WM_SHELLNOTIFY          (WM_USER+1)

extern int Shutdown();

struct gui_struct gui;
UINT WM_TASKBARCREATED;

WNDPROC DefEditProc;

//int WINAPI WinMain(HINSTANCE hInstance , HINSTANCE hPrevInstance , LPSTR lpCmdLine , int nCmdShow )
void guiThread( void *param  )
{
	HINSTANCE hInstance = (HINSTANCE)param;
	gui.hInstance = hInstance;

	WNDCLASSEX wc;
	//LoadLibrary("RichEd20.dll");
	LoadLibrary("Msftedit.dll");
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style  = 0;
	wc.lpfnWndProc = WndProc ;
	wc.cbClsExtra  = 0;
	wc.cbWndExtra  = 0;
	wc.hInstance   = hInstance;
	wc.hIcon       = LoadIcon  (NULL,IDI_APPLICATION);
	wc.hCursor     = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)3;
	//(COLOR_WINDOW+1)
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "PegLegWnd";
	wc.hIconSm       = LoadIcon(NULL,IDI_APPLICATION);

	if(!RegisterClassEx(&wc)) {
        	MessageBox(NULL , "Failed to register window class" , "Error" , MB_ICONEXCLAMATION | MB_OK );
 	        return;
	}

	gui.hwnd = CreateWindowEx(
		//WS_EX_CLIENTEDGE,
		WS_EX_TOOLWINDOW|WS_EX_TOPMOST,
		wc.lpszClassName,
		"PegLeg Console" ,
		WS_OVERLAPPED|WS_SYSMENU,
		CW_USEDEFAULT , CW_USEDEFAULT , gui.width, gui.height,
		NULL , NULL , 
		hInstance , NULL
		);
	HWND hwnd = gui.hwnd;

	if(hwnd == NULL) {
		MessageBox(NULL , "Failed to create window", "Error", MB_ICONEXCLAMATION | MB_OK );
		return;
    }

		ShowWindow  (hwnd,SW_HIDE);
	UpdateWindow(hwnd);
	UpdateWindow(gui.hwndConsole);
	SetEvent(gui.event_ready);
	
	WM_TASKBARCREATED = RegisterWindowMessageW(L"TaskbarCreated"); //when explorer crashes it will restore tray icon
	MSG Msg;
	BOOL bRet;
	while((bRet = GetMessage(&Msg, NULL, 0, 0)) != 0) {
		TranslateMessage(&Msg);
		DispatchMessage (&Msg);
	}

	PostThreadMessage(mainThreadId, WM_QUIT, 0, 0);

	return;
}

int InstallTrayIcon(HWND hwnd){
	if (!gui.createtray) return 1;
	NOTIFYICONDATA dta;
	dta.cbSize = sizeof(NOTIFYICONDATA);
	dta.hWnd = hwnd;
	dta.uID = ID_TRAY;
	dta.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	dta.uCallbackMessage = WM_SHELLNOTIFY;
	dta.hIcon = LoadIcon(gui.hInstance, MAKEINTRESOURCE(ID_ICON1));
	strcpy(dta.szTip, "PegLeg");
	Shell_NotifyIcon(NIM_ADD, &dta);

	return 1;
}

LRESULT EditKeyProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_KEYDOWN) {
		if (wParam == VK_RETURN) { // && GetAsyncKeyState(VK_CONTROL)
			int nLen = GetWindowTextLengthW(hwnd);
			WCHAR * buffer = 0;
			buffer = new WCHAR[nLen + 1];
			GetWindowTextW(hwnd, buffer, nLen+1);
			buffer[nLen+1] = NULL;

			repl->Enqueue(buffer);
			PostThreadMessage(mainThreadId, WM_COMMAND, REPL_EVAL, NULL);

			UINT text_length = SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);
			CHARRANGE range;

			range.cpMin = 0;
			range.cpMax = text_length;
			SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&range);
			SendMessage(hwnd, EM_REPLACESEL, FALSE, 0);

			return 0;
		}
	}
	return CallWindowProc(DefEditProc, hwnd, uMsg, wParam, lParam);
}


int OnCreate( HWND hwnd)
{
	gui.hwndConsole = CreateWindowExW(
		0,
		MSFTEDIT_CLASS,
		NULL,
		WS_CHILD|WS_VISIBLE|ES_READONLY|ES_MULTILINE|WS_VSCROLL|ES_NOHIDESEL,
		0, 0,
		gui.width+3, gui.height-60,
		hwnd,
		0,
		gui.hInstance,
		NULL);
	if (!gui.hwndConsole) return 0;
	gui.hwndInput = CreateWindowExW(
		0,
		MSFTEDIT_CLASS,
		NULL,
		WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_NOHIDESEL,
		0, gui.height - 60,
		gui.width + 3, 25,
		hwnd,
		0,
		gui.hInstance,
		NULL);

	DefEditProc = (WNDPROC)SetWindowLongPtr(gui.hwndInput, GWLP_WNDPROC, (LONG_PTR)&EditKeyProc);

	COLORREF bgclr = RGB(15, 15, 15);
	SendMessage(gui.hwndInput, EM_SETBKGNDCOLOR, 0, (LPARAM)bgclr);

	CHARFORMAT fmt;
	COLORREF clr = RGB(178, 178, 178);

	fmt.cbSize = sizeof(CHARFORMAT);
	fmt.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE | CFM_BOLD | CFM_ITALIC | CFM_STRIKEOUT | CFM_UNDERLINE;
	fmt.yHeight = 200;
	fmt.dwEffects = 0;
	fmt.crTextColor = clr;
	strcpy(fmt.szFaceName, "Consolas");

	SendMessage(gui.hwndInput, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&fmt);
	SendMessage(gui.hwndConsole, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&fmt);

	if (gui.createtray) {
		gui.hmenuTray = CreatePopupMenu();
		AppendMenu(gui.hmenuTray, MF_STRING, ID_SHOWCONSOLE, "&Show Console");
		AppendMenu(gui.hmenuTray, MF_STRING, ID_RELOAD, "&Reload");
		AppendMenu(gui.hmenuTray, MF_STRING, ID_HIDECONSOLE, "&Hide Console");
		AppendMenu(gui.hmenuTray, MF_STRING, ID_EXIT, "E&xit");

		InstallTrayIcon(hwnd);
	}
	return 1;
}

int OnClose( HWND hwnd)
{
	//DestroyWindow  (hwnd);
	ShowWindow(hwnd, SW_HIDE);
	return 0;
}

void KillTrayIcon(HWND hwnd)
{
    NOTIFYICONDATA dta;

    dta.cbSize = sizeof(NOTIFYICONDATA);
    dta.hWnd = hwnd;
    dta.uID = ID_TRAY;
    dta.uFlags = 0;
    Shell_NotifyIcon(NIM_DELETE, &dta);
}


static void OnCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	DWORD hiw = HIWORD(wParam);


	//if (hiw == EN_UPDATE) {
	//	int a = 0;
	//}

	UINT id = (UINT)wParam;

	if (id == ID_EXIT) {
		//Shutdown();
		DestroyWindow(gui.hwnd);
	}
	else if (id == ID_SHOWCONSOLE )
		ShowWindow(gui.hwnd, SW_SHOW);
	else if (id == ID_RELOAD )
		PostThreadMessage(mainThreadId,WM_COMMAND, RELOADLUA, NULL);
	else if (id == ID_HIDECONSOLE )
		ShowWindow(gui.hwnd, SW_HIDE);
}

static int OnShellNotify(HWND hwnd, int uID, int uMessage)
{
    if(uID == ID_TRAY) {
        if(uMessage == WM_RBUTTONDOWN) {
			POINT cp;
			GetCursorPos(&cp);
			SetForegroundWindow(gui.hwnd);
			TrackPopupMenu(gui.hmenuTray, TPM_LEFTALIGN|TPM_LEFTBUTTON, cp.x, cp.y, 0, gui.hwnd, NULL);
		}
    }
    return 0;
}

static int OnHotkey(HWND hwnd , WPARAM wParam , LPARAM lParam) {
	int id = (int)wParam;
	return 0;
}

LRESULT CALLBACK WndProc (HWND hwnd , UINT msg,WPARAM wParam , LPARAM lParam)
{
	//COLORREF clr = RGB(0, 0, 0);
	if (msg == WM_TASKBARCREATED ){
		InstallTrayIcon(hwnd);
		return 0;
	}
	switch(msg) {		
		case WM_CREATE:
			OnCreate(hwnd);
			break;
		case WM_CLOSE:
			OnClose(hwnd);
			break;
		//case WM_SIZE:
			//break;
		case WM_HOTKEY:
			OnHotkey(hwnd, wParam, lParam);
			break;
		case WM_DESTROY:
			KillTrayIcon(hwnd);
			PostQuitMessage(1);
			break;
		case WM_COMMAND:
			OnCommand(hwnd, wParam, lParam);
			break;
		case WM_SHELLNOTIFY:
			OnShellNotify(hwnd, wParam, lParam);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void guiAddText(const char *str)
{
	COLORREF clr = RGB(gui.text_r*255,gui.text_g*255,gui.text_b*255);
	int start_lines, text_length, end_lines;
	CHARRANGE cr;
	CHARRANGE ds;
	//CHARFORMAT fmt;
	text_length = SendMessage(gui.hwndConsole, WM_GETTEXTLENGTH, 0, 0);
	
	if ( text_length >30000 ) {
		ds.cpMin = 0;
		//ds.cpMax = text_length - 30000;
		ds.cpMax = 10000;
		SendMessage(gui.hwndConsole, EM_EXSETSEL, 0, (LPARAM)&ds);
		SendMessage(gui.hwndConsole, EM_REPLACESEL, FALSE, 0);
	}
	
	cr.cpMin = text_length;
	cr.cpMax = text_length;
	
	SendMessage(gui.hwndConsole, EM_EXSETSEL, 0, (LPARAM)&cr); 
	
	//fmt.cbSize = sizeof(CHARFORMAT);
	//fmt.dwMask = CFM_COLOR|CFM_FACE|CFM_SIZE|CFM_BOLD|CFM_ITALIC|CFM_STRIKEOUT|CFM_UNDERLINE;
	//	fmt.yHeight = 200;
	//fmt.dwEffects = 0;
	//fmt.crTextColor = clr;
	//strcpy(fmt.szFaceName,"Consolas");

	std::string buf(str);
	int reqChars = MultiByteToWideChar(CP_UTF8, 0, buf.c_str(), -1, 0, 0);

	std::wstring wstr(reqChars, 0);
	MultiByteToWideChar(CP_UTF8, 0, buf.c_str(), -1, &wstr[0], reqChars);

	//http://www.nubaria.com/en/blog/?p=289
	
	//SendMessage(gui.hwndConsole, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&fmt);
	SendMessageW(gui.hwndConsole, EM_REPLACESEL, FALSE, (LPARAM)wstr.c_str());
}

int gui_printf( const char *format, va_list arglist )
{
	char buff[4096];	
	int result = vsprintf(buff, format, arglist);
	guiAddText(buff);
	return result;
}