#ifndef _luaf_included
#define _luaf_included

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <windows.h>
#include <psapi.h>
#include <richedit.h>
#include "gui.h"
#include "inputemu.h"

#pragma comment( lib, "psapi.lib" )

extern lua_State *L;

int CreateLua();
int InitLua();
int ReloadLua();
int LoadScript(lua_State *L, const char* filename);
int luaB_print (lua_State *L);
int l_RegisterEvent(lua_State *L);
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

int l_ShowHUDCP( lua_State *L );
int l_HideHUDCP( lua_State *L );

#endif