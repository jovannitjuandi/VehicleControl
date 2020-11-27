//Compile in a C++ CLR empty project
#using <System.dll>

#include <conio.h>//_kbhit()
#include <iostream>
#include <SMObject.h>

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

#define CRC32_POLYNOMIAL 0xEDB88320L

#pragma pack(push,4)
struct GNSS {
	unsigned int Header;
	unsigned char Discards1[40];
	double Northing;
	double Easting;
	double Height;
	unsigned char Discards2[40];
	unsigned int CRC;
};
#pragma pack(pop,4)

// Functions for calculating CRC
unsigned long CRC32Value(int i);
unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char* ucBuffer);

int main()
{
	SMObject PMObj(_TEXT("PMObj"), sizeof(PM));
	PMObj.SMAccess();
	PM* PMObjPtr = (PM*)PMObj.pData;

	SMObject GPSSMObj(_TEXT("GPSSMObj"), sizeof(GPS));
	GPSSMObj.SMAccess();
	GPS* GPSSMObjPtr = (GPS*)GPSSMObj.pData;

	TcpClient^ Client;
	int PortNumber = 24000;

	//SerialPort^ Port = nullptr;
	//String^ PortName = nullptr;
	// arrays of unsigned chars to send and receive data
	array<unsigned char>^ SendData = nullptr;
	array<unsigned char>^ ReadData = nullptr;

	// Create TcpClient object and connect to it
	Client = gcnew TcpClient("192.168.1.200", PortNumber);
	// Configure connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	// unsigned char arrays of 16 bytes each are created on managed heap
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(5000);

	// Get the network stream object associated with client so we 
	// can use it to read and write
	NetworkStream^ Stream = Client->GetStream();

	System::Threading::Thread::Sleep(100);

	GNSS NovatelGPS;
	unsigned char* BytePtr = (unsigned char*)&NovatelGPS;
	unsigned char* TempPtr = BytePtr;

	unsigned int Header = 0, Data = 0;
	int j = 0;
	int Start; // Start of the data
	//GPSSMObjPtr->Counter = 0;
	int count = 0;
	unsigned int CRCCalculated;

	while (!_kbhit() && PMObjPtr->Shutdown.Flags.GPS != 1) {
		if (PMObjPtr->Heartbeats.Flags.GPS == 0) {
			PMObjPtr->Heartbeats.Flags.GPS = 1;
			count = 0;
		}
		else {
			count++;
			if (count > 1000000) {
				PMObjPtr->Shutdown.Status = 0xFF;
			}
		}
		if (Stream->DataAvailable) {
			Stream->Read(ReadData, 0, ReadData->Length);
			j = 0;
			do {
				Data = ReadData[j++];
				Header = ((Header << 8) | Data);
			} while (Header != 0xaa44121c);
			Start = j - 4;

			for (int i = Start; i < Start + sizeof(GNSS); i++) {
				*(BytePtr++) = ReadData[i];
			}
			CRCCalculated = CalculateBlockCRC32(108, TempPtr);
			Console::WriteLine("Northing:{0}, Easting:{1}, Height:{2}, CRC:{3}, CRC Calculated: {4}", NovatelGPS.Northing, NovatelGPS.Easting, NovatelGPS.Height, NovatelGPS.CRC, CRCCalculated);
			GPSSMObjPtr->Easting = NovatelGPS.Easting;
			GPSSMObjPtr->Northing = NovatelGPS.Northing;
			GPSSMObjPtr->Height = NovatelGPS.Height;
			GPSSMObjPtr->CRC = NovatelGPS.CRC;
			BytePtr = TempPtr;
		}
	}
	return 0;
}

unsigned long CRC32Value(int i)
{
	int j;
	unsigned long ulCRC;
	ulCRC = i;
	for (j = 8; j > 0; j--)
	{
		if (ulCRC & 1)
			ulCRC = (ulCRC >> 1) ^ CRC32_POLYNOMIAL;
		else
			ulCRC >>= 1;
	}
	return ulCRC;
}

unsigned long CalculateBlockCRC32(unsigned long ulCount, /* Number of bytes in the data block */
	unsigned char* ucBuffer) /* Data block */
{
	unsigned long ulTemp1;
	unsigned long ulTemp2;
	unsigned long ulCRC = 0;
	while (ulCount-- != 0)
	{
		ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
		ulTemp2 = CRC32Value(((int)ulCRC ^ *ucBuffer++) & 0xff);
		ulCRC = ulTemp1 ^ ulTemp2;
	}
	return(ulCRC);

	return 0;
}