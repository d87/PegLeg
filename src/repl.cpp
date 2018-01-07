#include "repl.h"
#include "pegleg.h"

REPL::REPL() {
	InitializeSRWLock(&lock);
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

		//luaL_dostring(L, str);
		luaL_loadstring(L, str.c_str());
		if (lua_pcall(L, 0, 0, 0) != 0)
			error(L, "error running function: %s", lua_tostring(L, -1));
	}
	return 1;
}