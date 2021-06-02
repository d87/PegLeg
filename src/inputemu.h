#define _ALLOW_RTCc_IN_STL

#include <windows.h>

#include <vector>
#include <string>
#include <unordered_map>

extern std::vector<std::string> VKEYS;
extern std::unordered_map<std::string, int> VKCODES;
extern void MakeReverseLookupMap();
extern int GetVKCodeByName(const char *name);
int KeyboardSend(char *name, int mode);
int KeyboardInput(char *str);
int MouseInput(char *eventname, int x, int y, int isabsolute);