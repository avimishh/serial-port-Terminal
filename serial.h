#ifndef _SERIAL_H
#define _SERIAL_H
#pragma once
#include <windows.h>
#include <stdio.h>

HANDLE hSerial;

// serial port initialize
bool port_init() {
	DCB dcbSerialParams = { 0 };	//by setting 0 -> clear all fields
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(hSerial, &dcbSerialParams)) {
		//error getting state
		return false;
	}
	// set serial port settings
	dcbSerialParams.BaudRate = CBR_19200;
	dcbSerialParams.ByteSize = 8;		// size = 8 bit
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (!SetCommState(hSerial, &dcbSerialParams)) {
		//error setting port
		return false;
	}

	return true;
}

// set timeout for read operation => avoid from stack if nothing recieved yet
bool set_timeout() {
	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	if (!SetCommTimeouts(hSerial, &timeouts)) {
		//error occureed
		return false;
	}

	return true;
}

bool open(TCHAR* data) {
	hSerial = CreateFile(data,	// access to serial port
		GENERIC_READ | GENERIC_WRITE,	//read or write to the serial port
		0,	//always 0
		0,	//always 0
		OPEN_EXISTING,	//Windows should only open an existing file, and since serial ports already exist, this is what we want
		FILE_ATTRIBUTE_NORMAL,	//tells Windows we don’t want anything fancy here
		0);

	if (hSerial == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			//serial port does not exist
			return false;
		}
		return false; //some other error occurred
	}

	if (!port_init())		// in case failed to initalize port than inform error
		return false;
	if (!set_timeout())		// in case failed to set timeout for reading opearation than inform error
		return false;

	return true;
}



char* read() {
	char buf[10 + 1] = { 0 };
	DWORD dwBytesRead = 0;
	if (!ReadFile(hSerial, &buf, 10, &dwBytesRead, NULL)) {
		//error occurred unable to read from port
	}
	return buf;
}

bool write(TCHAR* data, unsigned int byteSize) {
	DWORD dwBytesRead = 0;
	if (!WriteFile(hSerial, data, byteSize, &dwBytesRead, NULL)) {
		//error occurred unable to write into port
		return false;
	}
	return true;
}

bool close() {
	CloseHandle(hSerial);
	return true;
}

#endif
