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
--			Revised code to actually append.  Used part of a Microsoft example.
--			Left code to replace text in edit box for testing purposes
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
	//This code just displays each packet in the edit box
	//SetWindowText(hwndTextBox, text);

	//This code appends new packets into the edit box
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
	
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: GUI_Text
--
-- DATE: November 30, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Chris Holisky
--
-- PROGRAMMER: Chris Holisky
--
-- INTERFACE: VOID update()
--
-- RETURNS: void;
-- 
-- NOTES:
-- This function is draws the packet sent, lost and received statistics onto the window
--
----------------------------------------------------------------------------------------------------------------------*/
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