/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:	Output.cpp		A collection of functions that will be responsible
--								for sending data frames over the link from the output 
--								queue as well as the control frames.
-- PROGRAM:		BCP
--
-- FUNCTIONS:
--	BOOL WriteOut(byte* frame, unsigned len)
--	BOOL SendNext();
--	BOOL Resend();
--	BOOL SendACK();
--	BOOL SendNAK();
--	BOOL SendENQ();
--	BOOL SendEOT();
--
-- DATE: 		November 02, 2013
--
-- REVISIONS: 	none
--
-- DESIGNER: 	Andrew Burian
--
-- PROGRAMMER: 	Andrew Burian
--
-- NOTES:
-- All functions return a boolean success value.
----------------------------------------------------------------------------------------------------------------------*/
#include "BCP.h"

byte dataFrame[1024] = { NULL };
byte ctrlFrame[2] = { NULL };
int SOTval = 1;

queue<BYTE> *quOutputQueue = NULL;
HANDLE *hOutputCommPort = NULL;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	WriteOut
--
-- DATE: 		November 2, 2013
--
-- REVISIONS: 	none
--
-- DESIGNER: 	Andrew Burian
--
-- PROGRAMMER: 	Andrew Burian
--
-- INTERFACE: 	BOOL WriteOut
--
-- RETURNS: 	BOOL
--
-- NOTES:
-- Syncronously writes the buffer (frame) to length (len) to the serial port.
----------------------------------------------------------------------------------------------------------------------*/
BOOL WriteOut(byte* frame, unsigned len)
{
	OVERLAPPED ovrOut = { 0 };
	ovrOut.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	// Start Async write
	WriteFile(*hOutputCommPort, frame, len, NULL, &ovrOut);
	
	// wait for event imbedded in overlapped struct
	int result = WaitForSingleObject(ovrOut.hEvent, INFINITE);

	ResetEvent(ovrOut.hEvent);
	
	switch (result)
	{
		case WAIT_OBJECT_0:
			return TRUE;
		case WAIT_TIMEOUT:
			return FALSE;
		case WAIT_ABANDONED:
			return FALSE;
	}

	// how you would get here is beyond me, but probably failed horribly
	return FALSE;

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	Resend
--
-- DATE: 		November 2, 2013
--
-- REVISIONS: 	none
--
-- DESIGNER: 	Andrew Burian
--
-- PROGRAMMER: 	Andrew Burian
--
-- INTERFACE: 	BOOL Resend()
--
-- RETURNS: 	BOOL
--
-- NOTES:
-- Sends the previously sent frame again using WriteOut.
----------------------------------------------------------------------------------------------------------------------*/
BOOL Resend()
{
	if (dataFrame[0] == NULL)
	{
		// no previously sent frame.
		// failed to resend
		return FALSE;
	}

	// write to port
	return WriteOut(dataFrame, 1024);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	SendNext
--
-- DATE: 		November 2, 2013
--
-- REVISIONS: 	none
--
-- DESIGNER: 	Andrew Burian
--
-- PROGRAMMER: 	Andrew Burian
--
-- INTERFACE: 	BOOL SendNext()
--
-- RETURNS: 	BOOL
--
-- NOTES:
-- Packetizes and sends a new data frame using WriteOut.
----------------------------------------------------------------------------------------------------------------------*/
BOOL SendNext()
{
	// check for no data to send
	if (quOutputQueue->empty())
		return FALSE;

	// start of frame
	dataFrame[0] = SYN;

	//SOT byte
	if (SOTval == 1)
	{
		dataFrame[1] = DC1;
		SOTval = 2;
	}
	else if (SOTval == 2)
	{
		dataFrame[1] = DC2;
		SOTval = 1;
	}


	// data portion
	int i = 2;
	for (i = 2; i < 1022; ++i)
	{
		if (quOutputQueue->empty())
			break;
		dataFrame[i] = quOutputQueue->front();
		quOutputQueue->pop();
	}
	// pad if nessesary
	while (i < 1022)
	{
		dataFrame[i] = NULL;
		++i;
	}

	// add crc
	if (!MakeCRC(&dataFrame[0], &dataFrame[1022]))
		return FALSE;

	// write to port
	return WriteOut(dataFrame, 1024);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	SendACK
--
-- DATE: 		November 2, 2013
--
-- REVISIONS: 	none
--
-- DESIGNER: 	Andrew Burian
--
-- PROGRAMMER: 	Andrew Burian
--
-- INTERFACE: 	BOOL SendACK()
--
-- RETURNS: 	BOOL
--
-- NOTES:
-- Sends and ACK using WriteOut.
----------------------------------------------------------------------------------------------------------------------*/
BOOL SendACK()
{
	ctrlFrame[0] = SYN;
	ctrlFrame[1] = ACK;
	return WriteOut(ctrlFrame, 2);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	SendNAK
--
-- DATE: 		November 2, 2013
--
-- REVISIONS: 	none
--
-- DESIGNER: 	Andrew Burian
--
-- PROGRAMMER: 	Andrew Burian
--
-- INTERFACE: 	BOOL SendNAK()
--
-- RETURNS: 	BOOL
--
-- NOTES:
-- Sends a NAK using WriteOut.
----------------------------------------------------------------------------------------------------------------------*/
BOOL SendNAK()
{
	ctrlFrame[0] = SYN;
	ctrlFrame[1] = NAK;
	return WriteOut(ctrlFrame, 2);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	SendENQ
--
-- DATE: 		November 2, 2013
--
-- REVISIONS: 	none
--
-- DESIGNER: 	Andrew Burian
--
-- PROGRAMMER: 	Andrew Burian
--
-- INTERFACE: 	BOOL SendENQ()
--
-- RETURNS: 	BOOL
--
-- NOTES:
-- Sends and ENQ using WriteOut.
----------------------------------------------------------------------------------------------------------------------*/
BOOL SendENQ()
{
	ctrlFrame[0] = SYN;
	ctrlFrame[1] = ENQ;
	return WriteOut(ctrlFrame, 2);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	SendEOT
--
-- DATE: 		November 2, 2013
--
-- REVISIONS: 	none
--
-- DESIGNER: 	Andrew Burian
--
-- PROGRAMMER: 	Andrew Burian
--
-- INTERFACE: 	BOOL SendEOT()
--
-- RETURNS: 	BOOL
--
-- NOTES:
-- Sends and EOT using WriteOut.
----------------------------------------------------------------------------------------------------------------------*/
BOOL SendEOT()
{
	ctrlFrame[0] = SYN;
	ctrlFrame[1] = EOT;
	SOTval = 1;
	return WriteOut(ctrlFrame, 2);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	SetupOutput
--
-- DATE: 		November 2, 2013
--
-- REVISIONS: 	none
--
-- DESIGNER: 	Andrew Burian
--
-- PROGRAMMER: 	Andrew Burian
--
-- INTERFACE: 	VOID SetupOutput()
--
-- RETURNS: 	VOID
--
-- NOTES:
-- Sets up the output with pointers to the output queue and a handle to the comm port.
----------------------------------------------------------------------------------------------------------------------*/
VOID SetupOutput(SHARED_DATA_POINTERS* dat)
{
	quOutputQueue = dat->p_quOutputQueue;
	hOutputCommPort = dat->p_hCommPort;
}