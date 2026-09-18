#include <iostream>
#include "eventSource.h"
#ifdef _WIN32
#include "eventSourceWin.h"
#else
#include "eventSourcePosix.h"
#include <signal.h>
#endif
#include "eventQueue.h"
#include "event.h"
#include <thread>

#include <chrono>
#include <iomanip>
#include <atomic>
#ifdef _WIN32
#include <windows.h>
#endif
#include "apiClient.h"
#include <curl/curl.h>

using namespace std;

static atomic<bool> g_running = true;

#ifdef _WIN32
BOOL WINAPI consoleHandler(DWORD signal) {
	if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT || signal == CTRL_CLOSE_EVENT) {
		g_running = false;
		return TRUE;
	}
	return FALSE;
}
#else
static void signalHandler(int /*signum*/) {
	g_running = false;
}
#endif

void eventsThreadFunction(EventSource* eventSource, EventQueue* eventQueue) {
	while (g_running) {
		Event event = eventSource->getEvent();
		eventQueue->push(event);
		for (int i = 0; i < 20 && g_running; ++i) {
			this_thread::sleep_for(chrono::milliseconds(100));
		}
	}
}

void outputThreadFunction(EventQueue* eventQueue) {
	ApiClient apiClient("http://127.0.0.1:8000/data");
	while (g_running) {
		apiClient.processQueue(*eventQueue);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	// Final flush on exit: попытка отправки, если не получается - сохранить буфер в файл
	apiClient.processQueue(*eventQueue);
	bool ok = apiClient.sendNow();
	if (!ok) {
		apiClient.backupToFile("backup.json");
	}
}

int main()
{
	if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
		std::cerr << "curl_global_init failed\n";
		return 1;
	}
#ifdef _WIN32
	SetConsoleCtrlHandler(consoleHandler, TRUE);
#else
	signal(SIGINT, signalHandler);
#endif

	std::cout << "intec_time_tracker started. Press Ctrl+C to quit.\n" << std::flush;
#ifdef _WIN32
	WinEventSource eventSource;
#else
	PosixEventSource eventSource;
#endif
	EventQueue eventQueue;
	
	std::thread eventThread(eventsThreadFunction, &eventSource, &eventQueue);
	std::thread outputThread(outputThreadFunction, &eventQueue);

	eventThread.join();
	outputThread.join();
	std::cout << "intec_time_tracker stopped.\n";
	curl_global_cleanup();
	return 0;
}
