#include "systemclass.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	SystemClass System;
	bool result;

	result = System.Initialize();
	if(result)
	{
		System.Run();
	}

	System.Shutdown();

	return 0;
}
