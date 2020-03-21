#include "inputemu.h"



using namespace std;

// char VKEYS[255][15];
vector<string> VKEYS (255);
unordered_map<string, int> VKCODES;

void MakeReverseLookupMap() {
	for (int i = 0; i < VKEYS.size(); i++) {
		string VK = VKEYS[i];
		if (!VK.empty()) {
			VKCODES.insert(make_pair(VK, i));
		}
	}
}

int GetVKCodeByName(const char *name) {
	auto search = VKCODES.find(name);
	if (search != VKCODES.end()) {
		const int vkCode = search->second;
		return vkCode;
	}
	return NULL;
}

int KeyboardSend(char *name, int mode) {
	
	auto search = VKCODES.find(name);
	if (search != VKCODES.end()) {
		const int vkCode = search->second;

		INPUT  key = { 0 };
		if (mode != 1) {
			key.type = INPUT_KEYBOARD;
			key.ki.wVk = vkCode;
			SendInput(1, &key, sizeof(INPUT));
		}
		if (mode != -1) {
			ZeroMemory(&key, sizeof(INPUT));
			key.type = INPUT_KEYBOARD;
			key.ki.wVk = vkCode;
			key.ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(1, &key, sizeof(INPUT));
		}
	}
	return 0;
}

int KeyboardInput(char *str) {
//char *str = luaL_checkstring(L, 1);
	int i = 0;
	char buff[15];
	int bi = 0;
	int kbevent = 0;
	while (str[i]) {
		if (str[i] == '>') { kbevent = -1; i++; continue; }
		if (str[i] == '<') { kbevent = 1; i++; continue; }

		if (str[i] == '(') {
			i++;
			while (str[i] && str[i]!=')') {
				buff[bi] = str[i];
				i++;
				bi++;
			}
			buff[bi] = 0;
			bi = 0;
		} else {
			buff[0]=str[i];
			buff[1]=0;
		}
		// send
		KeyboardSend(buff, kbevent);
		kbevent = 0;
		i++;
	}
	return 1;
}

int MouseInput(char *eventname, int x, int y, int isabsolute) {
	//const char * eventname = luaL_checkstring(L, 1);

	int eventflag = 0;
	int mouseData = 0;
	if		(!strcmp(_strupr(eventname),"LEFTDOWN")) eventflag = MOUSEEVENTF_LEFTDOWN;
	else if (!strcmp(_strupr(eventname),"LEFTUP")) eventflag = MOUSEEVENTF_LEFTUP;
	else if (!strcmp(_strupr(eventname),"LEFT")) eventflag = MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP;
	else if (!strcmp(_strupr(eventname),"RIGHTDOWN")) eventflag = MOUSEEVENTF_RIGHTDOWN;
	else if (!strcmp(_strupr(eventname),"RIGHTUP")) eventflag = MOUSEEVENTF_RIGHTUP;
	else if (!strcmp(_strupr(eventname),"MOVE")) eventflag = MOUSEEVENTF_MOVE;
	else if (!strcmp(_strupr(eventname),"MIDDLEDOWN")) eventflag = MOUSEEVENTF_MIDDLEDOWN;
	else if (!strcmp(_strupr(eventname),"MIDDLEUP")) eventflag = MOUSEEVENTF_MIDDLEUP;
	else if (!strcmp(_strupr(eventname),"4DOWN")){ eventflag = MOUSEEVENTF_XDOWN; mouseData = XBUTTON1; }
	else if (!strcmp(_strupr(eventname),"4UP")){ eventflag = MOUSEEVENTF_XUP; mouseData = XBUTTON1; }
	else if (!strcmp(_strupr(eventname),"5DOWN")){ eventflag = MOUSEEVENTF_XDOWN; mouseData = XBUTTON2; }
	else if (!strcmp(_strupr(eventname),"5UP")){ eventflag = MOUSEEVENTF_XUP; mouseData = XBUTTON2; }

	int absolute = 0;
	int fx = 0;
	int fy = 0;
	if (eventflag == MOUSEEVENTF_MOVE){
		if (isabsolute)
				absolute = MOUSEEVENTF_ABSOLUTE;

		//int x = luaL_checkinteger( L, 2);
		//int y = luaL_checkinteger( L, 3);
		if (absolute ){
			double fScreenWidth    = GetSystemMetrics( SM_CXSCREEN )-1; 
			double fScreenHeight  = GetSystemMetrics( SM_CYSCREEN )-1; 
			fx = (int)x*(65535.0f/fScreenWidth);
			fy = (int)y*(65535.0f/fScreenHeight);
		} else {
			fx = x;
			fy = y;
		}
	}

	INPUT  mouse={0};
	mouse.type=INPUT_MOUSE;
	if (mouseData) mouse.mi.mouseData = mouseData;
	mouse.mi.dx=fx;
	mouse.mi.dy=fy;
	mouse.mi.dwFlags=absolute|eventflag;
	SendInput(1,&mouse,sizeof(INPUT));
	return 1;
}
