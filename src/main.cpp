#define _CRT_SECURE_NO_WARNINGS
#include <stdarg.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <process.h>
#include <time.h>

extern "C" {
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
}

#include "pegleg.h"
#include "gui.h"
#include "lua_func.h"
#include <richedit.h>

#pragma comment( lib, "lua5.2.lib" )
#pragma comment( lib, "Winmm.lib" )

#define EXTRACTBIT(var, index) ((var >> index) & 1)

int events[12][MAX_EVENTS];
int timerMap[MAX_EVENTS];

HINSTANCE g_hInstance;
HHOOK hhkLowLevelKeyboard = 0;
HHOOK hhkLowLevelMouse = 0;
HHOOK hhkActivate = 0;
#ifndef XINPUT

//#define DIRECTINPUT_VERSION 0x0800
LPDIRECTINPUT8 g_pDI;
LPDIRECTINPUTDEVICE8 g_pJoystick;

DIJOYSTATE2 js;
DWORD btnState;
char *g_GamepadButtonNames[17] = {
	"X", //Rect
	"A", //Cross
	"B", //Circle
	"Y", //Triangle
	"L1",
	"R1",
	"L2", //X rot
	"R2", //Y rot
	"BACK", //Select, Share
	"START", //Options
	"LS",
	"RS",
	"PS",
	"TOUCHPAD",
	(char)NULL
};
#else
XINPUT_STATE g_ControllerState;
char *g_GamepadButtonNames[17] = {
	"UP",
	"DOWN",
	"LEFT",
	"RIGHT",
	"START",
	"BACK",
	"LS",
	"RS",
	"L1",
	"R1",
	"UB1",
	"UB2",
	"A",
	"B",
	"X",
	"Y",
	(char)NULL
};
#endif

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

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
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

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam){
	/*if (...)
		HWND hNewActiveWnd = (HWND)lParam;
		char *Title = (char *)malloc(sizeof(char)*200);
		GetWindowText(hNewActiveWnd,Title,200);
		for (int i=0; events[ONCREATE][i] && i < MAX_EVENTS; i++ )
			FireEvent(L,events[ONCREATE][i], Title, 0, 0);
		free(Title);
	}*/
	return CallNextHookEx(NULL, nCode, wParam,lParam);
}

void error (lua_State *L, const char *fmt, ...) {
	va_list argp;
	va_start(argp, fmt);
	
	if (gui.hwndConsole){
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
#ifndef _DEBUG
	UnhookWindowsHookEx(hhkLowLevelKeyboard);
	//UnhookWindowsHookEx(hhkLowLevelMouse);
#endif
	if (gui.createtray)
		KillTrayIcon(gui.hwnd);
	if (gui.hwnd)
		DestroyWindow  (gui.hwnd);
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
	}else if (vkCode){ //for gamepad events
		lua_pushinteger(L, vkCode);
		return ExecuteCallback(1);
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

#ifndef XINPUT
BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext )
{
    HRESULT hr;

    // Obtain an interface to the enumerated joystick.
    hr = g_pDI->CreateDevice( pdidInstance->guidInstance, &g_pJoystick, NULL );

	if( FAILED( hr = g_pJoystick->SetDataFormat( &c_dfDIJoystick2 ) ) )
        return hr;

    if( FAILED( hr = g_pJoystick->SetCooperativeLevel( NULL, DISCL_NONEXCLUSIVE| DISCL_BACKGROUND ) ) )
        return hr;

    if( FAILED( hr ) )
        return DIENUM_CONTINUE;

    return DIENUM_STOP;
	//return DIENUM_CONTINUE;
}
#endif

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){

#ifdef _DEBUG
	AllocConsole();
	HWND h;
	h=GetConsoleWindow();
	SetActiveWindow(h);
	int fd = _open_osfhandle( (long)GetStdHandle( STD_OUTPUT_HANDLE ), 0);
	FILE *fp = _fdopen( fd, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );
#endif

	mainThreadId = GetCurrentThreadId();
	g_hInstance = hInstance;


	L = luaL_newstate();
	LoadScript(L, "pegleg.keys.lua");
	lua_getglobal(L, "code_name");

	ZeroMemory(&VKEYS, sizeof(char)*255*15);
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

	//gui = (gui_struct *)malloc(sizeof(gui_struct));

	lua_getglobal(L, "console");
	lua_getfield(L, -1, "enabled");
	int CreateConsole = lua_toboolean(L, -1);
	lua_pop(L,1);
	lua_getfield(L, -1, "tray");
	gui.createtray = lua_toboolean(L, -1);
	lua_pop(L,1);
	lua_getfield(L, -1, "width");
	gui.width = lua_tointeger(L, -1);
	lua_pop(L,1);
	lua_getfield(L, -1, "height");
	gui.height = lua_tointeger(L, -1);
	lua_pop(L,1);
	



	if (CreateConsole) {
		gui.event_ready = CreateEvent(NULL, FALSE, FALSE, NULL);
		_beginthread( guiThread, 0, (void *)hInstance );
		WaitForSingleObject( gui.event_ready, 5000 );
	}

//#ifndef _DEBUG
	lua_getglobal(L, "KEYBOARD_HOOKS_DISABLED");
	if (!lua_toboolean(L, -1)){
		hhkLowLevelKeyboard = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);
	}
	//hhkLowLevelMouse  = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, hInstance, 0);
	//hhkActivate =  SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)GetMsgProc, hInstance, 0);
//#endif

	InitLua();

	double pollingTimeout = 0.015;
	double prevPollTime = 0;
	double now = 0;
	double lastDeviceCheck = 0;
#ifndef XINPUT
	HRESULT hr;
    // Create a DInput object
	g_pJoystick = NULL;
	g_pDI = NULL;
    if( FAILED( hr = DirectInput8Create( GetModuleHandle( NULL ), DIRECTINPUT_VERSION,
                                         IID_IDirectInput8, ( VOID** )&g_pDI, NULL ) ) )
        return hr;
#else
	DWORD prevPacketNumber = 0;
	DWORD ControllerID = 0;
	ZeroMemory( &g_ControllerState, sizeof(XINPUT_STATE) );
	XINPUT_STATE prevControllerState;
#endif
	DWORD prevJoyButtonState = 0;
	DWORD changedJoyButtons = 0;
	DWORD highestBit = 1 << 31;


	MSG msg;
	while(1){
		//while (GetMessage(&msg, NULL, 0, 0) != 0) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_HOTKEY)
				FireEvent(L, events[HOTKEY][(int)msg.wParam], 0, 0, 0);
			/*else if (msg.message == WM_TIMER) {
				int tID = (int)msg.wParam;
				int i = 0;
				char str[255];
				sprintf(str, ">>> %i, %i", tID, i);
				guiAddText(str);
				for (; timerMap[i]; i++)
					if (timerMap[i] == tID) {
						FireEvent(L, events[TIMER][i], 0, 0, 0);
						break;
					}
			}*/
			else if (msg.message == WM_COMMAND){
				int cmdID = (int)msg.wParam;
				if (cmdID == RELOADLUA)
					ReloadLua();
			}
			//TranslateMessage(&msg);
			//DispatchMessage(&msg);
		}

		now = (double)clock()/CLOCKS_PER_SEC;
		if (now > prevPollTime + pollingTimeout){
			prevPollTime = now;

			ProcTimers( (int)(pollingTimeout*1000) );

#ifndef XINPUT
			if (g_pJoystick == NULL || FAILED( hr = g_pJoystick->Acquire()))  {
				if (now > lastDeviceCheck + 1){ //2s timeouts
					lastDeviceCheck = now;
					g_pJoystick = NULL;
					hr = g_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, NULL, DIEDFL_ATTACHEDONLY );
				}
			} else {
				hr = g_pJoystick->Poll();
				hr = g_pJoystick->GetDeviceState( sizeof( DIJOYSTATE2 ), &js );
				hr = g_pJoystick->Unacquire();

				btnState=0;
				for (int i = 0; i<32; i++) {
					btnState = btnState >> 1;
					if (js.rgbButtons[i] == 128) {
						btnState += highestBit;
					}
				}

				for (int i=0; events[JOYUPDATE][i] && i < MAX_EVENTS; i++ )
						FireEvent(L, events[JOYUPDATE][i], 0, 0, 0);


				changedJoyButtons = prevJoyButtonState ^ btnState;
				prevJoyButtonState = btnState;

				if (changedJoyButtons){
					for (int btn=0;btn<32; btn++){
						if (EXTRACTBIT(changedJoyButtons, btn)){
							if (EXTRACTBIT(prevJoyButtonState, btn)){ //it's inverted
								for (int i=0; events[JOYBUTTONDOWN][i] && i < MAX_EVENTS; i++ )
									FireEvent(L, events[JOYBUTTONDOWN][i], g_GamepadButtonNames[btn], btn+1, 0);
							}else{
								for (int i=0; events[JOYBUTTONUP][i] && i < MAX_EVENTS; i++ )
									FireEvent(L, events[JOYBUTTONUP][i], g_GamepadButtonNames[btn], btn+1, 0);
							}
						}
					}
				}
			}
#else
			 //ZeroMemory( &ControllerState, sizeof(XINPUT_STATE) );
			DWORD error = XInputGetState(ControllerID, &g_ControllerState);
			if ( error  == ERROR_SUCCESS ) {
				//if (g_ControllerState.dwPacketNumber != prevPacketNumber) {
					prevPacketNumber = g_ControllerState.dwPacketNumber;

					for (int i=0; events[JOYUPDATE][i] && i < MAX_EVENTS; i++ )
						FireEvent(L, events[JOYUPDATE][i], 0, 0, 0);

					changedJoyButtons = prevJoyButtonState ^ g_ControllerState.Gamepad.wButtons;
					prevJoyButtonState = g_ControllerState.Gamepad.wButtons;
					if (changedJoyButtons){
						for (int btn=0;btn<20; btn++){
							if (EXTRACTBIT(changedJoyButtons, btn)){
								if (EXTRACTBIT(prevJoyButtonState, btn)){ // it's inverted
									for (int i=0; events[JOYBUTTONDOWN][i] && i < MAX_EVENTS; i++ )
										FireEvent(L, events[JOYBUTTONDOWN][i], g_GamepadButtonNames[btn], btn+1, 0);
								}else{
									for (int i=0; events[JOYBUTTONUP][i] && i < MAX_EVENTS; i++ )
										FireEvent(L, events[JOYBUTTONUP][i], g_GamepadButtonNames[btn], btn+1, 0);
								}
							}
						}
					}
				//}
			} else {
				if (error == ERROR_DEVICE_NOT_CONNECTED){
					ControllerID++;
					if (ControllerID > 3) ControllerID = 0;
				}
				//g_ControllerState.Gamepad. = NULL;
			}
#endif
		}

		Sleep(1);
	}

	UnhookWindowsHookEx(hhkLowLevelKeyboard);
	EnableMouseHooks(0);
	//UnhookWindowsHookEx(hhkLowLevelMouse);
	lua_close(L);

	return 1;
}


