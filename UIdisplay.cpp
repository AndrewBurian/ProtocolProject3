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
--			Dec 1, 2013
--			Chris Holisky
--			Changed function to take byte instead of char
--			Changed funtion to take only one argument
--
-- DESIGNER: Chris Holisky
--
-- PROGRAMMER: Chris Holisky
--
-- INTERFACE: VOID GUI_Text(TCHAR* text)
-- TCHAR* text : Text to be written on screen
--
-- RETURNS: void;
-- 
-- NOTES:
-- This function is used to write data to the edit box.  It appends new data to the currently existing text.
--
----------------------------------------------------------------------------------------------------------------------*/
VOID GUI_Text(TCHAR* text)
{
	//text=(TCHAR*)"This is a test.  This is only a test.  Please do not be alarmed.  If it works....";
	//This might be what we have to use.  We'd have to create a buffer to 
	//hold all text and then append text to buffer and then redisplay it
	//SetWindowText(hwndTextBox, text);
	/*
	//Put window text in string buffer;
	int len = GetWindowTextLength(hwndTextBox);
	wchar_t * scrBuffer[10000000];
	wchar_t buffer[10000];
	GetWindowText(hwndTextBox, (LPWSTR)scrBuffer, len+1);
	(scrBuffer, sizeof(text), text);
	//Concatenate new text to window text
	*/
	//put total back on screen

 //HWND hEdit = GetDlgItem (,);
   int ndx = GetWindowTextLength (hwndTextBox);
   SetFocus (hwndTextBox);
   #ifdef WIN32
      SendMessage (hwndTextBox, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
   #else
      SendMessage (hwndTextBox, EM_SETSEL, 0, MAKELONG (ndx, ndx));
   #endif
	     #ifdef WIN32
      SendMessage (hwndTextBox, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
   #else
      SendMessage (hwndTextBox, EM_SETSEL, 0, MAKELONG (ndx, ndx));
   #endif
      SendMessage (hwndTextBox, EM_REPLACESEL, 0, (LPARAM) ((LPSTR) text));
	//SetWindowText(hwndTextBox, (LPCWSTR)NULL);
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