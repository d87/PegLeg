#include "lua_func.h"
#include "pegleg.h"

#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>


using namespace std;

struct PLTIMER {
	int interval;
	int remains;
	unsigned int luaFuncRef;
};

unordered_map<std::string, PLTIMER> Timers;
vector<HWND> wndList;

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
	lua_register(L, "IsXInputEnabled", l_IsXInputEnabled);
	lua_register(L, "GetMasterVolume", l_GetMasterVolume);
	lua_register(L, "SetMasterVolume", l_SetMasterVolume);
	lua_register(L, "GetMasterVolumeMute", l_GetMasterVolumeMute);
	lua_register(L, "SetMasterVolumeMute", l_SetMasterVolumeMute);
	lua_register(L, "ToggleWindowTitle", l_ToggleWindowTitle);
	lua_register(L, "GetClipboardText", l_GetClipboardText);
	lua_register(L, "ListWindows", l_ListWindows);
	lua_register(L, "GetWindowPos", l_GetWindowPos);
	lua_register(L, "SetWindowPos", l_SetWindowPos);
	lua_register(L, "ShowWindow", l_ShowWindow);
	lua_register(L, "GetMouseSpeed", l_GetMouseSpeed);
	lua_register(L, "SetMouseSpeed", l_SetMouseSpeed);

	lua_register(L, "GetSelectedGamepad", l_GetSelectedGamepad);
	lua_register(L, "SelectGamepad", l_SelectGamepad);

	lua_register(L, "PlaySound", l_PlaySound);
	lua_register(L, "SwitchToDesktop", l_SwitchToDesktop);
	lua_register(L, "MoveWindowToDesktop", l_MoveWindowToDesktop);
	lua_register(L, "TogglePinCurrentWindow", l_TogglePinCurrentWindow);
	lua_register(L, "IsWindowMaximized", l_IsWindowMaximized);
	lua_register(L, "GetDesktopCount", l_GetDesktopCount);
	lua_register(L, "GetCurrentDesktopNumber", l_GetCurrentDesktopNumber);
	lua_register(L, "MakeBorderless", l_MakeBorderless);
	
	

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

	FireEvent(L, PegLegEvent::ONCREATE,0, 0, 0);

	return 1;
}

int l_Reload(lua_State *L){
	PostThreadMessage(mainThreadId,WM_COMMAND, RELOADLUA, NULL);
	return 0;
}

int ReloadLua() {
	guiAddText("!Reloading...\n");

	//killing timers and hotkeys
	auto hotkeys = events[PegLegEvent::HOTKEY];
	for (int i=0; i < hotkeys.size(); i++)
		UnregisterHotKey(NULL, i);
	Timers.clear();

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
	const char * eventName = luaL_checkstring(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	unsigned int func_ref = luaL_ref(L, LUA_REGISTRYINDEX);


	string upperEventName = string(eventName);
	for (auto & c : upperEventName) c = toupper(c);

	auto search = EventStringToID.find(upperEventName);
	if (search != EventStringToID.end()) {
		auto EventID = search->second;

		events[EventID].push_back(func_ref);

		lua_pushboolean(L, 1);
		return 1;
	}
	

	
	return 0;
}
static int l_UnregisterEvent(lua_State *L) {
	//if (!lua_isnumber(L, 1))
	//		error(L, "Invalid arg#1 (expecting number)");
	/*
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
	*/
	return 0;
}

static int l_shell ( lua_State *luaVM ) {
	TCHAR cmdl[1000];
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
	HWND hwnd = GetForegroundWindow();
	UINT nLen = GetWindowTextLength(hwnd);

	std::string str(nLen+1, 0);
	GetWindowText(hwnd, &str[0], nLen+1);
	lua_pushstring(luaVM, str.c_str());
	lua_pushnumber(luaVM, (UINT32)hwnd);

	return 2;
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
	gui.text_r = r;
	gui.text_g = g;
	gui.text_b = b;
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
/*
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
}*/


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

	events[PegLegEvent::HOTKEY].push_back(func_ref);
	int i = events[PegLegEvent::HOTKEY].size()-1;

	RegisterHotKey(NULL, i, mods, vk);
	return 0;
}

static int l_GetWindowProcess( lua_State *L ) {
	unsigned long pid;
	HWND hwnd = GetForegroundWindow();
	GetWindowThreadProcessId(hwnd, &pid);

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
				lua_pushnumber(L, (UINT32)hwnd);
				free(pname);
				return 2;
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
			RunCallback(L, it->second.luaFuncRef);
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
	std::string text = (const char*)  luaL_checkstring(L, 1);
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
	R = GetWindowDC(Desktop);
	SetBkMode(R,OPAQUE);//TRANSPARENT);
	SetBkColor(R,RGB(0,0,0));
	SetTextColor(R,color);
	SelectObject(R, CreateFont(size,0,0,0,0,0,0,0,1,0,0,0,0,font) );
	TextOutA(R,_x,_y, text.c_str(), text.length() );
	ReleaseDC(Desktop,R);
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
	if (g_gamepadGroup->activeGamepad != NULL) {
		lua_pushnumber(L, g_gamepadGroup->activeGamepad->state.Gamepad.sThumbLX * 100 / 0xFFFF);
		lua_pushnumber(L, g_gamepadGroup->activeGamepad->state.Gamepad.sThumbLY * 100 / 0xFFFF);
		lua_pushnumber(L, g_gamepadGroup->activeGamepad->state.Gamepad.sThumbRX * 100 / 0xFFFF);
		lua_pushnumber(L, g_gamepadGroup->activeGamepad->state.Gamepad.sThumbRY * 100 / 0xFFFF);
		lua_pushnumber(L, g_gamepadGroup->activeGamepad->state.Gamepad.bLeftTrigger * 100 / 0xFF);
		lua_pushnumber(L, g_gamepadGroup->activeGamepad->state.Gamepad.bRightTrigger * 100 / 0xFF);
		lua_pushnumber(L, g_gamepadGroup->activeGamepad->state.dwPacketNumber);
		return 7;
	}
	return 0;
}

static int l_IsJoyButtonPressed ( lua_State *L ) {
	const char* btnName = (const char*) luaL_checkstring(L, 1);
		
	if (g_gamepadGroup->activeGamepad)
		lua_pushboolean(L, g_gamepadGroup->activeGamepad->IsPressed((char*)btnName));
	else
		lua_pushnil(L);
	return 1;
}

static int l_SetGamepadVibration( lua_State *L ) {
#ifdef XINPUT
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
	XInputSetState( g_gamepadGroup->activeGamepadID, &vibration );

#endif
	return 0;
}

static int l_IsXInputEnabled( lua_State *L ) {
#ifdef XINPUT
	lua_pushboolean( L, 1);
#else
	lua_pushboolean( L, 0);
#endif
	return 1;
}



float SetMasterVolumeLevelScalar(float fMasterVolumeAdd)
{
    IMMDeviceEnumerator *deviceEnumerator = NULL;
    IMMDevice *defaultDevice = NULL;
    IAudioEndpointVolume *endpointVolume = NULL;
    HRESULT hr;
    float fMasterVolume;
    BOOL bSuccess = FALSE;
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioEndpointVolume = __uuidof(IAudioEndpointVolume);

	hr = CoInitialize(NULL);

    hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void **)&deviceEnumerator);
    if(SUCCEEDED(hr))
    {
        hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
        if(SUCCEEDED(hr))
        {
            hr = defaultDevice->Activate(IID_IAudioEndpointVolume, CLSCTX_ALL, NULL, (void **)&endpointVolume);
            if(SUCCEEDED(hr))
            {
                if(SUCCEEDED(endpointVolume->GetMasterVolumeLevelScalar(&fMasterVolume)))
                {
					if (fMasterVolumeAdd != NULL) {
						fMasterVolume = fMasterVolumeAdd;

						if(fMasterVolume < 0.0)
							fMasterVolume = 0.0;
						else if(fMasterVolume > 1.0)
							fMasterVolume = 1.0;

						if(SUCCEEDED(endpointVolume->SetMasterVolumeLevelScalar(fMasterVolume, NULL)))
							bSuccess = TRUE;
					}
                }
                endpointVolume->Release();
            }
            defaultDevice->Release();
        }
        deviceEnumerator->Release();
    }

	CoUninitialize();

	return fMasterVolume;
}

BOOL SetMasterVolumeMute(BOOL bNewMute)
{
	IMMDeviceEnumerator *deviceEnumerator = NULL;
	IMMDevice *defaultDevice = NULL;
	IAudioEndpointVolume *endpointVolume = NULL;
	HRESULT hr;
	BOOL bMute = 0;
	BOOL bSuccess = FALSE;
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioEndpointVolume = __uuidof(IAudioEndpointVolume);

	hr = CoInitialize(NULL);

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void **)&deviceEnumerator);
	if (SUCCEEDED(hr))
	{
		hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
		if (SUCCEEDED(hr))
		{
			hr = defaultDevice->Activate(IID_IAudioEndpointVolume, CLSCTX_ALL, NULL, (void **)&endpointVolume);
			if (SUCCEEDED(hr))
			{
				if (SUCCEEDED(endpointVolume->GetMute(&bMute)))
				{
					if (bNewMute != -1) {
						bMute = bNewMute;

						if (SUCCEEDED(endpointVolume->SetMute(bMute, NULL)))
							bSuccess = TRUE;
					}
				}
				endpointVolume->Release();
			}
			defaultDevice->Release();
		}
		deviceEnumerator->Release();
	}

	CoUninitialize();

	return bMute;
}

static int l_GetMasterVolumeMute(lua_State *L) {
	lua_pushboolean(L, SetMasterVolumeMute(-1));
	return 1;
}
static int l_SetMasterVolumeMute(lua_State *L) {
	BOOL newMute = lua_toboolean(L, 1);
	lua_pushboolean(L, SetMasterVolumeMute(newMute));
	return 1;
}

static int l_GetMasterVolume(lua_State *L) {
	lua_pushnumber(L, SetMasterVolumeLevelScalar(NULL));
	return 1;
}

static int l_SetMasterVolume( lua_State *L ) {
	float newVolume = luaL_checknumber(L, 1);
	//bool bAbsolute = false;
	//if (lua_isboolean(L,2))
	//	if (lua_toboolean(L,2) == 1)
	//		bAbsolute = true;


	lua_pushnumber(L, SetMasterVolumeLevelScalar(newVolume));
	return 1;
}


static int l_ToggleWindowTitle(lua_State *L) {
	HWND hwnd = GetForegroundWindow();
	SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) ^ WS_CAPTION);
	return 0;
}

static int l_GetClipboardText(lua_State *L) {
	if (OpenClipboard(NULL)){
		//printf("Opened Clipboard\n");
		HGLOBAL hglb = GetClipboardData(CF_UNICODETEXT);
		LPWSTR wstr = 0;
		wstr = (LPWSTR)GlobalLock(hglb);
		GlobalUnlock(hglb);
		CloseClipboard();
		if (wstr) {
			int strLen = lstrlenW(wstr);
			std::string str(strLen + 1, 0);			
			WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], strLen, NULL, NULL);
			lua_pushstring(L, str.c_str());
			return 1;
		}
	}
	return 0;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam){
	vector<HWND> *winList = (vector<HWND>*)lParam;
	winList->push_back(hwnd);
	return true;
}

static int l_ListWindows(lua_State *L){
	vector<HWND> winList;
	EnumWindows(EnumWindowsProc, (LPARAM)&winList);
	char *title = (char *)malloc(sizeof(char) * 200);

	lua_newtable(L);

	WINDOWINFO winfo;
	for (std::vector<HWND>::size_type i = 0; i != winList.size(); i++) {
		/* std::cout << someVector[i]; ... */
		HWND hwnd = winList[i];
		GetWindowText(hwnd, title, 200);

		/*
		ZeroMemory(&winfo, sizeof(WINDOWINFO));
		winfo.cbSize = sizeof(WINDOWINFO);
		GetWindowInfo(hwnd, &winfo);
		printf("title: %s    w:%i h:%i\n", title, winfo.rcWindow.bottom - winfo.rcWindow.top, winfo.rcWindow.right - winfo.rcWindow.left);
		*/
		//printf("")
		if ((!title[0]) ||
			(!strcmp(title, "Default IME")) ||
			(!strcmp(title, "MSCTFIME UI")) ||
			(!strcmp(title, "GDI+ Window")) ||
			(!strcmp(title, "Msg")))
			continue;

		lua_pushnumber(L, (UINT32)hwnd);
		lua_pushstring(L, title);
		lua_settable(L, -3);
	}

	free(title);
	return 1;
}

static int l_GetWindowPos(lua_State *L){
	UINT32 hwndInt = luaL_checknumber(L, 1);
	HWND hwnd = (HWND)hwndInt;
	if (!IsWindow(hwnd)) return 0;
	RECT rc;
	GetWindowRect(hwnd, &rc);
	lua_pushnumber(L, rc.left);
	lua_pushnumber(L, rc.top);
	lua_pushnumber(L, rc.right-rc.left);
	lua_pushnumber(L, rc.bottom - rc.top);
	return 4;
}

static int l_SetWindowPos(lua_State *L){
	UINT32 hwndInt = luaL_checknumber(L, 1);
	HWND hwnd = (HWND)hwndInt;
	int x = luaL_checknumber(L, 2);
	int y = luaL_checknumber(L, 3);
	int width = luaL_checknumber(L, 4);
	int height = luaL_checknumber(L, 5);

	if (!IsWindow(hwnd)) return 0;

	SetWindowPos(hwnd, NULL, x, y, width, height, SWP_NOACTIVATE|SWP_NOZORDER);
	lua_pushboolean(L, 1);
	return 1;
}

//static int l_GetActiveWindowHandle(lua_State *L) {
//	HWND hwnd = GetForegroundWindow();
//	lua_pushnumber(L, (UINT32)hwnd);
//	return 1;
//}

static int l_IsWindowMaximized(lua_State *L) {
	UINT32 hwndNum = (UINT32)luaL_optnumber(L, 1, NULL);
	HWND hwnd = (HWND)hwndNum;
	if (!hwnd) hwnd = GetForegroundWindow();


	WINDOWPLACEMENT wpl;
	GetWindowPlacement(hwnd, &wpl);


	
	LONG style = GetWindowLong(hwnd, GWL_STYLE);

	if ((style & WS_MAXIMIZE) == WS_MAXIMIZE) {
		lua_pushboolean(L, 1);
	}
	else {
		lua_pushboolean(L, 0);
	}
	return 1;
}

static int l_ShowWindow(lua_State *L){
	UINT32 hwndNum = (UINT32)luaL_checknumber(L, 1);
	HWND hwnd = (HWND)hwndNum;
	
	const char * fs = luaL_checkstring(L, 2);
	unsigned int mods = 0;
	char *ptr = (char *)fs;
	while (*ptr) {
		if (!strncmp(ptr, "SW_MAXIMIZE", 11)) { mods |= SW_MAXIMIZE; ptr += 11; }
		if (!strncmp(ptr, "SW_MINIMIZE", 11)) { mods |= SW_MINIMIZE;  ptr += 11; }
		if (!strncmp(ptr, "SW_RESTORE", 10)) { mods |= SW_RESTORE;  ptr += 10; }
		ptr++;
	}
	ShowWindow(hwnd, mods);
	return 0;
}

static int l_GetMouseSpeed(lua_State *L){
	int MouseSpeed = 0;
	SystemParametersInfo(SPI_GETMOUSESPEED, 0, &MouseSpeed, 0);
	lua_pushnumber(L, MouseSpeed);
	return 1;
}

static int l_SetMouseSpeed(lua_State *L){
	const int newMouseSpeed = luaL_checknumber(L, 1);
	if (newMouseSpeed < 1 || newMouseSpeed > 20) {
		error(L, "Mouse Speed should range between 1 and 20: passed %s\r\n", lua_tostring(L, -1));
		return 0;
	}
	SystemParametersInfo(SPI_SETMOUSESPEED, 0, (PVOID)newMouseSpeed, SPIF_UPDATEINIFILE||SPIF_SENDCHANGE);
	lua_pushboolean(L, 1);
	return 1;
}

static int l_GetSelectedGamepad(lua_State *L) {
	lua_pushnumber(L, g_gamepadGroup->activeGamepadID+1);
	return 1;
}

static int l_SelectGamepad(lua_State *L) {
	/*const DWORD newGamepadID = luaL_checknumber(L, 1);
	if (newGamepadID > 0 && newGamepadID <= 5) {
		ControllerID = newGamepadID - 1;
		lua_pushnumber(L, ControllerID + 1);
		return 1;
	}*/
	return 0;
}


static int l_PlaySound(lua_State *L) {
	std::string fn = (const char*) luaL_checkstring(L, 1);

	int reqChars = MultiByteToWideChar(CP_UTF8, 0, fn.c_str(), -1, 0, 0);

	std::wstring wfn(reqChars, 0);
	MultiByteToWideChar(CP_UTF8, 0, fn.c_str(), -1, &wfn[0], reqChars);


	soundplayer->Play(wfn.c_str());
	lua_pushboolean(L, 1);
	return 1;
}


static int l_SwitchToDesktop(lua_State *L) {
	UINT desktopID = luaL_checknumber(L, 1);
	if (desktopID == 0) desktopID = 1;

	pVirtualDesktopControl->SwitchToDesktop(desktopID - 1);

	return 0;
}

static int l_MoveWindowToDesktop(lua_State *L) {
	UINT desktopID = luaL_checknumber(L, 1);
	if (desktopID == 0) desktopID = 1;
	HWND topWindow = GetForegroundWindow();

	if (topWindow != NULL) {
		pVirtualDesktopControl->MoveWindowToDesktop(topWindow, desktopID - 1);
		lua_pushboolean(L, 1);
		return 1;
	}
	return 0;
}

static int l_TogglePinCurrentWindow(lua_State *L) {
	HWND topWindow = GetForegroundWindow();
	pVirtualDesktopControl->TogglePinWindow(topWindow);
	return 0;
}


static int l_GetDesktopCount(lua_State *L) {
	lua_pushnumber(L, pVirtualDesktopControl->GetDesktopCount());
	return 1;
}

static int l_GetCurrentDesktopNumber(lua_State *L) {
	int cur = pVirtualDesktopControl->GetCurrentDesktopNumber();
	if (cur == -1) {
		return 0;
	}
	lua_pushnumber(L, cur+1);
	return 1;
}

static int l_MakeBorderless(lua_State *L) {
	HWND topWindow = GetForegroundWindow();
	SetWindowLongPtr(topWindow, GWL_STYLE, WS_VISIBLE);
	SetWindowLongPtr(topWindow, GWL_EXSTYLE, NULL);
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight= GetSystemMetrics(SM_CYSCREEN);
	SetWindowPos(topWindow, NULL, 0, 0, screenWidth, screenHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	lua_pushnumber(L, 1);
	return 1;
}