#include <iostream>
#include <Windows.h>
#include <WinUser.h>
#include <string>
#include <shobjidl_core.h>

#include "volume.h"

#if !defined(DLLExport)
#define DLLExport __declspec(dllexport)
#endif

HHOOK hKeyboardHook;
bool consoleInputActive = false;

double lastVolume = 0.0;
POINT lastCursorPos;

const HWND consoleHWND = FindWindowA("ConsoleWindowClass", nullptr);

void showConsoleWindow() {
	consoleInputActive = true;

	ShowWindow(consoleHWND, true);
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);

	SetWindowPos(consoleHWND, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetFocus(consoleHWND);
}

void lock() {
	showConsoleWindow();

	GetCursorPos(&lastCursorPos);

	lastVolume = getSystemVolume();
	setSystemVolume(0.0);

	RECT rect;
	GetWindowRect(GetDesktopWindow(), &rect);
	SetCursorPos(rect.right, rect.bottom);
}

void unlock() {
	ShowWindow(consoleHWND, false);

	SetCursorPos(lastCursorPos.x, lastCursorPos.y);

	setSystemVolume(lastVolume);
	lastVolume = 0.0;

	consoleInputActive = false;
}

DLLExport LRESULT CALLBACK keyboardEvent(int nCode, WPARAM wParam, LPARAM lParam) {
	if ((nCode == HC_ACTION && wParam == WM_SYSKEYDOWN) || wParam == WM_KEYDOWN) {
		KBDLLHOOKSTRUCT hookKey = *((KBDLLHOOKSTRUCT*)lParam);

		if (GetAsyncKeyState(VK_CONTROL) != 0) {
			if (hookKey.vkCode == VK_OEM_3) {
				if (!consoleInputActive) {
					lock();
				}
				else {
					unlock();
				}
			}
		}
	}

	return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

DWORD WINAPI hotkeyRoutine(LPVOID lpParm) {
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	if (!hInstance) {
		hInstance = LoadLibrary((LPCWSTR)lpParm);
		return 1;
	}

	hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)keyboardEvent, hInstance, NULL);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(hKeyboardHook);

	return 0;
}

int main(int argc, char** argv) {
	// No need to show main console window. To stop the application kill the 'desktop-hider.exe' in task list.
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);

	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode = 0;

	GetConsoleMode(hStdin, &mode);
	SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));

	// Resize console window to hide scrollbar
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO scrBufferInfo;

	GetConsoleScreenBufferInfo(hOut, &scrBufferInfo);

	COORD size;
	size.X = scrBufferInfo.dwSize.X;
	size.Y = scrBufferInfo.srWindow.Bottom - scrBufferInfo.srWindow.Top + 1;

	SetConsoleScreenBufferSize(hOut, size);
	// Hide underscore
	CONSOLE_CURSOR_INFO cursorInfo;

	GetConsoleCursorInfo(hOut, &cursorInfo);

	cursorInfo.bVisible = false;

	SetConsoleCursorInfo(hOut, &cursorInfo);

	HANDLE hThread;
	DWORD dwThread;

	// Initialize volume master earlier
	(void)getEndpointVolume();

	hThread = CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)hotkeyRoutine, (LPVOID)argv[0], NULL, &dwThread);

	ShowWindow(consoleHWND, false);

	if (hThread) {
		WaitForSingleObject(hThread, INFINITE);
	}
	else {
		return 1;
	}
}