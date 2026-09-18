#include "eventSourcePosix.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/scrnsaver.h>
#include <unistd.h>
#include <fstream>
#include <iostream>

static std::string read_proc_comm(unsigned long pid) {
	std::string proc = "Unknown";
	if (pid == 0) return proc;
	std::string path = "/proc/" + std::to_string(pid) + "/comm";
	std::ifstream ifs(path);
	if (ifs) {
		std::getline(ifs, proc);
	}
	return proc;
}

Event PosixEventSource::getEvent() {
	bool active = false;
	std::string window = "Unknown";
	std::string process = "Unknown";
	std::string agentUtf8 = "";

	Display* disp = XOpenDisplay(NULL);
	if (disp) {
		Window root = DefaultRootWindow(disp);

		Atom a_active = XInternAtom(disp, "_NET_ACTIVE_WINDOW", True);
		if (a_active != None) {
			Atom actual_type;
			int actual_format;
			unsigned long nitems, bytes_after;
			unsigned char *prop = nullptr;
			if (XGetWindowProperty(disp, root, a_active, 0, (~0L), False, AnyPropertyType,
				&actual_type, &actual_format, &nitems, &bytes_after, &prop) == Success && prop) {
				Window active_win = *(Window*)prop;
				XFree(prop);

				Atom utf8 = XInternAtom(disp, "UTF8_STRING", True);
				Atom a_name = XInternAtom(disp, "_NET_WM_NAME", True);
				if (a_name != None) {
					unsigned char *nameProp = nullptr;
					if (XGetWindowProperty(disp, active_win, a_name, 0, (~0L), False, utf8 ? utf8 : AnyPropertyType,
						&actual_type, &actual_format, &nitems, &bytes_after, &nameProp) == Success && nameProp) {
						window = std::string(reinterpret_cast<char*>(nameProp));
						XFree(nameProp);
					}
				}

				Atom a_pid = XInternAtom(disp, "_NET_WM_PID", True);
				if (a_pid != None) {
					unsigned char *pidProp = nullptr;
					if (XGetWindowProperty(disp, active_win, a_pid, 0, (~0L), False, XA_CARDINAL,
						&actual_type, &actual_format, &nitems, &bytes_after, &pidProp) == Success && pidProp) {
						unsigned long pid = 0;
						if (nitems > 0) pid = *((unsigned long*)pidProp);
						XFree(pidProp);
						if (pid != 0) {
							process = read_proc_comm(pid);
						}
					}
				}
			}
		}

		int idle_ms = 0;
		int event_base, error_base;
		if (XScreenSaverQueryExtension(disp, &event_base, &error_base)) {
			XScreenSaverInfo *info = XScreenSaverAllocInfo();
			if (info) {
				XScreenSaverQueryInfo(disp, DefaultRootWindow(disp), info);
				idle_ms = info->idle;
				XFree(info);
			}
		}

		XCloseDisplay(disp);

		if ((idle_ms / 1000) > 5) active = false; else active = true;
	}

	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) == 0) {
		agentUtf8 = std::string(hostname);
	}

	return Event(window, process, active, agentUtf8);
}
