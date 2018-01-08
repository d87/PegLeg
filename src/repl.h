#pragma once

#define _ALLOW_RTCc_IN_STL

#include <string>
#include <queue>
#include <windows.h>

using namespace std;

class REPL {
private:
	std::queue<wstring> replque;
	SRWLOCK lock;
	UINT historyIndex;
	std::vector<wstring> history;
public:
	REPL();
	int Enqueue(WCHAR *str);
	void Store(wstring command);
	std::wstring HistoryPrevious();
	std::wstring HistoryNext();
	int EvalTop();
};