#include "BCP.h"

int sent = 0, received = 0, lost = 0;
HWND hwndMainWin = NULL;
HWND hwndTextBox = NULL;

VOID SetupGUI(HWND main, HWND edit)
{
	hwndMainWin = main;
	hwndTextBox = edit;
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