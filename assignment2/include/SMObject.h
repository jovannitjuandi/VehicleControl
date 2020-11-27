#ifndef SMOBJECT_H
#define SMOBJECT_H
#include <Windows.h>
#include <tchar.h>
#include <string>

//#ifndef UNICODE  
//typedef std::string String;
//#else
//typedef std::wstring String;
//#endif

class SMObject
{
	HANDLE CreateHandle;
	HANDLE AccessHandle;
	TCHAR *szName;
	int Size;
public:
	void *pData;
	bool SMCreateError;
	bool SMAccessError;
public:
	SMObject();
	SMObject(TCHAR* szname, int size);

	~SMObject();
	int SMCreate();
	int SMAccess();
	void SetSzname(TCHAR* szname);
	void SetSize(int size);
};

struct Laser {
	double X;
	double Y;
};

struct GPS {
	double Northing;
	double Easting;
	double Height;
	unsigned int CRC;
};

struct Disp {
	double Speed;
	double Steer;
};

struct ModuleFlags {
	unsigned char PM : 1,
		GPS : 1,
		Laser : 1,
		Camera : 1,
		Vehicle : 1,
		Opengl : 1,
		Unused : 2;
};

union ExecFlags {
	unsigned char Status;
	ModuleFlags Flags;
};

struct PM {
	ExecFlags Heartbeats;
	ExecFlags Shutdown;
};
#endif


