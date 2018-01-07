#pragma once

#define _ALLOW_RTCc_IN_STL

#include <string>
#include <queue>
#include <windows.h>

using namespace std;

class REPL {
private:
	queue<wstring> replque;
	SRWLOCK lock;

public:
	REPL();
	int Enqueue(WCHAR *str);
	int EvalTop();
};