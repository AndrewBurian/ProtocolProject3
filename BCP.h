/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: BCP.h - 	header file for the Protocol Project
--
-- PROGRAM:		BCP
--
-- FUNCTIONS:
-- none
--
-- DATE: 		October 23, 2013
--
-- REVISIONS: 	November 27, 2013
--				Andrew Burian
--				Removed all global variables and replaced with defines and SHARED_DATA_POINTERS struct.
--
-- DESIGNER: 	Andrew Burian
--
-- PROGRAMMER: 	Andrew Burian
--
-- NOTES:
-- 
----------------------------------------------------------------------------------------------------------------------*/

// includes
// ----------------------------------------------------------------------------
using namespace std;
#include <Windows.h>
#include <Windowsx.h>
#include <queue>

// Definitions
// ----------------------------------------------------------------------------
#define TIMEOUT 200
#define SEND_LIMIT 5

#define ACK 0x06
#define NAK 0x15
#define EOT 0x04
#define ENQ 0x05
#define SYN 0x16
#define DC1 0x11
#define DC2 0x12

// Global Comm Port Object and utils
// ----------------------------------------------------------------------------
struct SHARED_DATA_POINTERS{
	HANDLE *p_hCommPort;
	queue<BYTE> *p_quOutputQueue;
	queue<BYTE> *p_quInputQueue;
	BOOL *p_bProgramDone;
	TCHAR * p_outFileName;
};


// Function Declarations
// ----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

DWORD WINAPI FileBufferThread(LPVOID threadParams);
DWORD WINAPI SerialReadThread(LPVOID threadParams);
DWORD WINAPI FileWriterThread(LPVOID threadParams);
DWORD WINAPI ProtocolControlThread(LPVOID threadParams);

//Output
BOOL SendNext();
BOOL Resend();
BOOL SendACK();
BOOL SendNAK();
BOOL SendEOT();
BOOL SendENQ();
VOID SetupOutput(SHARED_DATA_POINTERS* dat);

//CRC
BOOL MakeCRC(byte* input, byte* crc);
BOOL CheckCRC(byte* input, byte* crc);


// Global events
// ----------------------------------------------------------------------------
#define EVENT_ACK TEXT("BCP_ACK")
#define EVENT_NAK TEXT("BCP_NAK")
#define EVENT_ENQ TEXT("BCP_ENQ")
#define EVENT_EOT TEXT("BCP_EOT")
#define EVENT_DATA_RECEIVED TEXT("BCP_DATA_RECEIVED")
#define EVENT_BAD_DATA_RECEIVED TEXT("BCP_BAD_DATA_RECEIVED")
#define EVENT_END_PROGRAM TEXT("BCP_END_OF_PROGRAM")
#define EVENT_OUTPUT_AVAILABLE TEXT("BCP_OUTPUT_AVAILABLE")
#define EVENT_INPUT_AVAILABLE TEXT("BCP_INPUT_AVAILBLE")


// GUI functionallity
// ----------------------------------------------------------------------------

VOID SetupGUI(HWND main, HWND edit);
VOID GUI_Text(TCHAR* text);
VOID GUI_Sent();
VOID GUI_Received();
VOID GUI_Lost();

// Choose File popup
// ----------------------------------------------------------------------------
void FileInitialize(HWND hwnd, OPENFILENAME* ofn);
BOOL FileOpenDlg(HWND hwnd, PTSTR pstrFileName, OPENFILENAME *ofn);