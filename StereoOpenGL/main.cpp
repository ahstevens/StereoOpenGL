#include "Engine.h"
#include <conio.h>
#include <cstdio> // fclose
#include <string>


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	Engine *pMainApplication = new Engine(argc, argv, 0);

	if (!pMainApplication->init())
	{
		pMainApplication->Shutdown();
		delete pMainApplication;
		pMainApplication = NULL;
		return 1;
	}

	pMainApplication->RunMainLoop();

	pMainApplication->Shutdown();

	return 0;
}
