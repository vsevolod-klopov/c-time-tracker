#include "eventSourceWin.h"
#include "event.h"
#include <windows.h>
#include <iostream>
#include <psapi.h>

#pragma comment(lib, "psapi.lib")

static std::string to_utf8(const std::wstring &w) {
	if (w.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), NULL, 0, NULL, NULL);
	if (size_needed <= 0) return std::string();
	std::string s(size_needed, '\0');
	WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &s[0], size_needed, NULL, NULL);
	return s;
}

static std::string utf8_to_ansi(const std::string &utf8) {
	if (utf8.empty()) return std::string();
	int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
	if (wlen <= 0) return std::string();
	std::wstring wstr(wlen, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wstr[0], wlen);
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (len <= 0) return std::string();
	std::string out(len, '\0');
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &out[0], len, nullptr, nullptr);
	if (!out.empty() && out.back() == '\0') out.pop_back();
	return out;
}

Event WinEventSource::getEvent() {
	bool active = false;
	std::string window = "Unknown";
	std::string process = "Unknown";

	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	HWND hwnd = GetForegroundWindow();
	if (hwnd != NULL) {
		wchar_t windowTitleW[256] = { 0 };
		int resW = GetWindowTextW(hwnd, windowTitleW, static_cast<int>(std::size(windowTitleW)));
		if (resW > 0) {
			window = to_utf8(std::wstring(windowTitleW, resW));
		}

		DWORD processId = 0;
		GetWindowThreadProcessId(hwnd, &processId);

		if (processId != 0) {
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
			if (hProcess != NULL) {
				wchar_t processNameW[MAX_PATH] = { 0 };
				if (GetModuleBaseNameW(hProcess, NULL, processNameW, static_cast<DWORD>(std::size(processNameW)))) {
					process = to_utf8(std::wstring(processNameW));
				}
				CloseHandle(hProcess);
			}
		}
	}

	LASTINPUTINFO lii = {0};
	lii.cbSize = sizeof(LASTINPUTINFO);

	if (GetLastInputInfo(&lii)) {
		DWORD idleTime = GetTickCount() - lii.dwTime;
		if ((idleTime / 1000) > 5) { // Если пользователь неактивен более 5 секунд
			active = false;
		}
		else {
			active = true;
		}
	}
	wchar_t agent_nameW[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size = static_cast<DWORD>(std::size(agent_nameW));

	std::string agentUtf8 = "";
	if (GetComputerNameW(agent_nameW, &size)) {
		agentUtf8 = to_utf8(std::wstring(agent_nameW));
		std::string agentLocal = utf8_to_ansi(agentUtf8);
	}

	return Event(window, process, active, agentUtf8);
}
