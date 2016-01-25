#pragma once

#include "lua_func.h"

struct gui_struct {
	HWND hwnd;
	HWND hwndConsole;
	HMENU hmenuTray;
	HINSTANCE hInstance;
	int createtray;
	int width;
	int height;
	float text_r, text_g, text_b;
	HANDLE event_ready;
};
extern struct gui_struct gui;
//extern gui_struct *gui;
void KillTrayIcon(HWND hwnd);
void guiThread( void *param  );
void guiAddText(const char *str);
int gui_printf( const char *format, va_list arglist );
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);