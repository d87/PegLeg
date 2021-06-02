#pragma once

#define _ALLOW_RTCc_IN_STL

#include <string>
#include <queue>
#include <windows.h>

class REPL {
private:
	std::queue<std::wstring> replque;
	SRWLOCK lock;
	UINT historyIndex;
	std::vector<std::wstring> history;
public:
	REPL();
	int Enqueue(WCHAR *str);
	void Store(std::wstring command);
	std::wstring HistoryPrevious();
	std::wstring HistoryNext();
	int EvalTop();
};