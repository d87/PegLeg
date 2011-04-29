#include "lua_func.h"
#include "pegleg.h"

lua_State *L = 0;

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
	lua_register(L, "MouseInput", l_MouseInput);
	lua_register(L, "KeyboardInput", l_KeyboardInput);
	lua_register(L, "GetCursorPos", l_GetCursorPos);
	lua_register(L, "IsPressed", l_IsPressed);
	lua_register(L, "ShowWindow", l_ShowWindow);
	lua_register(L, "RegisterHotKey", l_RegisterHotKey);
	lua_register(L, "GetWindowProcess", l_GetWindowProcess);
	lua_register(L, "CreateTimer", l_CreateTimer);
	lua_register(L, "KillTimer", l_KillTimer);
	lua_register(L, "Reload", l_Reload);
	
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

	if (gui->hwndConsole)
		lua_register(L, "print", luaB_print);

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
	for (int i=0; events[TIMER][i]; i++){
		KillTimer(NULL, timerMap[i]);
		timerMap[i] = 0;
	}
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

static int l_AddScript( lua_State *luaVM ) {
	const char * filename = luaL_checkstring(luaVM, 1);
	LoadScript(luaVM, filename);
	lua_pushboolean( luaVM, 1 );
	return 1;
}

static int l_ShowConsole( lua_State *luaVM ) {
	if (gui->hwnd) {
		ShowWindow(gui->hwnd,SW_SHOW);
		//SetForegroundWindow(gui->hwnd);
	}
	return 0;
}
static int l_HideConsole( lua_State *luaVM ) {
	if (gui->hwnd) ShowWindow(gui->hwnd,SW_HIDE);
	return 0;
}
static int l_ClearConsole( lua_State *luaVM ) {
	SendMessage(gui->hwndConsole,WM_SETTEXT,0,0);
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
	SendMessage(gui->hwndConsole, EM_SETBKGNDCOLOR, 0, (LPARAM)clr);
	return 0;
}

static int l_SetConsoleTextColor( lua_State *L ) {
	double r = luaL_checknumber(L, 1);
	double g = luaL_checknumber(L, 2);
	double b = luaL_checknumber(L, 3);
	if (r>1) r = 1; if (r<0) r = 0;
	if (g>1) g = 1; if (g<0) g = 0;
	if (b>1) b = 1; if (b<0) b = 0;
	gui->text_r = (int)r*255;
	gui->text_g = (int)g*255;
	gui->text_b = (int)b*255;
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
int l_ShowWindow( lua_State *L ) {
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

int l_GetWindowProcess( lua_State *L ) {
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

int l_CreateTimer( lua_State *L ) {
	unsigned int interval = (unsigned int)luaL_checknumber(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	int func_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	int i=0;
	for (; events[TIMER][i]; i++ );
	if ( i < MAX_EVENTS ) events[TIMER][i] = func_ref;
	else return 0;

	int tID = SetTimer(NULL,i,interval,NULL);
	timerMap[i] = tID;
	lua_pushnumber(L, tID);
	return 1;
}

int l_KillTimer( lua_State *L ) {
	int tID = (int)luaL_checknumber(L, 1);
	int i = 0;
	for (; timerMap[i]; i++)
		if (timerMap[i] == tID) {
			KillTimer(NULL, tID);
			luaL_unref(L, LUA_REGISTRYINDEX,events[TIMER][i]);
			events[TIMER][i] = 0;
			timerMap[i] = 0;
			lua_pushboolean(L, 1);
			return 1;
		}
	lua_pushnil(L);
	return 1;
}