//Compile in a C++ CLR empty project
#using <System.dll>

#include <conio.h>//_kbhit()
#include <iostream>
#include <SMObject.h>

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

/*struct SMData {
	double SMFromGalil;
};*/

struct Laser {
	double X[361];
	double Y[361];
};

struct SMData {
	Laser laser;
};

struct ModuleFlags {
	unsigned char PM : 1,
		GPS : 1,
		Laser : 1,
		Camera : 1,
		Vehicle : 1,
		Unused : 3;
	// maybe add camera or display module
};

union ExecFlags {
	unsigned char Status;
	ModuleFlags Flags;
};

struct PM {
	ExecFlags Heartbeats;
	ExecFlags Shutdown;
};

int main()
{
	// SHARED MEMORY STUFF
	SMObject LaserSMObj(_TEXT("LaserSMObj"), sizeof(Laser));
	LaserSMObj.SMCreate();
	LaserSMObj.SMAccess();
	Laser* LaserSMObjPtr = (Laser*)LaserSMObj.pData;

	SMObject PMObj(_TEXT("PMObj"), sizeof(PM));
	PMObj.SMCreate();
	PMObj.SMAccess();
	PM* PMObjPtr = (PM*)PMObj.pData;


	// Galil PLC any port number is OK
	int PortNumber = 23000;
	// Pointer to TcpClent type object on managed heap
	TcpClient^ Client = nullptr;
	// arrays of unsigned chars to send and receive data
	array<unsigned char>^ SendData = nullptr;
	array<unsigned char>^ ReadData = nullptr;
	// String command to ask for Channel 1 analogue voltage from the PLC
	// These command are available on Galil RIO47122 command reference manual
	// available online
	String^ zID = gcnew String("5213864\n");
	String^ AskScan = gcnew String("sRN LMDscandata");
	// String to store received data for display
	String^ ResponseData;

	// Creat TcpClient object and connect to it
	Client = gcnew TcpClient("192.168.1.200", PortNumber);
	// Configure connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	// unsigned char arrays of 16 bytes each are created on managed heap
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);
	// Convert string command to an array of unsigned char
	SendData = System::Text::Encoding::ASCII->GetBytes(zID);


	// Get the network streab object associated with clien so we 
	// can use it to read and write
	NetworkStream^ Stream = Client->GetStream();

	// ask for data
	Stream->Write(SendData, 0, SendData->Length);
	System::Threading::Thread::Sleep(10);

	// read data
	Stream->Read(ReadData, 0, ReadData->Length);
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	ResponseData = ResponseData->Replace(":", "");

	SendData = System::Text::Encoding::ASCII->GetBytes(AskScan);
	array<wchar_t>^ Space = { ' ' };
	int count = 0;

	//Loop
	while (!_kbhit() && (PMObjPtr->Shutdown.Status != 0xFF)) // figure out how to check for the laser bit only
	{
		// Write command asking for data
		/*Stream->Write(SendData, 0, SendData->Length);
		// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
		System::Threading::Thread::Sleep(10);
		// Read the incoming data
		Stream->Read(ReadData, 0, ReadData->Length);
		// Convert incoming data from an array of unsigned char bytes to an ASCII string
		ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
		//Remove ":" from the string
		ResponseData = ResponseData->Replace(":", "");
		// Print the received string on the screen
		Console::WriteLine(ResponseData);*/
		// GalilSMObjPtr->SMFromGalil = 159.951;

		// Write command asking for data
		Stream->WriteByte(0x02);
		Stream->Write(SendData, 0, SendData->Length);
		Stream->WriteByte(0x03);
		// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
		System::Threading::Thread::Sleep(10);
		// Read the incoming data
		Stream->Read(ReadData, 0, ReadData->Length);
		// Convert incoming data from an array of unsigned char bytes to an ASCII string
		ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
		// Delete ":" from the string
		ResponseData = ResponseData->Replace(":", "");
		// Print the received string on the screen
		//Console::WriteLine(ResponseData);

		// ACTUAL Information
		array<String^>^ StringArray = ResponseData->Split(Space);
		double StartAngle = System::Convert::ToInt32(StringArray[23], 16);
		double Resolution = System::Convert::ToInt32(StringArray[24], 16) / 10000.0;
		int NumRanges = System::Convert::ToInt32(StringArray[25], 16);

		array<double>^ Range = gcnew array<double>(NumRanges);
		array<double>^ RangeX = gcnew array<double>(NumRanges);
		array<double>^ RangeY = gcnew array<double>(NumRanges);

		// Check the heartbeat flag
		if (PMObjPtr->Heartbeats.Flags.Laser == 0) { // PM is still active
			// Set laser heartbeat
			PMObjPtr->Heartbeats.Flags.Laser = 1; // Let PM know Laser is still active
			for (int i = 0; i < NumRanges; i++) {
				Range[i] = System::Convert::ToInt32(StringArray[26 + i], 16);
				RangeX[i] = Range[i] * sin(i * Resolution);
				RangeY[i] = -Range[i] * cos(i * Resolution);
				LaserSMObjPtr->X[i] = RangeX[i];
				LaserSMObjPtr->Y[i] = RangeY[i];
				Console::WriteLine("x:{0} y:{1}", RangeX[i], RangeY[i]);
			}
			count = 0;
		}
		else {
			count++; // time to PM between deactivates
			if (count > 100) {
				//PMObjPtr->Shutdown.Status = 0xFF; // Shutdown everything
			}
		}
		ResponseData = nullptr;
		System::Threading::Thread::Sleep(50);
	}

	Stream->Close();
	Client->Close();

	return 0;
}