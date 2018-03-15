#pragma once

#define _ALLOW_RTCc_IN_STL
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
#define REPL_EVAL 0xBBB

#define XINPUT

#include "soundplayer.h"
#include "VirtualDesktopControl.h"
#include "gamepad.h"
#include "repl.h"
#include "lua_func.h"


#include <Xinput.h>
//#pragma comment( lib, "XINPUT.lib" )
#pragma comment( lib, "XINPUT9_1_0.LIB" ) // for vs2012 sdk


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


extern int events[12][MAX_EVENTS];
extern int timerMap[MAX_EVENTS];
extern int mainThreadId;
extern HHOOK hhkLowLevelKeyboard;
extern HHOOK hhkLowLevelMouse;
extern SoundPlayer *soundplayer;
extern VirtualDesktopControl *pVirtualDesktopControl;
extern GamepadGroup *g_gamepadGroup;
extern REPL *repl;

void EnableMouseHooks(bool enable);
void error (lua_State *L, const char *fmt, ...);
int Shutdown();
int FireEvent(lua_State *L, int index, char * VK_NAME, int vkCode, int scanCode);
int FireMouseEvent(lua_State *L, int index, int Button, int x, int y);
static int l_RegisterEvent(lua_State *L);