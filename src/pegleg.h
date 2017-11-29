#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#define KDOWN 0
#define KUP 1
#define MDOWN 2
#define MUP 3
#define MMOVE 4
#define ONCREATE 5
#define TIMER 6
#define HOTKEY 7
#define ACTIVATION 8
#define JOYUPDATE 9
#define JOYBUTTONDOWN 10
#define JOYBUTTONUP 11
#define MAX_EVENTS 32

#define RELOADLUA 0xAAA

#define XINPUT

#ifndef XINPUT

#include <dinput.h>
#include <dinputd.h>
#pragma comment( lib, "dinput8.lib")
#pragma comment( lib, "dxguid.lib")

#else

#include <Xinput.h>
//#pragma comment( lib, "XINPUT.lib" )
#pragma comment( lib, "XINPUT9_1_0.LIB" ) // for vs2012 sdk

#endif

struct enum_struct {
//	int type;
	char *pattern;
	char buffer[200];
	HWND hwndReturned;
};

struct event_arglist {
	lua_State *L;
	int index;
	char * VK_NAME;
	int vkCode;
	int scanCode;
};


#ifndef XINPUT
extern JOYINFOEX g_joyInfo[4];
extern DIJOYSTATE2 js;
extern LPDIRECTINPUT8          g_pDI;
extern LPDIRECTINPUTDEVICE8    g_pJoystick;
extern DWORD btnState;
#else

//#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE 9000
extern XINPUT_STATE g_ControllerState;
extern DWORD ControllerID;
#endif
extern char *g_GamepadButtonNames[17];

extern int events[12][MAX_EVENTS];
extern int timerMap[MAX_EVENTS];
extern int mainThreadId;
extern HHOOK hhkLowLevelKeyboard;
extern HHOOK hhkLowLevelMouse;

void EnableMouseHooks(bool enable);
void error (lua_State *L, const char *fmt, ...);
int Shutdown();
int FireEvent(lua_State *L, int index, char * VK_NAME, int vkCode, int scanCode);
int FireMouseEvent(lua_State *L, int index, int Button, int x, int y);
static int l_RegisterEvent(lua_State *L);