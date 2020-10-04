#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <windows.h>
#include "serial_com.h"
#include <iostream>

using namespace std;


int open_serial(char *port_name, HANDLE &h)
{
	DCB param = { 0 };
	COMMTIMEOUTS CommTimeouts;

	printf("\nInitializing serial port and resetting Arduino.\n\nPlease wait ...");

	h = CreateFile(port_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (h == INVALID_HANDLE_VALUE) {
		printf("\nerror opening serial port");
		return 1; // error
	}

	// get current serial port parameters
	if (!GetCommState(h, &param)) {
		printf("\nerror: could not get serial port parameters");
		return 1;
	}

	param.StopBits = ONESTOPBIT;
	param.Parity = NOPARITY;
	param.ByteSize = 8;
	// note: the serial port driver should be set to the same data (baud) rate
	// in the device manager com port settings.
	// the uploader should also use the same rate.
	//	param.BaudRate    = CBR_9600;
	param.BaudRate = CBR_115200;
	//	param.BaudRate    = 500000;
	param.fDtrControl = DTR_CONTROL_ENABLE; // reset arduino with serial connection

	if (!SetCommState(h, &param)) {
		printf("\nerror: serial port parameters could not be set");
		return 1; // error
	}

	// set serial port time-out parameters for a total timeout of 5.5s
	CommTimeouts.ReadIntervalTimeout = 0; // not used
	CommTimeouts.ReadTotalTimeoutMultiplier = 0; // total time-out per byte
	CommTimeouts.ReadTotalTimeoutConstant = 10000; // add constant to get total time-out
	CommTimeouts.WriteTotalTimeoutMultiplier = 0; // not used
	CommTimeouts.WriteTotalTimeoutConstant = 0; // not used

	if (!SetCommTimeouts(h, &CommTimeouts)) {
		printf("\nerror: serial port time-out parameters could not be set");
		return 1; // error
	}

	PurgeComm(h, PURGE_RXCLEAR | PURGE_TXCLEAR);

	Sleep(100); // note: the extra delay helping / needed here
	PurgeComm(h, PURGE_RXCLEAR | PURGE_TXCLEAR);

	Sleep(3000); // wait for arduino to reset

	printf("\n\nSerial port and Arduino are ready.\n");

	return 0; // OK
}


int close_serial(HANDLE &h)
{
	if (h == INVALID_HANDLE_VALUE) {
		printf("\nerror closing serial port");
		return 1; // error
	}

	CloseHandle(h);
	h = INVALID_HANDLE_VALUE;

	return 0; // OK
}


int serial_recv(char *buffer, int n, HANDLE h)
// n - number of bytes desired/requested (0 < n <= nbuffer)
// note: this function will halt (ie block) the program as it waits for all n bytes.
// if you don't want to halt (block) then call n = serial_available() first
// and only call serial_recv (with n bytes) when n > 0.
// the time-out / blocking behaviour is invoked in my functions because
// I want to make sure the entire message is received before continuing the program
// and non-blocking can be easily achieved using the serial_available() function.
{
	DWORD nrecv;

	if (!ReadFile(h, (LPVOID)buffer, (DWORD)n, &nrecv, NULL)) {
		printf("\nerror: serial port read error");
		return 1; // error
	}

	if (nrecv != n) {
		printf("\nerror: serial port read time-out");
		return 1; // error
	}

	return 0; // OK
}


int serial_send(char *buffer, int n, HANDLE h)
{
	DWORD nsent; // number of bytes to be sent

	if (!WriteFile(h, (LPVOID)buffer, (DWORD)n, &nsent, NULL) ) {
		printf("\nserial port write error");
		return 1; // error
	}

	return 0; // OK
}


int serial_recv_char(char &ch, HANDLE h)
{
	// receive a buffer with only one character / byte
	return serial_recv(&ch, 1, h);
}


int serial_send_char(char ch, HANDLE h)
{
	// send a buffer with one character / byte
	return serial_send(&ch, 1, h);
}


int serial_available(HANDLE h)
// return the number of bytes available for reading from the serial port buffer
{
	COMSTAT status;
	DWORD errors;
	int nq; // number of bytes in serial port queue

	// get serial port status
	ClearCommError(h, &errors, &status);

	nq = status.cbInQue;

	return nq;
}


void show_serial(HANDLE h)
{
	char ch;
	while (serial_available(h) > 0)
	{
		serial_recv_char(ch, h);
		cout << ch;
		Sleep(1); // wait for more characters
	}

}
