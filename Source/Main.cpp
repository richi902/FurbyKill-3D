/*
	FurbyKill 3D
	============
	License: GPL
	Author: Andre Schnabel
	E-Mail: thefrogs@web.de
	Homepage: http://www.semler-service.de/andre/
*/

// Main.cpp

#ifdef GEKKO
#include <fat.h>
#endif

#include "Framework.h"
#include "Game.h"
#include "Splash.h"

bool ParseArgs(int argc, char **argv, bool *fullscreen, bool *customMap, char *mapName, bool *lowQuality, bool *noSound, bool *badQuality);

//
// main
//
int main(int argc, char **argv)
{
	char mapName[MAX_STR_LENGTH];
	bool customMap = false, lowQuality = false, badQuality = false;
	
	FrameworkSetup setup;
	Game *game;

#ifdef GEKKO
	fatInitDefault();
#endif

	strcpy(setup.caption, "FurbyKill3D");
	
	if(ParseArgs(argc, argv, &setup.fullscreen, &customMap, mapName, &lowQuality, &setup.noSound, &badQuality))
		return 0;
	
	if(lowQuality)
	{
		setup.scrW = 320;
		setup.scrH = 240;
		setup.smallFontSize /= 2;
		setup.bigFontSize /= 2;
	}
	else
	{
		setup.scrW = 640;
		setup.scrH = 480;
	}
	
	
	if(customMap)
		game = new Game(mapName, lowQuality);
		
	else
	{
		if(lowQuality)
			game = new Game(NULL, true, badQuality);
		else
			game = new Game();
	}
	
	new Framework(&setup);
	framework->AddState(new Splash);
	framework->AddState(game);
	framework->Run();
	
	delete framework;
	
	return 0;
}

//
// ParseArgs
//
bool ParseArgs(int argc, char **argv, bool *fullscreen, bool *customMap, char *mapName, bool *lowQuality, bool *noSound, bool *badQuality)
{
	int i;
	
	for(i=1; i<argc; i++)
	{
		if(!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
		{
			printf("FurbyKill3D - Help\n"
					"Possible options:\n"
					"-h or --help: Shows this message\n"
					"--usage: Small usage information\n"
					"-f or --fullscreen: Play it in fullscreen\n"
					"-m mapname: Play a specified map\n"
					"-l or --low: Low quality mode for slow machines\n"
					"-n or --nosound: Disable sound\n"
					"-b or --bad: Bad quality mode for very slow machines\n");
			return true;
		}
		else if(!strcmp(argv[i], "--usage"))
		{
			printf("Usage:\n"
					"furbykill3d [--fullscreen] [--help] [--usage] [-m mapname]\n");
			return true;
		}
		else if(!strcmp(argv[i], "--fullscreen") || !strcmp(argv[i], "-f"))
			*fullscreen = true;
		else if(!strcmp(argv[i], "-m"))
		{
			*customMap = true;
			strcpy(mapName, argv[i+1]);
		}
		else if(!strcmp(argv[i], "-l") || !strcmp(argv[i], "--low"))
			*lowQuality = true;
		else if(!strcmp(argv[i], "-n") || !strcmp(argv[i], "--nosound"))
			*noSound = true;
		else if(!strcmp(argv[i], "-b") || !strcmp(argv[i], "--bad"))
		{
			*lowQuality = true;
			*badQuality = true;
		}
	}
	
	return false;
}
