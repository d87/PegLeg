#define _CRT_SECURE_NO_WARNINGS
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "pegleg.h"
#include "gui.h"
#include "lua_func.h"
#include <richedit.h>

#pragma comment( lib, "lua5.1.lib" )

int events[8][MAX_EVENTS];
int timerMap[MAX_EVENTS];

HHOOK hhkLowLevelKeyboard;
HHOOK hhkLowLevelMouse;

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
		//free(p);
	}
			
  }
  return(block ? 1 : CallNextHookEx(NULL, nCode, wParam,lParam));
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM
lParam)
{
	if (nCode == HC_ACTION) 
	{	
		int trig = -1;
		int vk = 0;
		switch (wParam) 
		{
			case WM_LBUTTONDOWN: {	trig = MDOWN; vk = 1; break;	}
			case WM_LBUTTONUP: { trig = MUP; vk = 1; break;	}

			case WM_RBUTTONDOWN:  {	trig = MDOWN; vk = 2; break;	}
			case WM_RBUTTONUP:  {	trig = MUP; vk = 2; break;	}

			case WM_MBUTTONDOWN:  {	trig = MDOWN; vk = 4; break;	}
			case WM_MBUTTONUP:  {	trig = MUP; vk = 4; break;	}

			case WM_XBUTTONDOWN: 
				{
					PMSLLHOOKSTRUCT p = (PMSLLHOOKSTRUCT) lParam;
					trig = MDOWN;
					
					int btn4or5 = (( (GET_XBUTTON_WPARAM ( p->mouseData ) == XBUTTON2) && 1 ) || ((GET_XBUTTON_WPARAM ( p->mouseData ) == XBUTTON1) && 0 ));
					vk = 5 + btn4or5;
					break;
				}
			case WM_XBUTTONUP:
				{
					PMSLLHOOKSTRUCT p = (PMSLLHOOKSTRUCT) lParam;
					trig = MUP;
					vk = 4 + p->mouseData;
					break;
				}

			case WM_MOUSEMOVE:
				{
					trig = MMOVE;
					vk = 0x100;
					break;
				}
		}
		if (trig != -1) {
	
		PMSLLHOOKSTRUCT p = (PMSLLHOOKSTRUCT) lParam;
		for (int i=0; events[trig][i] && i < MAX_EVENTS; i++ )
		{	
			FireEvent(L,events[trig][i],VKEYS[vk], p->pt.x, p->pt.y);
		}
	}
	}
	
  return(CallNextHookEx(NULL, nCode, wParam,lParam));
  //return(fEatKeystroke ? 1 : CallNextHookEx(NULL, nCode, wParam,lParam));
}

void error (lua_State *L, const char *fmt, ...) {
	va_list argp;
	va_start(argp, fmt);
	
	if (gui->hwndConsole)
		gui_printf(fmt, argp);
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


int FireEvent(lua_State *L, int index, char * VK_NAME, int vkCode, int scanCode) {
	lua_rawgeti(L, LUA_REGISTRYINDEX, index);
	int args_num;
	if (VK_NAME) {
		lua_pushstring(L, VK_NAME);
		lua_pushinteger(L, vkCode);
		lua_pushinteger(L, scanCode);
		args_num = 3;
	} else args_num = 0;
	if (lua_pcall(L, args_num, 1, 0) != 0)
        error(L, "error running function: %s", lua_tostring(L, -1));
	int fBlock = 1;
	if (lua_isnil(L,-1)){
		fBlock = 0;
	}
	lua_pop(L, 1);
	return fBlock;
	/*if (lua_isstring(L, -1)){
		const char * cmd = lua_tostring(L, -1);
		if (!strcmp(cmd,"block")) return 1;
	}
	lua_pop(L, 1);*/
	//int scan = lua_tointeger(L, -1);LUA_API void lua_call (lua_State *L, int nargs, int nresults) {

	//if ( scan == scanCode) MessageBox(0,"fucked","yeah",0);
	return 0;
}
/*
void eventThread(void *arglist) {
	struct event_arglist * args = (event_arglist*)arglist;
	lua_rawgeti(args->L, LUA_REGISTRYINDEX, args->index);
	lua_pushstring(args->L, args->VK_NAME);
	lua_pushinteger(args->L, args->vkCode);
	lua_pushinteger(args->L, args->scanCode);
	if (lua_pcall(args->L, 3, 0, 0) != 0)
	//if (lua_pcall(args->L, 3, 1, 0) != 0)
        error(args->L, "error run6ning function: %s", lua_tostring(args->L, -1));
	free(args);
	//if (lua_isstring(L, -1)){
	//	const char * cmd = lua_tostring(L, -1);
	//	if (!strcmp(cmd,"block")) return 1;
	//}
	//lua_pop(args->L, 1);

	_endthread();
}

int FireEvent(lua_State *L, int index, char * VK_NAME, int vkCode, int scanCode) {
	//gui = (gui_struct *)malloc(sizeof(gui_struct));
	struct event_arglist * arglist = (event_arglist *)malloc(sizeof(event_arglist));
	//struct event_arglist arglist;
	arglist->L = L;
	arglist->index = index;
	arglist->VK_NAME = VK_NAME;
	arglist->vkCode = vkCode;
	arglist->scanCode = scanCode;

	_beginthread(eventThread, 0, (void*) arglist);

	
	//int scan = lua_tointeger(L, -1);LUA_API void lua_call (lua_State *L, int nargs, int nresults) {

	//if ( scan == scanCode) MessageBox(0,"fucked","yeah",0);
	return 0;
}
*/

//int FireHotkey(lua_State *L, int index) {}

/*void OnUpdateThread( void *param){
	while (1) {
		for (int i=0; events[ONUPDATE][i] && i < MAX_EVENTS; i++ )
			FireEvent(L,events[ONUPDATE][i],0, 0, 0);
		Sleep(200);
	}
	return;
}*/

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	mainThreadId = GetCurrentThreadId();

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
	//UnhookWindowsHookEx(hhkLowLevelMouse);
	lua_close(L);

  return 1;
}




 //fd = malloc(sizeof(WIN32_FIND_DATA));
  //fh = FindFirstFile((LPCSTR) path,fd);
//if((0 != strcmp(fd->cFileName,".")) && (0 != strcmp(fd->cFileName,"..")))
//while(FindNextFile(fh,fd));
 //FindClose(fh);