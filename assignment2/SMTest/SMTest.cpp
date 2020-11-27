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
	PM* PMObjectPtr = (PM*)PMObj.pData;

	SMObject GPSSMObj(_TEXT("GPSSMObj"), sizeof(GPS));
	GPSSMObj.SMCreate();
	GPSSMObj.SMAccess();
	GPS* GPSSMObjPtr = (GPS*)GPSSMObj.pData;

	SMObject DispSMObj(_TEXT("DispSMObj"), sizeof(Disp));
	DispSMObj.SMCreate();
	DispSMObj.SMAccess();
	Disp* DispSMObjPtr = (Disp*)DispSMObj.pData;

	array<String^>^ ModuleList = gcnew array<String^>{"Laser.exe", "GPS.exe", "Camera.exe", "OpenGL.exe", "Vehicle.exe"};
	array<int>^ Critical = gcnew array<int>(ModuleList->Length) { 1, 0, 1, 1, 0 };
	array<Process^>^ ProcessList = gcnew array<Process^>(ModuleList->Length);

	for (int i = 0; i < ModuleList->Length; i++) {
		ProcessList[i] = gcnew Process();
		ProcessList[i]->StartInfo->FileName = ModuleList[i];
		ProcessList[i]->StartInfo->CreateNoWindow = true;
		ProcessList[i]->Start();
	}
	
	array<int>^ idleCount = gcnew array<int>(ModuleList->Length) { 0, 0, 0, 0, 0 };
	PMObjectPtr->Heartbeats.Status = 0x00;
	PMObjectPtr->Shutdown.Status = 0x00;

	while(!_kbhit()) {
		for (int i = 0; i < ModuleList->Length; i++) {
			if (ProcessList[i]->HasExited) {
				Console::WriteLine("Process" + ProcessList[i]->StartInfo->FileName + " has exited!");
				if (Critical[i] == 1) {
					//PMObjectPtr->Shutdown.Status = 0xFF;
					//return 0;
				}
				else {
					ProcessList[i]->Start();
				}
			}
		}

		if (PMObjectPtr->Heartbeats.Flags.Laser == 1) {
			PMObjectPtr->Heartbeats.Flags.Laser = 0;
			idleCount[0] = 0;
		}
		else {
			idleCount[0] = idleCount[0] + 1;
		}

		if (PMObjectPtr->Heartbeats.Flags.GPS == 1) {
			PMObjectPtr->Heartbeats.Flags.GPS = 0;
			idleCount[1] = 0;
		}
		else {
			idleCount[1] = idleCount[1] + 1;
		}

		if (PMObjectPtr->Heartbeats.Flags.Camera == 1) {
			PMObjectPtr->Heartbeats.Flags.Camera = 0;
			idleCount[2] = 0;
		}
		else {
			idleCount[2] = idleCount[2] + 1;
		}

		if (PMObjectPtr->Heartbeats.Flags.Opengl == 1) {
			PMObjectPtr->Heartbeats.Flags.Opengl = 0;
			idleCount[3] = 0;
		}
		else {
			idleCount[3] = idleCount[3] + 1;
		}

		if (PMObjectPtr->Heartbeats.Flags.Vehicle == 1) {
			PMObjectPtr->Heartbeats.Flags.Vehicle = 0;
			idleCount[4] = 0;
		}
		else {
			idleCount[4] = idleCount[4] + 1;
		}

		for (int i = 0; i < ModuleList->Length; i++) {
			Console::WriteLine("IdleCount " + ProcessList[i]->StartInfo->FileName + ": {0}", idleCount[i]);
			
		}
		System::Threading::Thread::Sleep(70);
	}
	
	PMObjectPtr->Shutdown.Status = 0xFF;
	Console::ReadKey();
	return 0;
}