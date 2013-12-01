#include "BCP.h"

#include <tchar.h>
int sent = 0, received = 0, badReceived = 0, lost = 0;
HWND hwndMainWin = NULL;
HWND hwndTextBox = NULL;
VOID update();

VOID SetupGUI(HWND main, HWND edit)
{
	hwndMainWin = main;
	hwndTextBox = edit;
	update();
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: GUI_Text
--
-- DATE: November 19, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Chris Holisky
--
-- PROGRAMMER: Chris Holisky
--
-- INTERFACE: VOID GUI_Text(LPCSTR text)
-- LPCSTR text : Pointer to the text to be written to screen
--
-- RETURNS: void;
-- 
-- NOTES:
-- This function is used to write data to the edit box.  It appends new datato the currently existing text.
--
----------------------------------------------------------------------------------------------------------------------*/
VOID GUI_Text(TCHAR* text)
{
	//This might be what we have to use.  We'd have to create a buffer to 
	//hold all text and then append text to buffer and then redisplay it
	//SetWindowText(hwndTextBox, text);
	/*
	//Put window text in string buffer;
	int len = GetWindowTextLength(hwndTextBox);
	wchar_t * scrBuffer;
	wchar_t buffer[10000];
	GetWindowText(hwndTextBox, scrBuffer, len+1);
	(scrBuffer, sizeof(text), text);
	//Concatenate new text to window text
	*/
	//put total back on screen



	SetWindowText(hwndTextBox, text);
	/*
	int len = GetWindowTextLength(hwndTextBox);
	std::vector<TCHAR> temp(len + sizeof(text) + 1);

	GetWindowText(hwndTextBox, temp.data(), temp.size());
	wcscat_s(temp.data(),sizeof(text), (const wchar_t*)text);
	SetWindowText(hwndTextBox, temp.data());
	*/
}

VOID update()
{
	HDC hdc=	GetDC(hwndMainWin);

	TCHAR num[3];
	const TCHAR *fmt = TEXT("%d");

	_sntprintf_s(num, 10, fmt, sent);
	TextOut(hdc, 550, 350,num,2);

	_sntprintf_s(num, 10, fmt, lost);
	TextOut(hdc, 550, 380, num,2);

	_sntprintf_s(num, 10, fmt, received);
	TextOut(hdc, 550, 410, num, 2);

	_sntprintf_s(num, 10, fmt, badReceived);
	TextOut(hdc, 550, 440, num, 2);

	ReleaseDC(hwndMainWin, hdc);
}

VOID GUI_Sent()
{
	++sent;
	update();
}

VOID GUI_Received()
{
	++received;
	update();
}

VOID GUI_Lost()
{
	++lost;
	update();
}

VOID GUI_ReceivedBad()
{
	++badReceived;
	update();
}