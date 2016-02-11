/*
	FurbyKill 3D
	============
	License: GPL
	Author: Andre Schnabel
	E-Mail: thefrogs@web.de
	Homepage: http://www.semler-service.de/andre/
*/

// Splash.h

#ifndef SPLASH_H
#define SPLASH_H

#include "State.h"

class Splash : public State
{
	public:
		virtual void Init();
		virtual void Quit();
			
		virtual void Input();
		virtual void Update();
		virtual void Draw();
		
	private:
		int startY, alpha;
		SDL_Surface *splashImg, *aboutSurf, *sdlImg;
		Mix_Chunk *splashSnd;
		Uint32 black;
		
};

#endif
