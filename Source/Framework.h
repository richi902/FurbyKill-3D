/*
	FurbyKill 3D
	============
	License: GPL
	Author: Andre Schnabel
	E-Mail: thefrogs@web.de
	Homepage: http://www.semler-service.de/andre/
*/

// Framework.h

#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#ifdef _linux_
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#else
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#endif

#include <stdlib.h>
#include <list>

#ifdef _XBOX
#include "../furbyxbox/x_Globals.h"
#elif GEKKO
#include "../furbywii/wii_Globals.h"
#elif _WIN32
#include "../furbywin/w32_Globals.h"
#elif linux
#include "../furbylinux/lin32_Globals.h"
#endif

#ifdef _XBOX
#include <xtl.h>
#endif

using namespace std;

#define MAX_STR_LENGTH	256
#define MAX_FPS			60


struct FrameworkSetup
{
	char caption[MAX_STR_LENGTH];

	int scrW, scrH;
	int scrBpp;
	bool fullscreen;

	bool noSound;

	int smallFontSize, bigFontSize;

	FrameworkSetup()
	{
		strcpy(caption, "Framework");
		scrW = 640;
		scrH = 480;
		scrBpp = 16;
		fullscreen = false;
		noSound = false;
		smallFontSize = 15;
		bigFontSize = 32;
	}
};

enum MouseButton
{
	BUTTON_LEFT = 1,
	BUTTON_MIDDLE,
	BUTTON_RIGHT
};

struct Color
{
	int r, g, b;

	Color(int r, int g, int b)
	{
		this->r = r;
		this->g = g;
		this->b = b;
	}

	Uint32 ToUint32();

	SDL_Color ToSDLCol()
	{
		SDL_Color col;
		col.r = r;
		col.g = g;
		col.b = b;
		return col;
	}
};

namespace Colors
{
	static Color black(0, 0, 0);
	static Color white(255, 255, 255);
	static Color red(255, 0, 0);
	static Color green(0, 255, 0);
	static Color blue(0, 0, 255);
	static Color yellow(255, 255, 0);
	static Color cyan(0, 255, 255);
	static Color magenta(255, 0, 255);
}

class State;

class Framework
{
	public:
		Framework(FrameworkSetup *setup = NULL);
		~Framework();

		void Run();

		void AddState(State *state);

		bool DrawPixel(int x, int y, Uint32 color);
		Uint32 GetPixelColor(SDL_Surface *surf, int x, int y) const;
		void Error(const char *msg);
		void Output(const char *msg);

		SDL_Surface *LoadImage(const char *filename, bool useAlpha = false);

		void LockScreen();
		void UnlockScreen();

		SDL_Surface *GetScreen();
		int GetScrW() const;
		int GetScrH() const;

		void FillRect(SDL_Rect *rect, Uint32 color);

		bool UpdateKeys();
		bool KeyPressed(int keyNum, bool wasReleased = true);
		int GetJButton(int Index,int Button);
		int JButtonState(int Index, int Button);
		float GetAxis(int Index,int Axis);
		float GetAxisAWPositive(int Index, int Axis);
		bool GetHatCentered(int Index);
		int GetHatWhich(int Index, int WhichHat);

		Uint32 GetColor(int r, int g, int b);
		Uint32 GetMagenta() const;

		void Draw(SDL_Surface *image, int x = 0, int y = 0);
		void Draw(SDL_Surface *image, SDL_Rect *srcRect, int x = 0, int y = 0);
		void DrawCentered(SDL_Surface *image);
		int ArmMovmeantH();
		int ArmMovmeantW();

		void DrawText(const char *text, int x, int y, Color color = Colors::white, bool useBigFont = false);
		void DrawTextCentered(const char *text, Color color = Colors::white, bool useBigFont = false);
		SDL_Surface *DrawTextToSurface(const char *text, Color color, bool useBigFont);

		int CountRowsInFile(const char *filename);
		int CountColsInFile(const char *filename);

		void GetMouseMovement(int *x, int *y);

		bool FileExists(const char *filename);

		void ShowFPS();
		void Debug(bool showdebug = true);
		float GetFrameTime();

		bool ButtonPressed(MouseButton button);

		Mix_Music *LoadMusic(const char *filename);
		Mix_Chunk *LoadSound(const char *filename);
		Mix_Chunk *EnemyShoot, *OpeningDoor, *ClosingDoor;

		bool Coll(SDL_Rect *rect1, SDL_Rect *rect2);

		int PlaySound(Mix_Chunk *sound, int loops = 0);
		void PlayMusic(Mix_Music *music, int loops = 0);

		bool GetNoSound() const;
		void GlobalSounds();

#ifdef _XBOX

		int xbox_SetScreenPosition(float x, float y);
		int xbox_SetScreenStretch(float xs, float ys);

#endif
		bool Exit;

	private:
		FrameworkSetup *setup;
		SDL_Surface *screen;
		list<State *> states;
		Uint8 *keys, *oldKeys;
		int numKeys;
		Uint32 magenta, black;
		bool allDone;
		bool ownSetup;
		TTF_Font *font, *bigFont;
		float frameTime;
		bool noSound;

};
extern Framework *framework;
#endif
