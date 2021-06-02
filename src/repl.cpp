#include "repl.h"
#include "pegleg.h"

REPL::REPL() {
	InitializeSRWLock(&lock);
	historyIndex = 0;
}

int REPL::Enqueue(WCHAR *str) {
	std::wstring newwstring(str);
	AcquireSRWLockExclusive(&lock);
	replque.push(newwstring);
	ReleaseSRWLockExclusive(&lock);
	return 1;
}

int REPL::EvalTop() {
	std::wstring evalstr;
	BOOL gotWork = false;
	AcquireSRWLockExclusive(&lock);
	if (!replque.empty()) {
		evalstr = replque.front();
		replque.pop();
		gotWork = true;
	}
	ReleaseSRWLockExclusive(&lock);

	if (gotWork) {
		int strLen = evalstr.length();
		int reqBytes = WideCharToMultiByte(CP_UTF8, 0, evalstr.c_str(), strLen, NULL, 0, NULL, NULL);

		std::string str(reqBytes + 1, 0);
		WideCharToMultiByte(CP_UTF8, 0, evalstr.c_str(), strLen, &str[0], reqBytes, NULL, NULL);

		Store(evalstr);
		historyIndex = 0;

		if (luaL_loadstring(L, str.c_str()) == LUA_OK) {
			if (lua_pcall(L, 0, 0, 0) != 0)
				error(L, "error running function: %s", lua_tostring(L, -1));
		}
		else {
			error(L, "error compiling chunk: %s", lua_tostring(L, -1));
		}
	}
	return 1;
}

void REPL::Store(std::wstring command) {
	AcquireSRWLockExclusive(&lock);
	history.insert(history.begin(), command);
	if (history.size() > 10)
		history.pop_back();
	ReleaseSRWLockExclusive(&lock);
	return;
}

std::wstring REPL::HistoryNext() {
	AcquireSRWLockShared(&lock);
	if (history.empty()) return L"<None>";

	std::wstring r = history[historyIndex];
	if (historyIndex+1 < history.size()) {
		historyIndex++;
	}
	ReleaseSRWLockShared(&lock);
	return r;
}

std::wstring REPL::HistoryPrevious() {
	AcquireSRWLockShared(&lock);
	if (history.empty()) return L"<None>";

	if (historyIndex == 0) return L"";

	if (historyIndex > 0) {
		historyIndex--;
	}
	std::wstring r = history[historyIndex];
	ReleaseSRWLockShared(&lock);
	return r;
}