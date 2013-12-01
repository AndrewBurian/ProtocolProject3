#include "BCP.h"
#include <queue>
//#include <deque>
//#include <list>
//#include <string>

HANDLE hOutputReady = CreateEvent(NULL, FALSE, FALSE, EVENT_OUTPUT_AVAILABLE);


HANDLE hInputEvents[2] = {CreateEvent(NULL, FALSE, FALSE, EVENT_INPUT_AVAILABLE),
	CreateEvent(NULL, FALSE, FALSE, EVENT_END_PROGRAM)};


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: FileBufferThread
--
-- DATE: November 19, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Chris Holisky
--
-- PROGRAMMER: Chris Holisky
--
-- INTERFACE: DWORD WINAPI FileBufferThread(LPVOID threadParams)
-- LPVOID threadParams : Pointer to struct that will contain file name
--
-- RETURNS: returns 0 on success;
-- 
-- NOTES:
-- This function is used to write the file to be sent into the output queue.
--
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI FileBufferThread(LPVOID threadParams)
{
	TCHAR * fileName = ((SHARED_DATA_POINTERS*)threadParams)->p_outFileName;
	queue<BYTE> *outQueue=((SHARED_DATA_POINTERS*)threadParams)-> p_quOutputQueue;
	BYTE   bySwap ;
	DWORD  dwBytesRead ;
	HANDLE hFile ;
	int    i, iFileLength, iUniTest ;
	PBYTE  pBuffer;

	if (INVALID_HANDLE_VALUE == 
		(hFile = CreateFile (fileName, GENERIC_READ, FILE_SHARE_READ,
		//(hFile = CreateFile ((LPCWSTR)fileName, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, 0, NULL)))
		return FALSE ;

	iFileLength = GetFileSize (hFile, NULL) ; 
	pBuffer= (PBYTE)malloc (iFileLength + 2) ;
	ReadFile (hFile, pBuffer, iFileLength, &dwBytesRead, NULL) ;
	CloseHandle (hFile) ;
	pBuffer[iFileLength] = '\0' ;
	pBuffer[iFileLength + 1] = '\0' ;
	for (int i=0;i<iFileLength;++i){
		outQueue->push(pBuffer[i]);
		SetEvent(hOutputReady);
	}	
	free (pBuffer) ;

	return 0;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: FileWriterThread
--
-- DATE: November 19, 2013
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Chris Holisky
--
-- PROGRAMMER: Chris Holisky
--
-- INTERFACE: DWORD WINAPI FileWriterThread(LPVOID threadParams)
-- LPVOID threadParams : Pointer to struct that will contain file name
--
-- RETURNS: returns 0 on success;
-- 
-- NOTES:
-- This function takes in the data received from the receive buffer one byte at a time 
--	and sends it to a display function
--
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI FileWriterThread(LPVOID threadParams)
{
	char buffer[1023];
	int count=0;
	int waits;
	queue<BYTE> *inQueue=((SHARED_DATA_POINTERS*)threadParams)-> p_quInputQueue;
	BOOL progDone = *((SHARED_DATA_POINTERS*)threadParams)-> p_bProgramDone;
	inQueue->push('T');
	inQueue->push('h');
	inQueue->push('i');
	inQueue->push('s');
	inQueue->push(' ');
	inQueue->push('i');
	inQueue->push('s');
	inQueue->push(' ');
	inQueue->push('a');
	inQueue->push(' ');
	inQueue->push('t');
	inQueue->push('e');
	inQueue->push('s');
	inQueue->push('t');
	while(!(progDone))
	{
	//waits=	WaitForMultipleObjects(2, hInputEvents, FALSE, INFINITE);
	//if(waits==WAIT_OBJECT_0 + 1){break;}
		while(!(inQueue->empty()))
		{
			if(count==1022){break;}
			else{
				buffer[count]=(inQueue->front());
				count++;
				//get the length of the current buffer string, add the characters one at a time 
				//to the appropriate places then add the terminating null character
				//sprintf_s(buffer,1, %s%c, (CHAR*)inQueue->front());
				//if only every other character is printed, remove the pop
				inQueue->pop();
			}
		}
		if (count>0){
			buffer[count+1]='\0';
			//convert to wide char string
			size_t newsize = count + 1;
			wchar_t * wcstring = new wchar_t[newsize];
			size_t convertedChars = 0;
			mbstowcs_s(&convertedChars, wcstring, newsize, buffer, _TRUNCATE);
			//send buffer to display function
			GUI_Text(wcstring);
			buffer[0]='\0';
			count=0;
		}
	}
	return 0;
}