#ifndef _luaf_included
#define _luaf_included

extern "C" {
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
}
#include <windows.h>
#include <psapi.h>
#include <richedit.h>
#include "gui.h"
#include "inputemu.h"

#pragma comment( lib, "psapi.lib" )

extern lua_State *L;

struct PLFrame {
	int invterval;
	int data;
	//int interval;
};

void ProcTimers(int elapsed);
int CreateLua();
int InitLua();
int ReloadLua();
int LoadScript(lua_State *L, const char* filename);
int luaB_print (lua_State *L);
int l_RegisterEvent(lua_State *L);
int l_UnregisterEvent(lua_State *L);
int l_shell ( lua_State *luaVM );
int l_Start ( lua_State *luaVM );
int l_Shutdown ( lua_State *luaVM );
int l_GetWindowTitle( lua_State *luaVM );
int l_GetWindowProcess( lua_State *L );
int l_AddScript( lua_State *luaVM );
int l_ShowConsole( lua_State *luaVM );
int l_HideConsole( lua_State *luaVM );
int l_ClearConsole( lua_State *luaVM );
int l_SetConsoleBackgroundColor( lua_State *L );
int l_SetConsoleTextColor( lua_State *L );
int l_IsPressed( lua_State *L );
int l_GetCursorPos( lua_State *L );
int l_KeyboardInput( lua_State *L);
int l_MouseInput( lua_State *L);
int l_ShowWindow( lua_State *L );
int l_RegisterHotKey( lua_State *L );
int l_CreateTimer( lua_State *L );
int l_KillTimer( lua_State *L );
int l_Reload( lua_State *L );
int l_SetAlwaysOnTop( lua_State *luaVM );
int l_IsAlwaysOnTop( lua_State *luaVM );
int l_EnableMouseHooks( lua_State *luaVM );
int l_DisableMouseHooks( lua_State *luaVM );
int l_OSDTextLong ( lua_State *L );
int l_CreateFrame( lua_State *L);
int l_KillFrame( lua_State *L);
int l_Frame_GetData( lua_State *L);
int l_SetKeyDelay( lua_State *L);
int l_TurnOffMonitor( lua_State *L);
int l_GetJoyPosInfo( lua_State *L);
int l_IsJoyButtonPressed( lua_State *L);

#endif