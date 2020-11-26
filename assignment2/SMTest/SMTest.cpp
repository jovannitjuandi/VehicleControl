//Compile in a C++ CLR empty project
#using <System.dll>

#include <conio.h>//_kbhit()
#include <iostream>
#include <SMObject.h>

using namespace System;
using namespace System::Diagnostics;
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
	/*SMObject GalilSMObj(_TEXT("GalilSMObj"), sizeof(SMData));
	GalilSMObj.SMCreate();
	GalilSMObj.SMAccess();
	SMData* GalilSMObjPtr = (SMData*)GalilSMObj.pData;*/

	SMObject LaserSMObj(_TEXT("LaserSMObj"), sizeof(Laser));
	LaserSMObj.SMCreate();
	LaserSMObj.SMAccess();
	Laser* LaserSMObjPtr = (Laser*)LaserSMObj.pData;

	SMObject PMObj(_TEXT("PMObj"), sizeof(PM));
	PMObj.SMCreate();
	PMObj.SMAccess();
	PM* PMObjectPtr = (PM*)PMObj.pData;

	array<String^>^ ModuleList = gcnew array<String^>{"Laser.exe", "Vehicle.exe", "GPS.exe", "Camera.exe"};
	array<int>^ Critical = gcnew array<int>(ModuleList->Length) { 1, 1, 1, 1 };
	array<Process^>^ ProcessList = gcnew array<Process^>(ModuleList->Length);

	for (int i = 0; i < ModuleList->Length; i++) {
		ProcessList[i] = gcnew Process();
		ProcessList[i]->StartInfo->FileName = ModuleList[i];
		ProcessList[i]->StartInfo->CreateNoWindow = true;
		ProcessList[i]->Start();
	}
	int Laser = 0;
	int Vehicle = 0;
	int GPS = 0;
	int Camera = 0;
	int PM = 0;

	int count = 0;
	int LaserCritical = 1;

	while(!_kbhit()) {
		//Console::WriteLine("{0,12:F3}", GalilSMObjPtr->SMFromGalil);
		// check if program has quit

		/*for (int i = 0; i < ModuleList->Length; i++) {
			if (ProcessList[i]->HasExited) {
				Console::WriteLine("Process" + ProcessList[i]->StartInfo->FileName + " has exited!");
			}
		}*/

		// Check laser heartbeat
		/*if (PMObjectPtr->Shutdown.Status != 0xFF) {
			if (PMObjectPtr->Heartbeats.Flags.Laser == 1) {
				PMObjectPtr->Heartbeats.Flags.Laser = 0;
				Laser = 0;
			}
			else {
				Laser = Laser + 1;
				if (Laser > 100) {
					ProcessList[0]->CloseMainWindow();
					ProcessList[0]->WaitForExit();
				}
			}

			if (PMObjectPtr->Heartbeats.Flags.Vehicle == 1) {
				PMObjectPtr->Heartbeats.Flags.Vehicle = 0;
				Vehicle = 0;
			}
			else {
				Vehicle = Vehicle + 1;
				if (Vehicle > 100) {
					ProcessList[1]->CloseMainWindow();
					ProcessList[1]->WaitForExit();
				}
			}

			if (PMObjectPtr->Heartbeats.Flags.GPS == 1) {
				PMObjectPtr->Heartbeats.Flags.GPS = 0;
				GPS = 0;
			}
			else {
				GPS = GPS + 1;
				if (GPS > 100) {
					ProcessList[2]->CloseMainWindow();
					ProcessList[2]->WaitForExit();
				}
			}
		}
		else {
			for (int i = 0; i < ModuleList->Length; i++) {
				ProcessList[i]->CloseMainWindow();
				ProcessList[i]->WaitForExit();
			}
			//PMObjectPtr->Shutdown.Status = 0xFF;
			break;
		}*/

		Console::WriteLine("Laser: {0}", Laser);
		Console::WriteLine("GPS: {0}", GPS);
		Console::WriteLine("Vehicle: {0}", Vehicle);
		Console::WriteLine("Camera: {0}", Camera);

		if (ProcessList[0]->HasExited) {
			Console::WriteLine("Process" + ProcessList[0]->StartInfo->FileName + " has exited!");
		}
		else if (PMObjectPtr->Shutdown.Status != 0xFF) {
			if (PMObjectPtr->Heartbeats.Flags.Laser == 1) {
				PMObjectPtr->Heartbeats.Flags.Laser = 0;
				count = 0;
			}
			else {
				count = count + 1;
				if (count > 100) {
					// Shutdown everything
					if (LaserCritical == 1) {
						ProcessList[0]->CloseMainWindow();
						ProcessList[0]->WaitForExit();
					}
					// Restart the process
					else {
						// Then kill the process
						ProcessList[0]->Kill(); // If the process is not running (not responsive), if not use shutdown flag (heartbeat)
						// Then restart the process
						ProcessList[0]->Start();
					}
				}
			}
		}
		else {
			break;
		}
		System::Threading::Thread::Sleep(70);
	}
	
	PMObjectPtr->Shutdown.Status = 0xFF;
	Console::ReadKey();
	return 0;
}