#include <windows.h>

extern char VKEYS[255][15];

int KeyboardSend(char *name, int mode);
int KeyboardInput(char *str);
int MouseInput(char *eventname, int x, int y, int isabsolute);
int PhotoshopShowHUDCP();
int PhotoshopHideHUDCP();