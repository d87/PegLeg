#include <Windows.h>
#include <richedit.h>
#include <stdio.h>

#include "gui.h"
#include "res.h"
#include "pegleg.h"
#define WM_SHELLNOTIFY          (WM_USER+1)

//extern HHOOK hhkLowLevelKeyboard;
//extern HHOOK hhkLowLevelMouse;
extern int Shutdown();

gui_struct *gui = 0;

//int WINAPI WinMain(HINSTANCE hInstance , HINSTANCE hPrevInstance , LPSTR lpCmdLine , int nCmdShow )
void guiThread( void *param  )
{
	//gui->event_ready = CreateEvent(NULL, FALSE, FALSE, NULL);

	HINSTANCE hInstance = (HINSTANCE)param;
	gui->hInstance = hInstance;

	WNDCLASSEX wc;
	LoadLibrary("RichEd20.dll");
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

	//gr?
	if(!RegisterClassEx(&wc)) {
        	MessageBox(NULL , "Failed to register window class" , "Error" , MB_ICONEXCLAMATION | MB_OK );
 	        return;
	}

	HWND hwnd = CreateWindowEx(
		//WS_EX_CLIENTEDGE,
		WS_EX_TOOLWINDOW|WS_EX_TOPMOST,
		wc.lpszClassName,
		"PegLeg Console" ,
		WS_OVERLAPPED|WS_SYSMENU,
		CW_USEDEFAULT , CW_USEDEFAULT , gui->width, gui->height,
		NULL , NULL , 
		hInstance , NULL
		);
	gui->hwnd = hwnd;

	if(hwnd == NULL) {
		MessageBox(NULL , "Failed to create window", "Error", MB_ICONEXCLAMATION | MB_OK );
		return;
    }

		ShowWindow  (hwnd,SW_HIDE);
	UpdateWindow(hwnd);
	UpdateWindow(gui->hwndConsole);
	SetEvent(gui->event_ready);
	

	MSG Msg;
	while(GetMessage(&Msg,NULL,0,0)) {	
		TranslateMessage(&Msg);
		DispatchMessage (&Msg);
	}

	return;
}

int OnCreate( HWND hwnd)
{
	gui->hwndConsole = CreateWindowEx(
		0,
		RICHEDIT_CLASS,
		NULL,
		WS_CHILD|WS_VISIBLE|ES_READONLY|ES_MULTILINE|WS_VSCROLL|ES_NOHIDESEL,
		0, 0,
		gui->width, gui->height,
		hwnd,
		0,
		0,
		NULL);
	if (!gui->hwndConsole) return 0;

	//tray
	if (gui->createtray) {
	NOTIFYICONDATA dta;

	gui->hmenuTray = CreatePopupMenu();
	AppendMenu(gui->hmenuTray, MF_STRING, ID_SHOWCONSOLE, "&Show Console");
	AppendMenu(gui->hmenuTray, MF_STRING, ID_RELOAD, "&Reload");
	AppendMenu(gui->hmenuTray, MF_STRING, ID_HIDECONSOLE, "&Hide Console");
	AppendMenu(gui->hmenuTray, MF_STRING, ID_EXIT, "E&xit");

	dta.cbSize = sizeof(NOTIFYICONDATA);
	dta.hWnd = hwnd;
	dta.uID = ID_TRAY;
	dta.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
	dta.uCallbackMessage = WM_SHELLNOTIFY;
	dta.hIcon = LoadIcon(gui->hInstance, MAKEINTRESOURCE(ID_ICON1));
	strcpy(dta.szTip, "PegLeg");
	//strcat(dta.szTip, " ");
	//strcat(dta.szTip, D2CS_VERSION_STRING);
	Shell_NotifyIcon(NIM_ADD, &dta);
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

static void OnCommand(HWND hwnd, int id)
					  //, HWND hwndCtl, UINT codeNotify)
{
	if( id == ID_EXIT ) {
		Shutdown();
	}
	else if (id == ID_SHOWCONSOLE )
		ShowWindow(gui->hwnd, SW_SHOW);
	else if (id == ID_RELOAD )
		PostThreadMessage(mainThreadId,WM_COMMAND, RELOADLUA, NULL);
	else if (id == ID_HIDECONSOLE )
		ShowWindow(gui->hwnd, SW_HIDE);
}

static int OnShellNotify(HWND hwnd, int uID, int uMessage)
{
    if(uID == ID_TRAY) {
        if(uMessage == WM_RBUTTONDOWN) {
			POINT cp;
			GetCursorPos(&cp);
			SetForegroundWindow(gui->hwnd);
			TrackPopupMenu(gui->hmenuTray, TPM_LEFTALIGN|TPM_LEFTBUTTON, cp.x, cp.y, 0, gui->hwnd, NULL);
		}
    }
    return 0;
}

static int OnHotkey(HWND hwnd , WPARAM wParam , LPARAM lParam) {
	int id = (int)wParam;
	id = 0;
	return 0;
}

LRESULT CALLBACK WndProc (HWND hwnd , UINT msg,WPARAM wParam , LPARAM lParam)
{
	//COLORREF clr = RGB(0, 0, 0);
	switch(msg) {
		
		case WM_CREATE:
			return OnCreate(hwnd);
		case WM_CLOSE:
			return OnClose(hwnd);
		case WM_SIZE:
			return 0;
		case WM_HOTKEY:
			OnHotkey(hwnd, wParam, lParam);
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_COMMAND:
			OnCommand(hwnd, (int)wParam);
			return 0;
		case WM_SHELLNOTIFY:
			return OnShellNotify(hwnd, wParam, lParam);
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
	//return 0;
}

void guiAddText(const char *str)
{
	COLORREF clr = RGB(gui->text_r,gui->text_g,gui->text_b);
	int start_lines, text_length, end_lines;
	CHARRANGE cr;
	CHARRANGE ds;
	CHARFORMAT fmt;
	text_length = SendMessage(gui->hwndConsole, WM_GETTEXTLENGTH, 0, 0);
	
	if ( text_length >30000 ) {
		ds.cpMin = 0;
		ds.cpMax = text_length - 30000;
		SendMessage(gui->hwndConsole, EM_EXSETSEL, 0, (LPARAM)&ds);
		SendMessage(gui->hwndConsole, EM_REPLACESEL, FALSE, 0);
	}
	
	cr.cpMin = text_length;
	cr.cpMax = text_length;
	
	SendMessage(gui->hwndConsole, EM_EXSETSEL, 0, (LPARAM)&cr); 
	
	fmt.cbSize = sizeof(CHARFORMAT);
	fmt.dwMask = CFM_COLOR|CFM_FACE|CFM_SIZE|CFM_BOLD|CFM_ITALIC|CFM_STRIKEOUT|CFM_UNDERLINE;
	fmt.yHeight = 160;
	fmt.dwEffects = 0;
	fmt.crTextColor = clr;
	strcpy(fmt.szFaceName,"Courier New");
	
	SendMessage(gui->hwndConsole, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&fmt);
	SendMessage(gui->hwndConsole, EM_REPLACESEL, FALSE, (LPARAM)str);
}

int gui_printf( const char *format, va_list arglist )
{
	char buff[4096];	
	int result = vsprintf(buff, format, arglist);
	guiAddText(buff);
	return result;
}