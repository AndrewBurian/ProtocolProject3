/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Controller.cpp		Functions in this file call appropriate functions depending on events received.
--
-- PROGRAM:		BCP
--
-- FUNCTIONS:
--	DWORD WINAPI ProtocolControlThread(LPVOID params)
--
-- DATE: 		November 25, 2013
--
-- REVISIONS: 	None
--
-- DESIGNER: 	Shane Spoor
--
-- PROGRAMMER: 	Shane Spoor
--
-- NOTES: Functions in this file handle events from the other parts of the program.
--
----------------------------------------------------------------------------------------------------------------------*/
#include <tchar.h>
#include "BCP.h"

// TxProc/RxProc Return value if the program ends while sending or receiving
#define RET_END_PROGRAM			-1

// TxProc
// return values
#define TX_RET_SUCCESS				0
#define TX_RET_NO_ENQ				1
#define TX_RET_EXCEEDED_RETRIES		2
// other stuff
#define MAX_RETRIES	SEND_LIMIT - 1

// RxProc
// return vales
#define RX_RET_SUCCESS			0
#define RX_RET_DATA_TIMEOUT		1

static int TxProc();
static int RxProc();
static int AttemptRetransmission(int *send_count, HANDLE *hEvents, int *signaled);
static void MessageError(const TCHAR* message);
queue<BYTE> *output_queue = NULL;
HANDLE hQueueMutex		  = CreateMutex(NULL, FALSE, LOCK_OUTPUT);


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	ProtocolControlThread
--
-- DATE: 		November 30, 2013
--
-- REVISIONS: 	none
--
-- DESIGNER: 	Shane Spoor
--
-- PROGRAMMER: 	Shane Spoor
--
-- INTERFACE: 	DWORD WINAPI ProtocolControlThread(LPVOID params)
--					LPVOID params Pointer to a struct containing data pointers used across the program.
--
-- RETURNS: 	A DWORD indicating the thread's exit status (this should be 0).
--
-- NOTES:
-- Waits continuously for output to become available or an ENQ to be received and calls the corresponding functions
-- when they are.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI ProtocolControlThread(LPVOID params)
{
	int	   signaled		= -1;
	HANDLE hEvents[3]	= { CreateEvent(NULL, FALSE, FALSE, EVENT_ENQ),
							CreateEvent(NULL, FALSE, FALSE, EVENT_OUTPUT_AVAILABLE),
							CreateEvent(NULL, TRUE, FALSE, EVENT_END_PROGRAM)};
	output_queue		= ((SHARED_DATA_POINTERS*)params)->p_quOutputQueue;
	BOOL *bProgramDone	= ((SHARED_DATA_POINTERS*)params)->p_bProgramDone;


	while (!*bProgramDone)
	{
		int retVal;

		WaitForSingleObject(hQueueMutex, INFINITE);
		if(!output_queue->empty())
			SetEvent(hEvents[1]);		// if there is still output to send, make sure the event is set
		ReleaseMutex(hQueueMutex);

		signaled = WaitForMultipleObjects(3, hEvents, FALSE, INFINITE);

		switch(signaled)
		{
		
		case WAIT_OBJECT_0: // ENQ received
			retVal = RxProc();

			if (retVal == RET_END_PROGRAM)
			{
				*bProgramDone = TRUE;
				break;
			}
			else if (retVal == RX_RET_DATA_TIMEOUT)
			{
				// Do something with packet statistics
			}
			break;
		case WAIT_OBJECT_0 + 1: // Output availble
			retVal = TxProc();

			if (retVal == RET_END_PROGRAM)
			{
				*bProgramDone = TRUE;
				break;
			}
			else if (retVal == TX_RET_EXCEEDED_RETRIES)
			{
				MessageBox(NULL, TEXT("Exceeded retransmission attempts."), TEXT("Exceeded Retries"), MB_OK);
				//Do something with packet statistics
			}
			break;
		case WAIT_FAILED:
			MessageError(TEXT("Waiting for ENQ or Output Available in ProtocolControlThread failed"));
			SetEvent(hEvents[2]);
			*bProgramDone = TRUE;
			return 0;
		}
	}
	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	TxProc
--
-- DATE: 		November 30, 2013
--
-- REVISIONS: 	none
--
-- DESIGNER: 	Shane Spoor
--
-- PROGRAMMER: 	Shane Spoor
--
-- INTERFACE: 	static int TxProc()
--
-- RETURNS: 	TX_RET_SUCCESS if the packets are all transmitted succesfully; TX_RET_EXCEEDED_RETRIES if the function
--				fails to resend a packet; and RET_END_PROGRAM if the program ends during transmission or wait failed.
--
-- NOTES:
-- Called when a packet is lost to attempt retransmission. If this returns MAX_RETRIES, the ProtocolControlThread will
-- notify the user and ask whether they want to try again.
----------------------------------------------------------------------------------------------------------------------*/
static int TxProc()
{
	int		signaled	= -1;
	int		send_count	= 0; // The ENQ doesn't count as a packet, so this starts at -1
	int		retries		= 0;
	HANDLE	hEvents[] = { CreateEvent(NULL, FALSE, FALSE, EVENT_END_PROGRAM),
						  CreateEvent(NULL, FALSE, FALSE, EVENT_ACK),
						  CreateEvent(NULL, FALSE, FALSE, EVENT_NAK),
						  CreateEvent(NULL, FALSE, FALSE, EVENT_ENQ) };

	SendENQ();

	while (send_count < SEND_LIMIT && retries < MAX_RETRIES)
	{
		WaitForSingleObject(hQueueMutex, INFINITE);
		if(output_queue->empty())
		{
			ReleaseMutex(hQueueMutex);
			break;
		}
		ReleaseMutex(hQueueMutex);

		signaled = WaitForMultipleObjects(4, hEvents, FALSE, TIMEOUT);
		switch (signaled)
		{
		case WAIT_OBJECT_0:		// End of program
			return RET_END_PROGRAM;
		case WAIT_OBJECT_0 + 1: // ACK Received
			SendNext();
			++send_count;
			retries = 0;
			GUI_Sent();
			break;
		case WAIT_OBJECT_0 + 2:
		case WAIT_TIMEOUT:		// NAK or timed out; resend the packet max of 5 times
			if(send_count == 0)	// The ENQ hasn't been ACK'd; go back to idle and try again
				return TX_RET_SUCCESS;
			GUI_Lost();
			--send_count;
			++retries;
			Resend();

			break;

		case WAIT_OBJECT_0 + 3:
			Sleep((rand() % TIMEOUT) + TIMEOUT);
			return TX_RET_SUCCESS;

		case WAIT_FAILED: // something's clearly gone horribly wrong; display a message and exit
			MessageError(TEXT("Waiting for an acknowledgement in SendProc failed"));
			SetEvent(hEvents[0]); // Set the End of Program event to allow everyone else to clean up
			return RET_END_PROGRAM;
		}
	}

	// if we've gotten out of the loop, then there's no more data to send or the limit was reached
	if(retries == MAX_RETRIES)
	{
		WaitForSingleObject(hQueueMutex, INFINITE);
		ClearOutputQueue();
		ReleaseMutex(hQueueMutex);
		return TX_RET_EXCEEDED_RETRIES;
	}
	SendEOT();
	Sleep(TIMEOUT);		// Give time for the other side to grab the line.
	return TX_RET_SUCCESS;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	RxProc
--
-- DATE: 		November 29, 2013
--
-- REVISIONS: 	none
--
-- DESIGNER: 	Shane Spoor
--
-- PROGRAMMER: 	Shane Spoor
--
-- INTERFACE: 	static int RxProc
--
-- RETURNS: 	RX_RET_SUCCESS on success, or RX_END_PROGRAM if there was a fatal error or another process set the
--				end of program event.
--
-- NOTES:
-- Calls appropriate event handlers depending on data received from the other host.
----------------------------------------------------------------------------------------------------------------------*/
static int RxProc()
{
	int	   signaled		= -1;
	HANDLE hEvents[4] = { CreateEvent(NULL, FALSE, FALSE, EVENT_END_PROGRAM),
						  CreateEvent(NULL, FALSE, FALSE, EVENT_DATA_RECEIVED),
						  CreateEvent(NULL, FALSE, FALSE, EVENT_BAD_DATA_RECEIVED),
						  CreateEvent(NULL, FALSE, FALSE, EVENT_EOT)};
							
	SendACK(); // Acknowledge the ENQ

	while (TRUE)
	{
		signaled = WaitForMultipleObjects(4, hEvents, FALSE, TIMEOUT);
		switch (signaled)
		{
		case WAIT_OBJECT_0:				// End of program
			return RET_END_PROGRAM;
		case WAIT_OBJECT_0 + 1:			// Good data received
			GUI_Received();
			SendACK();
			break;
		case WAIT_OBJECT_0 + 2:			// Bad data received; send NAK
			GUI_ReceivedBad();
			SendNAK();
			break;
		case WAIT_OBJECT_0 + 3:			// EOT, so return RX_RET_SUCCESS
			return RX_RET_SUCCESS;
		case WAIT_TIMEOUT:
			return RX_RET_DATA_TIMEOUT;
		case WAIT_FAILED:				// Something went wrong; end the program
			MessageError(TEXT("Waiting for data, EOT or end of program in RxProc failed"));
			SetEvent(hEvents[0]);
			return RET_END_PROGRAM;
		}
	}
	MessageError(TEXT("RxProc failed due to an unforeseen error")); // How have we even gotten out here? This isn't good
	SetEvent(hEvents[0]);											// There's probably been some catastrophic failure, so end the program
	return RET_END_PROGRAM;				
}



/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	AttemptRetransmission
--
-- DATE: 		November 25, 2013
--
-- REVISIONS: 	none
--
-- DESIGNER: 	Shane Spoor
--
-- PROGRAMMER: 	Shane Spoor
--
-- INTERFACE: 	int AttemptRetransmission(size_t *send_count, HANDLE *hEvents)
--					size_t *send_count: Pointer to variable holding the number of packets successfully sent.
--					HANDLE *hEvents:	Array of events to wait on while retransmitting.
--					int *signaled		Pointer to the return value for WaitForMultipleObjects.
--
-- RETURNS: 	Number of retransmission attempts.
--
-- NOTES:
-- Called when a packet is lost to attempt retransmission. If this returns MAX_RETRIES, the ProtocolControlThread
----------------------------------------------------------------------------------------------------------------------*/
static int AttemptRetransmission(int *send_count, HANDLE *hEvents, int *signaled)
{
	int i;
	for (i = 0; i < MAX_RETRIES; ++i)
	{
		Resend();
		GUI_Sent();
		*++send_count;
		
		*signaled = WaitForMultipleObjects(4, hEvents, FALSE, TIMEOUT); // Wait for ACK/NAK
		if (*signaled == WAIT_OBJECT_0 + 1)
			break;
	
		GUI_Lost();
		*--send_count;
	}
	return i;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	MessageError
--
-- DATE: 		November 26, 2013
--
-- REVISIONS: 	none
--
-- DESIGNER: 	Shane Spoor
--
-- PROGRAMMER: 	Shane Spoor
--
-- INTERFACE: 	int MessageError(const TCHAR* message)
--					const TCHAR *message: The error message to print.
--
-- RETURNS: 	void
--
-- NOTES:
-- Displays a message box containing a formatted error message and the last error from the calling thread.
----------------------------------------------------------------------------------------------------------------------*/
void MessageError(const TCHAR* message)
{

	TCHAR err_msg[256];
	const TCHAR *err = TEXT("Fatal Error: %s (error code : %d). Exiting the program.");
	_sntprintf_s(err_msg, 256, err, message, GetLastError());
	MessageBox(NULL, err_msg, TEXT("Fatal Error"), MB_ICONERROR);
}