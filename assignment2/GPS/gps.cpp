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
	SMObject PMObj(_TEXT("PMObj"), sizeof(PM));
	PMObj.SMCreate();
	PMObj.SMAccess();
	PM* PMObjPtr = (PM*)PMObj.pData;

	int count = 0;

	//Loop
	while ((!_kbhit()) && (PMObjPtr->Shutdown.Status != 0xFF)) // figure out how to check for the laser bit only
	{
		Console::WriteLine("still running:{0}", count);
		// Check the heartbeat flag
		/*if (PMObjPtr->Heartbeats.Flags.GPS == 0) { // PM is still active
			// Set laser heartbeat
			PMObjPtr->Heartbeats.Flags.GPS = 1; // Let PM know Laser is still active
			count = 0;
		}
		else {
			count++; // time to PM between deactivates
			if (count > 100) {
				//PMObjPtr->Shutdown.Status = 0xFF; // Shutdown everything
				break;
			}
		}*/
		System::Threading::Thread::Sleep(50);
	}

	return 0;
}