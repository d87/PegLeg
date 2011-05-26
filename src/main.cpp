#define _CRT_SECURE_NO_WARNINGS
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>

extern "C" {
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
}

#include "pegleg.h"
#include "gui.h"
#include "lua_func.h"
#include <richedit.h>

#pragma comment( lib, "lua5.1.lib" )

int events[8][MAX_EVENTS];
int timerMap[MAX_EVENTS];

HINSTANCE g_hInstance;
HHOOK hhkLowLevelKeyboard = 0;
HHOOK hhkLowLevelMouse = 0;

int mainThreadId;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  int block = 0;
  if (nCode == HC_ACTION) 
  {
	int trig = -1;
    switch (wParam) 
	{
		case WM_KEYDOWN:  
		case WM_SYSKEYDOWN:
			{ 
				trig = KDOWN; break;
			}
		case WM_KEYUP:    
		case WM_SYSKEYUP:
			{
				trig = KUP; break;
			}
    }
	if (trig != -1) {
	
		PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT) lParam;
		for (int i=0; events[trig][i] && i < MAX_EVENTS; i++ )
		{	
			if (FireEvent(L,events[trig][i],VKEYS[p->vkCode], (int)p->vkCode,(int)p->scanCode))
				block = 1;
		}
	}
			
  }
  return(block ? 1 : CallNextHookEx(NULL, nCode, wParam,lParam));
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM
lParam)
{
	int block = 0;
	if (nCode == HC_ACTION) 
	{	
		int trig = -1;
		int btn = 0;
		switch (wParam) 
		{
			case WM_LBUTTONDOWN: {	trig = MDOWN; btn = 1; break;	}
			case WM_LBUTTONUP: { trig = MUP; btn = 1; break;	}

			case WM_RBUTTONDOWN:  {	trig = MDOWN; btn = 2; break;	}
			case WM_RBUTTONUP:  {	trig = MUP; btn = 2; break;	}

			case WM_MBUTTONDOWN:  {	trig = MDOWN; btn = 3; break;	}
			case WM_MBUTTONUP:  {	trig = MUP; btn = 3; break;	}

			case WM_XBUTTONDOWN: 
				{
					PMSLLHOOKSTRUCT p = (PMSLLHOOKSTRUCT) lParam;
					trig = MDOWN;
					
					//int btn4or5 = 
					//(( (GET_XBUTTON_WPARAM ( p->mouseData ) == XBUTTON2) && 1 ) || ((GET_XBUTTON_WPARAM ( p->mouseData ) == XBUTTON1) && 0 ));
					btn = 3 + GET_XBUTTON_WPARAM ( p->mouseData );
					break;
				}
			case WM_XBUTTONUP:
				{
					PMSLLHOOKSTRUCT p = (PMSLLHOOKSTRUCT) lParam;
					trig = MUP;
					btn = 3 + GET_XBUTTON_WPARAM ( p->mouseData );
					break;
				}

			/*case WM_MOUSEMOVE:
				{
					trig = MMOVE;
					btn = 0xFF;
					break;
				}*/
		}
		if (trig != -1){
			PMSLLHOOKSTRUCT p = (PMSLLHOOKSTRUCT) lParam;
			for (int i=0; events[trig][i] && i < MAX_EVENTS; i++ ){
				if (FireMouseEvent(L,events[trig][i],btn, p->pt.x, p->pt.y))
					block = 1;
			}
		}
	}
	
	return(block ? 1 : CallNextHookEx(NULL, nCode, wParam,lParam));
}

void error (lua_State *L, const char *fmt, ...) {
	va_list argp;
	va_start(argp, fmt);
	
	if (gui->hwndConsole){
		gui_printf(fmt, argp);
		gui_printf("\n",0);
	}
	else {
		FILE *f = fopen("error.log","a");
		vfprintf(f, fmt, argp);
		fclose(f);
	}
	va_end(argp);
	//lua_close(L);
	//exit(EXIT_FAILURE);
} 

int Shutdown() {
	UnhookWindowsHookEx(hhkLowLevelKeyboard);
	//UnhookWindowsHookEx(hhkLowLevelMouse);
	if (gui->createtray)
		KillTrayIcon(gui->hwnd);
	if (gui->hwnd)
		DestroyWindow  (gui->hwnd);
	exit(1);
	return 1;
}

int ExecuteCallback(int arg_num){
	if (lua_pcall(L, arg_num, 1, 0) != 0)
        error(L, "error running function: %s", lua_tostring(L, -1));
	int isnil = 1;
	if (lua_isnil(L,-1)){
		isnil = 0;
	}
	lua_pop(L, 1);
	return isnil;
}
int FireEvent(lua_State *L, int index, char * VK_NAME, int vkCode, int scanCode) {
	lua_rawgeti(L, LUA_REGISTRYINDEX, index);
	if (VK_NAME) {
		lua_pushstring(L, VK_NAME);
		lua_pushinteger(L, vkCode);
		lua_pushinteger(L, scanCode);
		return ExecuteCallback(3);
	}
	return ExecuteCallback(0);
}
int FireMouseEvent(lua_State *L, int index, int Button, int x, int y) {
	lua_rawgeti(L, LUA_REGISTRYINDEX, index);
	//MOUSEMOVE//if (Button == 0xFF) ... 
	lua_pushinteger(L, Button);
	lua_pushinteger(L, x);
	lua_pushinteger(L, y);
	return ExecuteCallback(3);
}
void EnableMouseHooks(bool enable){
	if (!hhkLowLevelMouse && enable)
		hhkLowLevelMouse  = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, g_hInstance, 0);
	else if (hhkLowLevelMouse){
		UnhookWindowsHookEx(hhkLowLevelMouse);
		hhkLowLevelMouse = 0;
	}
}



int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	mainThreadId = GetCurrentThreadId();
	g_hInstance = hInstance;

	L = luaL_newstate();
	LoadScript(L, "pegleg.keys.lua");
	lua_getglobal(L, "code_name");
	for (int i=1; i<255; i++) {
		lua_pushnumber(L, i);
		lua_gettable(L, -2);
		if (lua_isstring(L, -1))
			strcpy(VKEYS[i],lua_tostring(L, -1));
		lua_pop(L, 1);
	}
	lua_close(L);



	CreateLua();

	LoadScript(L, "pegleg.conf.lua");

	gui = (gui_struct *)malloc(sizeof(gui_struct));

	lua_getglobal(L, "console");
	lua_getfield(L, -1, "enabled");
	int CreateConsole = lua_toboolean(L, -1);
	lua_pop(L,1);
	lua_getfield(L, -1, "tray");
	gui->createtray = lua_toboolean(L, -1);
	lua_pop(L,1);
	lua_getfield(L, -1, "width");
	gui->width = lua_tointeger(L, -1);
	lua_pop(L,1);
	lua_getfield(L, -1, "height");
	gui->height = lua_tointeger(L, -1);
	lua_pop(L,1);
	



	if (CreateConsole) {
		gui->event_ready = CreateEvent(NULL, FALSE, FALSE, NULL);
		_beginthread( guiThread, 0, (void *)hInstance );
		WaitForSingleObject( gui->event_ready, 5000 );
	}

	hhkLowLevelKeyboard  = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);
	//hhkLowLevelMouse  = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, hInstance, 0);

	InitLua();
	
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) != 0) {
		if (msg.message == WM_HOTKEY)
			FireEvent(L, events[HOTKEY][(int)msg.wParam], 0, 0, 0);
		else if (msg.message == WM_TIMER) {
			int tID = (int)msg.wParam;
			int i = 0;
			for (; timerMap[i]; i++)
				if (timerMap[i] == tID) {
					FireEvent(L, events[TIMER][i], 0, 0, 0);
					break;
				}
		}
		else if (msg.message == WM_COMMAND){
			int cmdID = (int)msg.wParam;
			if (cmdID == RELOADLUA)
				ReloadLua();
		}
	}

	UnhookWindowsHookEx(hhkLowLevelKeyboard);
	EnableMouseHooks(0);
	//UnhookWindowsHookEx(hhkLowLevelMouse);
	lua_close(L);

  return 1;
}