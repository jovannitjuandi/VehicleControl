//Compile in a C++ CLR empty project
#using <System.dll>

#include <conio.h>//_kbhit()
#include <iostream>
#include <SMObject.h>

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

int main()
{
	SMObject PMObj(_TEXT("PMObj"), sizeof(PM));
	PMObj.SMAccess();
	PM* PMObjPtr = (PM*)PMObj.pData;

	SMObject DispSMObj(_TEXT("DispSMObj"), sizeof(Disp));
	DispSMObj.SMAccess();
	Disp* DispSMObjPtr = (Disp*)DispSMObj.pData;

	// LMS151 port number must be 2111
	int PortNumber = 25000;
	// Pointer to TcpClent type object on managed heap
	TcpClient^ Client;
	// arrays of unsigned chars to send and receive data
	array<unsigned char>^ SendData = nullptr;
	array<unsigned char>^ ReadData = nullptr;

	// String command to ask for Channel 1 analogue voltage from the PLC
	// These command are available on Galil RIO47122 command reference manual
	// available online
	String^ zID = gcnew String("5213864\n");
	// String to store received data for display
	String^ ResponseData = nullptr;

	// Create TcpClient object and connect to it
	Client = gcnew TcpClient("192.168.1.200", PortNumber);
	// Configure connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	// unsigned char arrays of 16 bytes each are created on managed heap
	SendData = gcnew array<unsigned char>(1024);
	ReadData = gcnew array<unsigned char>(2500);
	// Convert string command to an array of unsigned char
	SendData = System::Text::Encoding::ASCII->GetBytes(zID);

	// Get the network stream object associated with client so we 
	// can use it to read and write
	NetworkStream^ Stream = Client->GetStream();

	// Write command asking for data
	Stream->Write(SendData, 0, SendData->Length);
	// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
	System::Threading::Thread::Sleep(10);
	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);
	// Convert incoming data from an array of unsigned char bytes to an ASCII string
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	// Delete ":" from the string
	ResponseData = ResponseData->Replace(":", "");

	int Flag = 0;
	String^ SendString = gcnew String("# ");
	int counter = 0;

	//Loop
	while ((!_kbhit()) && PMObjPtr->Shutdown.Flags.Vehicle != 1) // figure out how to check for the laser bit only
	{
		if (PMObjPtr->Heartbeats.Flags.Vehicle == 0) {
			PMObjPtr->Heartbeats.Flags.Vehicle = 1;
			counter = 0;
		}
		else {
			counter++;
			if (counter > 100) {
				PMObjPtr->Shutdown.Status = 0xFF;
			}
		}
		SendString = SendString + DispSMObjPtr->Steer.ToString("F3")
			+ " " + DispSMObjPtr->Speed.ToString("F3") + " " + Flag + " #";
		SendData = System::Text::Encoding::ASCII->GetBytes(SendString);
		Stream->Write(SendData, 0, SendData->Length);
		Console::WriteLine(SendString);

		if (Flag) {
			Flag = 0;
		}
		else {
			Flag = 1;
		}
		SendString = "# ";
		System::Threading::Thread::Sleep(100);
	}

	return 0;
}