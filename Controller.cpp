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
static void MessageError(const TCHAR* message);
queue<BYTE> *output_queue = NULL;

DWORD WINAPI ProtocolControlThread(LPVOID params)
{
	int	   signaled		= -1;
	HANDLE hEvents[3]	= { CreateEvent(NULL, FALSE, FALSE, EVENT_ENQ),
							CreateEvent(NULL, FALSE, FALSE, EVENT_OUTPUT_AVAILABLE),
							CreateEvent(NULL, TRUE, FALSE, EVENT_END_PROGRAM) };
	output_queue		= ((SHARED_DATA_POINTERS*)params)->p_quOutputQueue;

	while ((signaled = WaitForMultipleObjects(3, hEvents, FALSE, INFINITE)) != WAIT_OBJECT_0 + 2)
	{
		int retVal;

		if (signaled == WAIT_OBJECT_0) // ENQ received
		{
			//MessageBox(NULL, TEXT("ENQ received"), TEXT("Message"), MB_OK);
			retVal = RxProc();

			if (retVal == RET_END_PROGRAM)
				break;
			else if (retVal == RX_RET_DATA_TIMEOUT)
			{
				// Do something with packet statistics
			}
		}
		else if (signaled - WAIT_OBJECT_0 + 1) // Output availble
		{
			//MessageBox(NULL, TEXT("Output Available"), TEXT("Message"), MB_OK);
			retVal = TxProc();

			if (retVal == RET_END_PROGRAM)
				break;
			else if (retVal == TX_RET_EXCEEDED_RETRIES)
			{
				MessageBox(NULL, TEXT("Exceeded retransmission attempts."), TEXT("Exceeded Retries"), MB_OK);
				//Do something with packet statistics
			}
		}
		else if (signaled == WAIT_FAILED)
		{
			MessageError(TEXT("Waiting for ENQ or Output Available in ProtocolControlThread failed"));
			SetEvent(hEvents[2]);
			return 0;
		}
	}
	return 0;
}

int TxProc()
{
	int		signaled	= -1;
	HANDLE	hEvents[] = { CreateEvent(NULL, FALSE, FALSE, EVENT_END_PROGRAM),
						  CreateEvent(NULL, FALSE, FALSE, EVENT_ACK),
						  CreateEvent(NULL, FALSE, FALSE, EVENT_NAK) };
	size_t send_count = 0;

	SendENQ();

	while (!output_queue->empty() && send_count < SEND_LIMIT)
	{
		signaled = WaitForMultipleObjects(3, hEvents, FALSE, TIMEOUT); // possible issue: program ends after timeout
		switch (signaled)
		{
		case WAIT_OBJECT_0:		// End of program
			return RET_END_PROGRAM;
		case WAIT_OBJECT_0 + 1: // ACK Received
			//MessageBox(NULL, TEXT("SendNext() in TxProc"), TEXT("SendNext()"), MB_OK);
			SendNext();
			++send_count;
			GUI_Sent();
			break;
		case WAIT_OBJECT_0 + 2:
		case WAIT_TIMEOUT:		// NAK or timed out; resend the packet max of 5 times
		{
			//MessageBox(NULL, TEXT("Resend() in TxProc"), TEXT("Resend()"), MB_OK);
			GUI_Lost();
			--send_count;
			size_t i;
			for (i = 0; i < MAX_RETRIES; ++i)
			{
				Resend();
				GUI_Sent();
				++send_count;

				signaled = WaitForSingleObject(hEvents[1], TIMEOUT); // Wait for an ACK

				if (signaled == WAIT_OBJECT_0)
					break;

				GUI_Lost();
				--send_count;
			}
			
			if (i == MAX_RETRIES)
			{
				ClearOutputQueue();
				return TX_RET_EXCEEDED_RETRIES;
			}
			break;
		}

		case WAIT_FAILED: // something's clearly gone horribly wrong; display a message and exit
			MessageError(TEXT("Waiting for an acknowledgement in SendProc failed"));
			SetEvent(hEvents[0]); // Set the End of Program event to allow everyone else to clean up
			return RET_END_PROGRAM;
		}
	}

	// if we've gotten out of the loop, then there's no more data to send
	SendEOT();
	return TX_RET_SUCCESS;
}

int RxProc()
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
			//MessageBox(NULL, TEXT("SendACK() in RxProc"), TEXT("SendACK()"), MB_OK);
			GUI_Received();
			SendACK();
			break;
		case WAIT_OBJECT_0 + 2:			// Bad data received; do nothing
			//MessageBox(NULL, TEXT("Bad data received in RxProc"), TEXT("Bad data"), MB_OK);
			GUI_ReceivedBad();
			break;
		case WAIT_OBJECT_0 + 3:			// EOT, so return RX_RET_SUCCESS
			//MessageBox(NULL, TEXT("Received EOT in RxProc"), TEXT("Rx EOT"), MB_OK);
			return RX_RET_SUCCESS;
		case WAIT_TIMEOUT:
			//MessageBox(NULL, TEXT("Data timed out in RxProc"), TEXT("Timeout"), MB_OK);
			return RX_RET_DATA_TIMEOUT;
		case WAIT_FAILED:				// Something went wrong; end the program
			//MessageError(TEXT("Waiting for data, EOT or end of program in RxProc failed"));
			SetEvent(hEvents[0]);
			return RET_END_PROGRAM;
		}
	}
	MessageError(TEXT("RxProc failed due to an unforeseen error")); // How have we even gotten out here? This isn't good
	SetEvent(hEvents[0]);											// There's probably been some catastrophic failure, so end the program
	return RET_END_PROGRAM;				
}


// Displays a formatted error message containing the error code from GetLastError.
void MessageError(const TCHAR* message)
{

	TCHAR err_msg[256];
	const TCHAR *err = TEXT("Fatal Error: %s (error code : %d). Exiting the program.");
	_sntprintf_s(err_msg, 256, err, message, GetLastError());
	MessageBox(NULL, err_msg, TEXT("Fatal Error"), MB_ICONERROR);
}