#include <Windows.h>

COLORREF color = 0;
HDC R;
HFONT font = 0;
unsigned int bgMode;

static int SetupOSDText (double r, double g, double b, char *fontname, int size, unsigned int newBGMode );

static int SetupOSDText (double r, double g, double b, char *fontname, int size, unsigned int newBGMode ) {
	/*const char* text = luaL_checkstring(L, 1);
	const int _x = luaL_checkinteger(L, 2);
	const int _y = luaL_checkinteger(L, 3);
	double r = ( lua_isnumber(L, 4) ? lua_tonumber(L, 4) : 1 );
	double g = ( lua_isnumber(L, 5) ? lua_tonumber(L, 5) : 1 );
	double b = ( lua_isnumber(L, 6) ? lua_tonumber(L, 6) : 1 );
	const char* font = ( lua_isstring(L, 7) ? lua_tostring(L, 7) : "Trebuchet MS" );
	int size = ( lua_isnumber(L, 8) ? lua_tointeger(L, 8) : 25 );
	*/
	COLORREF color;
	HDC R;

	if (r>1) r = 1; if (r<0) r = 0;
	if (g>1) g = 1; if (g<0) g = 0;
	if (b>1) b = 1; if (b<0) b = 0;
	color = RGB(r*255,g*255,b*255);

	font = CreateFont(size,0,0,0,0,0,0,0,1,0,0,0,0,fontname);

	HWND Desktop = GetDesktopWindow();
	R = GetWindowDC(Desktop);
	bgMode = newBGMode;
	if (newBGMode == OPAQUE) {
		SetBkMode(R, OPAQUE);
		SetBkColor(R, RGB(0,0,0));
	} else {
		SetBkMode(R, TRANSPARENT);
	}
	SetTextColor(R, color);
	SelectObject(R, font);
	ReleaseDC(Desktop,R);
	return 1;
}

static int PrintOnScreen (char * text, int x, int y ) {
	
}