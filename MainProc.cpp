#include "BCP.h"

#define BTN_CONNECT 5001
#define BTN_SEND	5002

HDC hdc;
HWND edit, btn1, btn2;
BOOL bMasterProgramDone = FALSE;
queue<BYTE> quMasterOutputQueue;
queue<BYTE> quMasterInputQueue;
HANDLE hMasterCommPort = NULL;
LPCSTR szFileToSendNake = NULL;
HANDLE threads[4] = { 0 };
HANDLE hMasterProgramDoneEvent = CreateEvent(NULL, TRUE, FALSE, EVENT_END_PROGRAM);
BOOL bConnected = FALSE;
TCHAR fileName[MAX_PATH];
TCHAR titleName[MAX_PATH];

OPENFILENAME ofn;

COMMCONFIG cc;

SHARED_DATA_POINTERS MasterDat;

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message,
	WPARAM wParam, LPARAM lParam)
{

	switch (Message)
	{
	case WM_CREATE:
		edit = CreateWindowEx(
		 WS_EX_CLIENTEDGE, TEXT("Edit"), NULL,
		 WS_CHILD | WS_VISIBLE | WS_BORDER|ES_MULTILINE|ES_AUTOHSCROLL|WS_VSCROLL|ES_READONLY,
		 20, 20, 350,530, hwnd, NULL, NULL, NULL); 

		btn1 = CreateWindow(TEXT("Button"), TEXT("Connect"), WS_CHILD | BS_PUSHBUTTON, 
			400, 150, 80, 20, hwnd, (HMENU)BTN_CONNECT, NULL, NULL);

		btn2 = CreateWindow(TEXT("Button"), TEXT("Send File"), WS_CHILD | BS_PUSHBUTTON, 
			400, 200, 80, 20, hwnd, (HMENU)BTN_SEND, NULL, NULL);

		ShowWindow(edit, SW_SHOW);
		ShowWindow(btn1, SW_SHOW);
		ShowWindow(btn2, SW_SHOW);
		SetupGUI(hwnd, edit);
		break;

	case WM_PAINT:
		BeginPaint(hwnd, NULL);
		hdc = GetDC(hwnd);
		SelectObject(hdc, GetStockBrush(NULL_BRUSH));
		Rectangle(hdc, 400, 300, 750, 550);	// left, top, right, bottom
		TextOut(hdc, 410, 310, TEXT("Packet stats"), 12);
		TextOut(hdc, 410, 350, TEXT("Sent:"), 5);
		TextOut(hdc, 410, 380, TEXT("Received:"), 9);
		TextOut(hdc, 410, 410, TEXT("Lost:"), 5);
		EndPaint(hwnd, NULL);
		
		ReleaseDC(hwnd, hdc);
		break;
	case WM_COMMAND:
		switch (wParam)
		{
		case BTN_CONNECT:
			// Ensure handle is closed already
			CloseHandle(hMasterCommPort);
			
			// open
			hMasterCommPort = CreateFile(TEXT("COM1"), GENERIC_READ | GENERIC_WRITE, 0, 
				NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
			
			if (hMasterCommPort == INVALID_HANDLE_VALUE)
			{
				MessageBox(hwnd, TEXT("Comm port failed"), TEXT("Error"), MB_ICONERROR | MB_OK);
				break;
			}
			
			cc.dwSize = sizeof(COMMCONFIG);
			cc.wVersion = 0x100;
			
			GetCommConfig(hMasterCommPort, &cc, &cc.dwSize);
			if (!CommConfigDialog(TEXT("COM1"), hwnd, &cc))
			{
				MessageBox(hwnd, TEXT("You did not finish connecting"), TEXT("Alert"), MB_ICONWARNING | MB_OK);
				break;
			}

			bConnected = TRUE;

			MasterDat.p_hCommPort = &hMasterCommPort;
			MasterDat.p_bProgramDone = &bMasterProgramDone;
			MasterDat.p_quInputQueue = &quMasterInputQueue;
			MasterDat.p_quOutputQueue = &quMasterOutputQueue;

			SetupOutput(&MasterDat);

			threads[0] = CreateThread(NULL, NULL, ProtocolControlThread, (LPVOID)&MasterDat, NULL, NULL);
			threads[1] = CreateThread(NULL, NULL, SerialReadThread, (LPVOID)&MasterDat, NULL, NULL);
			threads[2] = CreateThread(NULL, NULL, FileWriterThread, (LPVOID)&MasterDat, NULL, NULL);
			break;

		case BTN_SEND:
			if (bConnected)
			{
				FileInitialize(hwnd, &ofn);
				ofn.hwndOwner = hwnd;
				ofn.lpstrFile = fileName;
				ofn.lpstrFileTitle = titleName;
				if(!GetOpenFileName(&ofn))
				{
					//error
					break;
				}

				MasterDat.p_outFileName=fileName;
				

				threads[3] = CreateThread(NULL, NULL, FileBufferThread, (LPVOID)&MasterDat, NULL, NULL);
			}
			
			else
			{
				MessageBox(hwnd, TEXT("Not connected. Please select connect first."), TEXT("Error"), MB_OK | MB_ICONERROR);
			}
			break;
		}
		break;

	case WM_DESTROY:
		if (!quMasterOutputQueue.empty())
		{
			int msgresult = MessageBox(hwnd, TEXT("Files are currently being sent. Are you sure you wish to exit?"), 
				TEXT("Are you sure?"), 
				MB_OKCANCEL | MB_ICONEXCLAMATION);
			if (msgresult != IDOK)
				break;
		}
		bMasterProgramDone = TRUE;
		SetEvent(hMasterProgramDoneEvent);

		WaitForMultipleObjects(4, threads, TRUE, INFINITE);
		PostQuitMessage(0);
		break;

	default: // Let Win32 process all other messages
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}