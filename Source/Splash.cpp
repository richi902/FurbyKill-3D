/*
	FurbyKill 3D
	============
	License: GPL
	Author: Andre Schnabel
	E-Mail: thefrogs@web.de
	Homepage: http://www.semler-service.de/andre/
*/

// Splash.cpp

#include "Framework.h"
#include "Splash.h"

#ifdef _XBOX
#include "../furbyxbox/x_Globals.h"
#elif GEKKO
#include "../furbywii/wii_Globals.h"
#elif _WIN32
#include "../furbywin/w32_Globals.h"
#elif linux
#include "../furbylinux/lin32_Globals.h"
#endif
//
// Init
//
void Splash::Init()
{

	if(framework->GetScrW() >= 640)
	{
#ifdef GEKKO
		sdlImg = framework->LoadImage("images/sdl.bmp");
		splashImg = framework->LoadImage("images/splash.bmp");
#elif _WIN32
		sdlImg = framework->LoadImage("images\\sdl.jpg");
		splashImg = framework->LoadImage("images\\splash.bmp");
#elif _XBOX
        sdlImg = framework->LoadImage("images\\sdl.jpg");
        splashImg = framework->LoadImage("images\\splash.bmp");
#elif _linux_
        sdlImg = framework->LoadImage("images/sdl.jpg");
        splashImg = framework->LoadImage("images/splash.bmp");
#endif
	}

	else
		splashImg = framework->LoadImage("images\\lowQuality\\splash.bmp");

#ifdef GEKKO
	splashSnd = framework->LoadSound("sounds/introsound.ogg");
#elif _WIN32
	splashSnd = framework->LoadSound("sounds\\introsound.ogg");
#elif _XBOX
    splashSnd = framework->LoadSound("sounds\\introsound.ogg");
#elif _linux_
    splashSnd = framework->LoadSound("sounds/introsound.ogg");
#endif

	alpha = SDL_ALPHA_TRANSPARENT;
	black = Colors::black.ToUint32();
	aboutSurf = framework->DrawTextToSurface("2006 by Andre Schnabel", Colors::white, false);
}

//
// Quit
//
void Splash::Quit()
{
	if(!framework->GetNoSound())
	{
		Mix_HaltChannel(-1);
		Mix_FreeChunk(splashSnd);
	}

	SDL_FreeSurface(aboutSurf);
	SDL_FreeSurface(splashImg);
	SDL_FreeSurface(sdlImg);
}

//
// Input
//
void Splash::Input()
{
	if(framework->KeyPressed(SDLK_ESCAPE) || framework->KeyPressed(SDLK_RETURN)||framework->GetJButton(0, JOY_BTTN_START))
		SetDone(true);
}

//
// Update
//
void Splash::Update()
{
	if(startY < 0)
	{
		alpha ++;
		startY += 2;

		if(startY >= 0)
		{
			alpha = SDL_ALPHA_OPAQUE;
			startY = 0;
		}

		SDL_SetAlpha(splashImg, SDL_SRCALPHA, alpha);
	}
}

//
// Draw
//
void Splash::Draw()
{
	static bool Switch = false;
	int cur_time = 0;
	int diff_time = 0;
	int last_time = 0;
	int accumulator = 0;

	cur_time = SDL_GetTicks();
	diff_time = cur_time - last_time;
	accumulator += diff_time;

	framework->Draw(sdlImg, 0, 0);

	if(accumulator>3000)
	{
		accumulator -= 3000;
		framework->FillRect(NULL, black);

		if(Switch==false)
		{
			startY = -splashImg->h/2;
			framework->PlaySound(splashSnd);
			Switch = true;
		}
		framework->Draw(splashImg, 0, startY);
		framework->Draw(aboutSurf, framework->GetScrW() - aboutSurf->w - 2, framework->GetScrH() - aboutSurf->h);
	}
}
