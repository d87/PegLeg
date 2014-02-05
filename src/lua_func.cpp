#include "lua_func.h"
#include "pegleg.h"

#include <cstdlib>
#include <string>
#include <hash_map>
using namespace std;

struct PLTIMER {
	int interval;
	int remains;
	unsigned int luaFuncRef;
};

hash_map<std::string, PLTIMER> Timers;

lua_State *L = 0;

static const struct luaL_Reg timerlib [] = {
	//{"new", l_CreateNewUserdata},
	{"Kill", l_KillFrame},
	{"GetID", l_Frame_GetData},
	{NULL, NULL}
};

int CreateLua() {
	L = luaL_newstate(); 
	luaL_openlibs(L);

	lua_register(L, "RegisterEvent", l_RegisterEvent);
	lua_register(L, "UnregisterEvent", l_UnregisterEvent);
	lua_register(L, "AddScript", l_AddScript);
	lua_register(L, "Shell", l_shell);
	lua_register(L, "sh", l_shell);
	lua_register(L, "Start", l_Start);
	lua_register(L, "Shutdown", l_Shutdown);
	lua_register(L, "GetWindowTitle", l_GetWindowTitle);
	lua_register(L, "RemoveMenu", l_RemoveMenu);
	lua_register(L, "MouseInput", l_MouseInput);
	lua_register(L, "KeyboardInput", l_KeyboardInput);
	lua_register(L, "GetCursorPos", l_GetCursorPos);
	lua_register(L, "IsPressed", l_IsPressed);
	lua_register(L, "ShowWindow", l_ShowWindow);
	lua_register(L, "RegisterHotKey", l_RegisterHotKey);
	lua_register(L, "GetWindowProcess", l_GetWindowProcess);
	lua_register(L, "CreateTimer", l_CreateTimer);
	lua_register(L, "KillTimer", l_KillTimer);
	lua_register(L, "SetKeyDelay", l_SetKeyDelay);
	lua_register(L, "TurnOffMonitor", l_TurnOffMonitor);
	lua_register(L, "Reload", l_Reload);
	lua_register(L, "SetAlwaysOnTop", l_SetAlwaysOnTop);
	lua_register(L, "IsAlwaysOnTop", l_IsAlwaysOnTop);
	lua_register(L, "EnableMouseHooks", l_EnableMouseHooks);
	lua_register(L, "DisableMouseHooks", l_DisableMouseHooks);
	lua_register(L, "OSDTextLong", l_OSDTextLong);
	lua_register(L, "GetJoyPosInfo", l_GetJoyPosInfo);
	lua_register(L, "IsJoyButtonPressed", l_IsJoyButtonPressed);
	lua_register(L, "SetGamepadVibration", l_SetGamepadVibration);

	lua_register(L, "CreateFrame", l_CreateFrame);
	
	lua_newtable(L);
	lua_setglobal(L, "console");
	lua_getglobal(L, "console");
	lua_pushboolean(L, 1);
	lua_setfield(L, -2, "enabled");
	//lua_pushboolean(L, 0);
	//lua_setfield(L, -2, "init");
	lua_pushnumber(L, 300);
	lua_setfield(L, -2, "width");
	lua_pushnumber(L, 400);
	lua_setfield(L, -2, "height");
	lua_pushcfunction(L, l_SetConsoleBackgroundColor);
	lua_setfield(L, -2, "SetBackgroundColor");
	lua_pushcfunction(L, l_SetConsoleTextColor);
	lua_setfield(L, -2, "SetColor");
	lua_pushcfunction(L, l_ShowConsole);
	lua_setfield(L, -2, "Show");
	lua_pushcfunction(L, l_HideConsole);
	lua_setfield(L, -2, "Hide");
	lua_pushcfunction(L, l_ClearConsole);
	lua_setfield(L, -2, "Clear");
	return 1;
}

int InitLua() {

	if (gui.hwndConsole)
		lua_register(L, "print", luaB_print);

	luaL_newmetatable(L, "PegLeg.timer");
	lua_newtable(L);
	luaL_setfuncs(L, timerlib, 0);
	lua_setglobal(L, "timer");

	LoadScript(L, "pegleg.init.lua");

	for (int i=0; events[ONCREATE][i] && i < MAX_EVENTS; i++ )
		FireEvent(L,events[ONCREATE][i],0, 0, 0);

	return 1;
}

int l_Reload(lua_State *L){
	PostThreadMessage(mainThreadId,WM_COMMAND, RELOADLUA, NULL);
	return 0;
}

int ReloadLua() {
	guiAddText("!Reloading...\n");

	//killing timers and hotkeys
	for (int i=0; events[HOTKEY][i]; i++)
		UnregisterHotKey(NULL, i);
	Timers.clear();
	/*for (int i=0; events[TIMER][i]; i++){
		KillTimer(NULL, timerMap[i]);
		timerMap[i] = 0;
	}*/
	ZeroMemory( &events, sizeof(events) ); // purging old events table

	lua_close(L);

	CreateLua();
	InitLua();
	return 1;
}

int LoadScript(lua_State *L, const char* filename){
	if (luaL_loadfile(L, filename) || lua_pcall(L, 0, 0, 0))
        error(L, "cannot run configuration file: %s\r\n", lua_tostring(L, -1) );
	return 1;
}

int luaB_print (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  int i;
  lua_getglobal(L, "tostring");
  guiAddText(">");
  for (i=1; i<=n; i++) {
    const char *s;
    lua_pushvalue(L, -1);  /* function to be called */
    lua_pushvalue(L, i);   /* value to print */
    lua_call(L, 1, 1);
    s = lua_tostring(L, -1);  /* get result */
    if (s == NULL)
      return luaL_error(L, LUA_QL("tostring") " must return a string to "
                           LUA_QL("print"));
    if (i>1) guiAddText("\t");
    guiAddText(s);
    lua_pop(L, 1);  /* pop result */
  }
  guiAddText("\n");
  return 0;
}

static int l_RegisterEvent(lua_State *L) {
	const char * eventname = luaL_checkstring(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	unsigned int func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	unsigned int trig = 999;
	if		(!strcmp(_strupr((char*)eventname),"MOUSEDOWN")) trig = MDOWN;
	else if (!strcmp(_strupr((char*)eventname),"MOUSEUP")) trig = MUP;
	else if (!strcmp(_strupr((char*)eventname),"KEYDOWN")) trig = KDOWN;
	else if (!strcmp(_strupr((char*)eventname),"KEYUP")) trig = KUP;
	else if (!strcmp(_strupr((char*)eventname),"MOUSEMOVE")) trig = MMOVE;
	else if (!strcmp(_strupr((char*)eventname),"ONCREATE")) trig = ONCREATE;
	else if (!strcmp(_strupr((char*)eventname),"ACTIVATION")) trig = ACTIVATION;
	else if (!strcmp(_strupr((char*)eventname),"JOYUPDATE")) trig = JOYUPDATE;
	else if (!strcmp(_strupr((char*)eventname),"JOYBUTTONDOWN")) trig = JOYBUTTONDOWN;
	else if (!strcmp(_strupr((char*)eventname),"JOYBUTTONUP")) trig = JOYBUTTONUP;
	//else if (!strcmp(_strupr((char*)eventname),"ONUPDATE")) trig = ONUPDATE;
	

	int i=0;
	if (trig != 999) {
		for (; events[trig][i]; i++ );
		if ( i < MAX_EVENTS ) events[trig][i] = func_ref;
		lua_pushnumber(L, (trig << 5)+i );
		return 1;
	}
	return 0;
}
static int l_UnregisterEvent(lua_State *L) {
	//if (!lua_isnumber(L, 1))
	//		error(L, "Invalid arg#1 (expecting number)");
	unsigned int eventID = (int)luaL_checknumber(L, 1);
	unsigned int eventType = eventID >> 5;
	unsigned int eventIndex = eventID - (eventType << 5);

	events[eventType][eventIndex] = 0;
	for (int i = eventIndex+1; i < MAX_EVENTS; i++){
		if (events[eventType][i])
			events[eventType][i-1] = events[eventType][i];
		else{
			events[eventType][i-1] = 0;
			break;
		}
	}

	return 0;
}

static int l_shell ( lua_State *luaVM ) {
	char cmdl[1000];
	sprintf(cmdl,"%s /c %s","cmd.exe",luaL_checkstring(luaVM, 1));
	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
	char *env = 0;
	char *cwd = (char *)luaL_optstring(L, 2, NULL);
		//( lua_isstring(luaVM, 2) ? lua_tostring(luaVM, 2) : NULL );

	int a = CreateProcess(0, cmdl, 0,0, true, CREATE_NEW_CONSOLE, env, cwd, &si, &pi);
	//CREATE_NO_WINDOW
	/*ShellExecute(0,"open",	,
								,
							( lua_isstring(luaVM, 3) ? lua_tostring(luaVM, 3) : "" ),
							( lua_isnumber(luaVM, 4) ? lua_tointeger(luaVM, 4) : SW_SHOW )
							);*/
	
	return 0;
}

static int l_Start ( lua_State *luaVM ) {
	char *cmdl =  (char *)luaL_checkstring(luaVM, 1);
	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
	char *env = 0;
	char *cwd = (char *)luaL_optstring(L, 2, NULL);

	int a = CreateProcess(0, cmdl, 0,0, true, CREATE_NO_WINDOW, env, cwd, &si, &pi);
	return 0;
}
static int l_Shutdown ( lua_State *luaVM ) {
	Shutdown();
	return 0;
}
static int l_GetWindowTitle( lua_State *luaVM ) {
	char *Title = (char *)malloc(sizeof(char)*200);
	GetWindowText(GetForegroundWindow(),Title,200);
	lua_pushstring(luaVM, Title);
	free(Title);
	return 1;
}

static int l_RemoveMenu( lua_State *luaVM ) {
	SetMenu(GetForegroundWindow(), NULL);
	return 0;
}

static int l_IsAlwaysOnTop( lua_State *luaVM ) {
	HWND fgw = GetForegroundWindow();
	DWORD dwExStyle = (DWORD)GetWindowLong(fgw,GWL_EXSTYLE);
	lua_pushboolean(luaVM, ((dwExStyle & WS_EX_TOPMOST)!=0) );
	return 1;
}
static int l_SetAlwaysOnTop( lua_State *luaVM ) {
	int ontop = 1;
	if (lua_isboolean(luaVM,1))
		if (lua_toboolean(luaVM,1) == 0)
			ontop = 0;

	HWND fgw = GetForegroundWindow();
	SetWindowPos(fgw,ontop ? HWND_TOPMOST : HWND_NOTOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
	return 0;
}


static int l_AddScript( lua_State *luaVM ) {
	const char * filename = luaL_checkstring(luaVM, 1);
	LoadScript(luaVM, filename);
	lua_pushboolean( luaVM, 1 );
	return 1;
}

static int l_ShowConsole( lua_State *luaVM ) {
	if (gui.hwnd) {
		ShowWindow(gui.hwnd,SW_SHOW);
		//SetForegroundWindow(gui.hwnd);
	}
	return 0;
}
static int l_HideConsole( lua_State *luaVM ) {
	if (gui.hwnd) ShowWindow(gui.hwnd,SW_HIDE);
	return 0;
}
static int l_ClearConsole( lua_State *luaVM ) {
	SendMessage(gui.hwndConsole,WM_SETTEXT,0,0);
	return 0;
}
static int l_SetConsoleBackgroundColor( lua_State *L ) {
	double r = luaL_checknumber(L, 1);
	double g = luaL_checknumber(L, 2);
	double b = luaL_checknumber(L, 3);
	if (r>1) r = 1; if (r<0) r = 0;
	if (g>1) g = 1; if (g<0) g = 0;
	if (b>1) b = 1; if (b<0) b = 0;
	COLORREF clr = RGB(r*255,g*255,b*255);
	SendMessage(gui.hwndConsole, EM_SETBKGNDCOLOR, 0, (LPARAM)clr);
	return 0;
}

static int l_SetConsoleTextColor( lua_State *L ) {
	double r = luaL_checknumber(L, 1);
	double g = luaL_checknumber(L, 2);
	double b = luaL_checknumber(L, 3);
	if (r>1) r = 1; if (r<0) r = 0;
	if (g>1) g = 1; if (g<0) g = 0;
	if (b>1) b = 1; if (b<0) b = 0;
	gui.text_r = (int)r*255;
	gui.text_g = (int)g*255;
	gui.text_b = (int)b*255;
	return 0;
}

static int l_GetCursorPos( lua_State *L ) {     
    POINT pos;
	GetCursorPos(&pos);
	lua_pushnumber(L, pos.x);
	lua_pushnumber(L, pos.y);
	return 2;
}

static int l_KeyboardInput( lua_State *L){
	KeyboardInput((char *)luaL_checkstring(L, 1));
	return 0;
}

static int l_IsPressed( lua_State *L){
	char * name = (char *)luaL_checkstring(L, 1);
	int vkCode = 0;
	for (int i=1; i<255; i++ ) {
		if (VKEYS[i][0]) {
			if (!strcmp(VKEYS[i],name))
				vkCode = i;
		}
	}
	if (!vkCode) return 1;

	lua_pushboolean(L, GetAsyncKeyState(vkCode) ? 1 : 0);
	return 1;
}

static int l_MouseInput( lua_State *L){
	char * eventname =(char *)luaL_checkstring(L, 1);
	if (!strcmp(_strupr(eventname),"MOVE")) {
		int x = luaL_checkinteger( L, 2);
		int y = luaL_checkinteger( L, 3);
		int absolute = 0;
		if (lua_isboolean(L, 4))
			if (lua_toboolean(L, 4))
				absolute = MOUSEEVENTF_ABSOLUTE;
		MouseInput(eventname, x, y, absolute);
	} else MouseInput(eventname, 0, 0, 0);
	return 0;
}


int __stdcall EWProc(HWND hwnd,LPARAM lParam)
{
	struct enum_struct * wData = (struct enum_struct *)lParam;
	GetWindowText(hwnd,wData->buffer, 200);
	if (strstr(wData->buffer,wData->pattern)) {
		wData->hwndReturned = hwnd;
		return 0;
	}
	return 1;
}
static int l_ShowWindow( lua_State *L ) {
	const char *title = luaL_checkstring(L, 1);
	struct enum_struct wData;
	ZeroMemory(&wData,sizeof(wData));
	wData.pattern = (char *)title;
	EnumWindows(EWProc,(LPARAM)&wData);
	if (wData.hwndReturned) {
		ShowWindow(wData.hwndReturned, SW_SHOW);
		SetForegroundWindow(wData.hwndReturned );
		lua_pushboolean(L, 1);
	} else lua_pushnil(L );

	//BringWindowToTop(wData.hwndReturned);
	return 1;
}


int ParseModifiers(char *ptr) {
	int mods = 0;
	while (*ptr) {
		if (!strncmp(ptr,"CTRL",4)) { mods |= MOD_CONTROL; ptr += 4; }
		if (!strncmp(ptr,"ALT",3)) { mods |= MOD_ALT;  ptr += 3; }
		if (!strncmp(ptr,"SHIFT",5)) {mods |= MOD_SHIFT;  ptr += 5; }
		ptr++;
	}
	return mods;
}
int l_RegisterHotKey( lua_State *L ) {
	const char * modstr = luaL_checkstring(L, 1);
	const char * keyname = luaL_checkstring(L, 2);
	int vk;
	for (vk=0;vk<256;vk++)
		if (!strcmp(VKEYS[vk],keyname))
			break;
	luaL_checktype(L, 3, LUA_TFUNCTION);
	int func_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	char *pEnd;
	int mods = ParseModifiers(_strupr((char *)modstr));

	int i;
	for (i=0; events[HOTKEY][i]; i++ );
	if ( i < MAX_EVENTS ) events[HOTKEY][i] = func_ref;

	RegisterHotKey(NULL, i, mods, vk);
	return 0;
}

static int l_GetWindowProcess( lua_State *L ) {
	unsigned long pid;
	GetWindowThreadProcessId(GetForegroundWindow(), &pid);

	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid );

	if (NULL != hProcess )
    {
		HMODULE hMod;
        DWORD bytesReturned = 0;
		
		// If this function is called from a 32-bit application running on WOW64, it can only enumerate the modules of a 32-bit process. If the process is a 64-bit process, this function fails and the last error code is ERROR_PARTIAL_COPY (299).
        if ( EnumProcessModulesEx( hProcess, &hMod, sizeof(hMod), &bytesReturned, LIST_MODULES_ALL) )
        {
			if (bytesReturned) {
				char *pname = (char *)malloc(sizeof(char)*200);
				GetModuleBaseName( hProcess, hMod, pname, 200 );
				lua_pushstring(L, pname);
				free(pname);
				return 1;
			}
        }
    }
	return 0;
}

/*void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT tID, DWORD dwTime){
		//int tID = (int)msg.wParam;
				int i = 1;
				//char str[255];
				//sprintf(str, "proc>>> %i, %i", tID, i);
				//guiAddText(str);
				for (; timerMap[i]; i++)
					if (timerMap[i] == tID) {
						FireEvent(L, events[TIMER][i], 0, 0, 0);
						break;
					}
}
*/

void ProcTimers(int elapsed){
	for (auto it=Timers.begin(); it!=Timers.end(); it++){
		it->second.remains -= elapsed;
		if (it->second.remains <= 0){
			FireEvent(L, it->second.luaFuncRef, 0, 0, 0);
			it->second.remains += it->second.interval;
		}
	}
}

static int l_CreateTimer( lua_State *L ) {
	/*unsigned int i;
	if (lua_isnumber(L, 1))
		i = (unsigned int)lua_tonumber(L, 1);
	else
		for (i=1; events[TIMER][i]; i++ );*/
	std::string name = (const char*) luaL_checkstring(L, 1);

	unsigned int interval = (unsigned int)luaL_checknumber(L, 2);
	luaL_checktype(L, 3, LUA_TFUNCTION);
	//lua_pushvalue(L, 2);
	unsigned int func_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	if (Timers.find(name) != Timers.end()){
		PLTIMER * t = &Timers[name];
		t->interval = interval;
		t->remains = interval;
		luaL_unref(L, LUA_REGISTRYINDEX, t->luaFuncRef);
		t->luaFuncRef = func_ref;
	}else{
		PLTIMER t;
		t.interval = interval;
		t.remains = interval;
		t.luaFuncRef = func_ref;

		Timers[name] = t;
	}
	
	/*if ( i < MAX_EVENTS ) events[TIMER][i] = func_ref;
	else return 0;

	HWND hWnd = gui.hwnd;

	unsigned int tID = SetTimer(hWnd,i,interval, (TIMERPROC)&TimerProc);
	timerMap[i] = tID;
	lua_pushnumber(L, tID);*/
	return 0;
}

static PLFrame *checkuserdata (lua_State *L, int idx) {
	void *ud = luaL_checkudata(L, idx, "PegLeg.timer");
	luaL_argcheck(L, ud != NULL, idx, "`timer' expected");
	return (PLFrame *)ud;
}
static int l_CreateFrame( lua_State *L ) {
	unsigned int interval = (unsigned int)luaL_checknumber(L, 1);
	unsigned int data = (unsigned int)luaL_checknumber(L, 1);
	//luaL_checktype(L, 2, LUA_TFUNCTION);
	//int func_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	/*int i=0;
	for (; events[TIMER][i]; i++ );
	if ( i < MAX_EVENTS ) events[TIMER][i] = func_ref;
	else return 0;

	int tID = SetTimer(NULL,i,interval,NULL);
	timerMap[i] = tID;*/
	PLFrame *timer = (PLFrame *)lua_newuserdata(L, sizeof(PLFrame));

	luaL_getmetatable(L, "PegLeg.timer");
	lua_setmetatable(L, -2);
	timer->data = data;
	timer->invterval = interval;
	return 1;
}

static int l_Frame_GetData( lua_State *L ) {
	PLFrame *t = checkuserdata(L,1);
	lua_pushnumber(L, t->invterval);
	return 1;
}

static int l_KillFrame( lua_State *L ) {

	return 0;
}

static int l_KillTimer( lua_State *L ) {
	std::string name = (const char*) luaL_checkstring(L, 1);
	int nRemoved = Timers.erase(name);
	/*int tID = (int)luaL_checknumber(L, 1);
	int i = 1;
	HWND hWnd = gui.hwnd;
	for (; timerMap[i]; i++)
		if (timerMap[i] == tID) {
			KillTimer(hWnd, tID);
			luaL_unref(L, LUA_REGISTRYINDEX,events[TIMER][i]);
			events[TIMER][i] = 0;
			timerMap[i] = 0;
			lua_pushboolean(L, 1);
			return 1;
		}*/
	lua_pushnumber(L, nRemoved);
	return 1;
}

static int l_EnableMouseHooks( lua_State *luaVM ) {
	int enable = 1;
	if (lua_isboolean(luaVM,1)){
		if (lua_toboolean(luaVM,1) == 0)
			enable = 0;
	}

	EnableMouseHooks(enable);
	return 0;
}
static int l_DisableMouseHooks( lua_State *luaVM ) {
	EnableMouseHooks(0);
	return 0;
}

static int l_OSDTextLong ( lua_State *L ) {
	const char* text = luaL_checkstring(L, 1);
	const int _x = luaL_checkinteger(L, 2);
	const int _y = luaL_checkinteger(L, 3);
	double r = ( lua_isnumber(L, 4) ? lua_tonumber(L, 4) : 1 );
	double g = ( lua_isnumber(L, 5) ? lua_tonumber(L, 5) : 1 );
	double b = ( lua_isnumber(L, 6) ? lua_tonumber(L, 6) : 1 );
	const char* font = ( lua_isstring(L, 7) ? lua_tostring(L, 7) : "Trebuchet MS" );
	int size = ( lua_isnumber(L, 8) ? lua_tointeger(L, 8) : 25 );

	COLORREF color;
	HDC R;

	if (r>1) r = 1; if (r<0) r = 0;
	if (g>1) g = 1; if (g<0) g = 0;
	if (b>1) b = 1; if (b<0) b = 0;

	color = RGB(r*255,g*255,b*255);
	HWND Desktop = GetDesktopWindow();
	R = GetWindowDC(GetDesktopWindow());
	SetBkMode(R,OPAQUE);//TRANSPARENT);
	SetBkColor(R,RGB(0,0,0));
	SetTextColor(R,color);
	SelectObject(R, CreateFont(size,0,0,0,0,0,0,0,1,0,0,0,0,font) );
	TextOutA(R,_x,_y,text,strlen(text));
	ReleaseDC(0,R);
	lua_pushnumber ( L, 1);
	return 1;
}


static int l_SetKeyDelay ( lua_State *L ) {
	const int delay = luaL_checkinteger(L, 1);
	const int repeat = luaL_checkinteger(L, 2);
	FILTERKEYS keys;
	keys.cbSize = sizeof(FILTERKEYS);
	keys.iDelayMSec = delay;
	keys.iRepeatMSec = repeat;
	keys.dwFlags = FKF_FILTERKEYSON|FKF_AVAILABLE;
	if (SystemParametersInfo (SPI_SETFILTERKEYS, 0, (LPVOID) &keys, 0)) {
		lua_pushboolean(L, 1);
		return 1;
	}
	return 0;
}

static int l_TurnOffMonitor( lua_State *L ) {
	SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM) 2);
	return 0;
}

static int l_GetJoyPosInfo ( lua_State *L ) {
#ifndef XINPUT
	// POV, X, Y, Z, R, U
	DWORD POV = g_joyInfo.dwPOV;
	if (POV == -2) return 0; //joystick polling error occured

	if (POV  == JOY_POVCENTERED) lua_pushstring ( L, "CENTERED");
	else if (POV  == JOY_POVBACKWARD) lua_pushstring ( L, "DOWN");
	else if (POV  == JOY_POVFORWARD) lua_pushstring ( L, "UP");
	else if (POV  == JOY_POVLEFT) lua_pushstring ( L, "LEFT");
	else if (POV  == JOY_POVRIGHT) lua_pushstring ( L, "RIGHT");

	lua_pushnumber ( L, g_joyInfo.dwXpos*100/0xFFFF);
	lua_pushnumber ( L, g_joyInfo.dwYpos*100/0xFFFF);
	lua_pushnumber ( L, g_joyInfo.dwZpos*100/0xFFFF);
	lua_pushnumber ( L, g_joyInfo.dwRpos*100/0xFFFF);
	lua_pushnumber ( L, g_joyInfo.dwUpos*100/0xFFFF);

	return 6;
#else
	lua_pushnumber ( L, g_ControllerState.Gamepad.sThumbLX*100/0xFFFF);
	lua_pushnumber ( L, g_ControllerState.Gamepad.sThumbLY*100/0xFFFF);
	lua_pushnumber ( L, g_ControllerState.Gamepad.sThumbRX*100/0xFFFF);
	lua_pushnumber ( L, g_ControllerState.Gamepad.sThumbRY*100/0xFFFF);
	lua_pushnumber ( L, g_ControllerState.Gamepad.bLeftTrigger*100/0xFF);
	lua_pushnumber ( L, g_ControllerState.Gamepad.bRightTrigger*100/0xFF);
	return 6;
#endif
}

static int l_IsJoyButtonPressed ( lua_State *L ) {
#ifndef XINPUT
	const int buttonID = luaL_checkinteger(L, 1) - 1;
	lua_pushboolean(L, (g_joyInfo.dwButtons >> buttonID) &1);
	return 1;
#else
	int buttonID = -1;
	if (lua_isstring(L, 1)) {
		const char* btnName = (const char*) luaL_checkstring(L, 1);
		for (int i=0; g_GamepadButtonNames[i]; i++)
			if (!strcmp(_strupr((char*)btnName), g_GamepadButtonNames[i]))
				buttonID = i;
	} else {
		buttonID = luaL_checkinteger(L, 1) - 1;
	}
	if (buttonID > -1)
		lua_pushboolean(L, (g_ControllerState.Gamepad.wButtons >> buttonID) &1);
	else
		lua_pushnil(L);
	return 1;
#endif
}

static int l_SetGamepadVibration( lua_State *L ) {
	std::string motor = (const char*) luaL_checkstring(L, 1);
	const int percent = luaL_checkinteger(L, 2);
	unsigned int speed =  65535 * ((float)percent/100);

	XINPUT_VIBRATION vibration;
	ZeroMemory( &vibration, sizeof(XINPUT_VIBRATION) );
	if (motor == "LEFT")
		vibration.wLeftMotorSpeed = speed; // use any value between 0-65535 here
	else if (motor == "RIGHT")
		vibration.wRightMotorSpeed = speed; // use any value between 0-65535 here
	else if (motor == "BOTH") {
		vibration.wRightMotorSpeed = speed;
		vibration.wLeftMotorSpeed = speed;
	}
	XInputSetState( 0, &vibration );
	return 0;
}