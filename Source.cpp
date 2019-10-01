#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include "serial.h"

// TEXTBOX
#define IDC_PORT_SELECT 101
#define IDC_TX_OUTPUT 102
#define IDC_RX_INPUT 103

// BUTTONTS
#define ID_CONNECT 1
#define ID_DISCONNECT 2
#define ID_TRANSMIT 3
#define ID_CLEAR 4

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//global
HWND hw;
HANDLE readThread;
char RXbuf[30];

// get_last_error
char* error() {
	char lastError[1024];
	char error_msg[1024];
	strcpy(error_msg, "Error: ");
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		lastError,
		1024,
		NULL);

	strcat(error_msg, lastError);

	return error_msg;
}

// genereric format msg to user
void print_msg(char* msg) {
	MessageBox(NULL, msg, "Message", MB_OK);
}

// thread for synchronic read operation
DWORD WINAPI readFunc(PVOID Param) {
	strcpy(RXbuf, "");		// init, clear
	while(true){
		strcat(RXbuf, read());
		SetDlgItemTextA(hw, IDC_RX_INPUT, (LPCSTR)RXbuf);
		Sleep(500);		//sleep for 1/2 sec
		if (strlen(RXbuf) >= 30)
			strcpy(RXbuf, "");		//if RXbuf size >=30 than clear
	}
	return 0;
}

void CreateWindowObjects(HWND hwnd) {
	HWND hEditA, hEditTX, hEditRX, hEditCOM;//handle for text box

	// BUTTONS
	CreateWindowW(L"Button", L"Connect", WS_VISIBLE | WS_CHILD, 200, 20, 80, 25, hwnd, (HMENU)ID_CONNECT, NULL, NULL);
	CreateWindowW(L"Button", L"Disconnect", WS_VISIBLE | WS_CHILD, 200, 50, 80, 25, hwnd, (HMENU)ID_DISCONNECT, NULL, NULL);
	CreateWindowW(L"Button", L"Transmit", WS_VISIBLE | WS_CHILD, 200, 120, 80, 25, hwnd, (HMENU)ID_TRANSMIT, NULL, NULL);
	CreateWindowW(L"Button", L"Clear", WS_VISIBLE | WS_CHILD, 200, 230, 80, 25, hwnd, (HMENU)ID_CLEAR, NULL, NULL);

	// LABELS
	HWND lbl_port = CreateWindow("STATIC", "Port: (COM1,COM2...)", SS_LEFT | WS_CHILD, 20, 20, 200, 25, hwnd, NULL, NULL, NULL);
	ShowWindow(lbl_port, 1);
	HWND lbl_TX = CreateWindow("STATIC", "TX:", SS_LEFT | WS_CHILD, 20, 100, 50, 50, hwnd, NULL, NULL, NULL);
	ShowWindow(lbl_TX, 1);
	HWND lbl_RX = CreateWindow("STATIC", "RX:", SS_LEFT | WS_CHILD, 20, 180, 50, 25, hwnd, NULL, NULL, NULL);
	ShowWindow(lbl_RX, 1);

	// Port TextBox
	hEditCOM = CreateWindowEx(WS_EX_CLIENTEDGE,
		"EDIT", "",
		WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
		20, 40,	// x and y coordinates of the button in pixels
		150, 25,	//width and height of the button in pixels
		hwnd,	// handle of parent object
		(HMENU)IDC_PORT_SELECT,	// object ID
		GetModuleHandle(NULL), NULL);
	SetDlgItemTextA(hwnd, IDC_PORT_SELECT, "COM1");		//default port = "COM1"

	// TX TextBox
	hEditTX = CreateWindowEx(WS_EX_CLIENTEDGE,
		"EDIT", "",
		WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
		20, 120,
		150, 25,
		hwnd,
		(HMENU)IDC_TX_OUTPUT,
		GetModuleHandle(NULL), NULL);

	// RX TextBox
	hEditRX = CreateWindowEx(WS_EX_CLIENTEDGE,
		"EDIT", "",
		WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
		20, 200,
		250, 25,
		hwnd,
		(HMENU)IDC_RX_INPUT,
		GetModuleHandle(NULL), NULL);

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	hw = hwnd;
	HDC hdcStatic = NULL;
	switch (msg) {

		// create window's components
	case WM_CREATE:	
		CreateWindowObjects(hwnd);
		break;

		// make labels background transparent
	case WM_CTLCOLORSTATIC:	

		hdcStatic = (HDC)wParam;
		SetTextColor(hdcStatic, RGB(0, 0, 0));
		SetBkMode(hdcStatic, TRANSPARENT);

		return (LRESULT)GetStockObject(NULL_BRUSH);

		// buttons operations
	case WM_COMMAND:	

		// CLEAR Button clicked
		if (LOWORD(wParam) == ID_CLEAR) {
			strcpy(RXbuf, "");
			SetDlgItemTextA(hwnd, IDC_RX_INPUT, (LPCSTR)"");
		}

		// CONNECT Button clicked
		if (LOWORD(wParam) == ID_CONNECT) {
			TCHAR port[30];
			//HWND hEditWnd;
			//hEditWnd = GetDlgItem(hwnd, IDC_PORT_SELECT);
			GetWindowText(GetDlgItem(hwnd, IDC_PORT_SELECT), port, 30);
			if (open(port))
				print_msg((char*)"Connected");
			else
				print_msg(error());


			readThread = CreateThread(NULL,0,readFunc,NULL,0,NULL);
		}

		// DISCONNECT Button clicked
		if (LOWORD(wParam) == ID_DISCONNECT) {
			if(close())
				print_msg((char*)"Disconnected");
		}

		// TRANSMIT Button clicked
		if (LOWORD(wParam) == ID_TRANSMIT) {
			TCHAR data[30];
			//HWND hEditWnd;
			//hEditWnd = GetDlgItem(hwnd, IDC_TX_OUTPUT);
			int msgSize = GetWindowText(GetDlgItem(hwnd, IDC_TX_OUTPUT), data, 30);
			if(write(data, msgSize))
				print_msg((char*)"Message Transmitted");
			else
				print_msg(error());
		}
		break;

	case WM_DESTROY:	//window exit

		PostQuitMessage(0);
		CloseHandle(readThread);		//close thread habdle
		break;
	}

	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
	// window inital definitions
	MSG  msg;
	WNDCLASSW wc = { 0 };
	wc.lpszClassName = L"Terminal";
	wc.hInstance = hInstance;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpfnWndProc = WndProc;
	wc.hCursor = LoadCursor(0, IDC_ARROW);

	RegisterClassW(&wc);

	CreateWindowW(wc.lpszClassName, L"Terminal",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		150, 150, 300, 300, 0, 0, hInstance, 0);

	while (GetMessage(&msg, NULL, 0, 0)) {

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}