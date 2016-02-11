/*
	FurbyKill 3D
	============
	License: GPL
	Author: Andre Schnabel
	E-Mail: thefrogs@web.de
	Homepage: http://www.semler-service.de/andre/
*/

// Game.h

#ifndef GAME_H
#define GAME_H

#ifdef _XBOX
#include "../furbyxbox/x_Globals.h"
#elif GEKKO
#include "../furbywii/wii_Globals.h"
#elif _WIN32
#include "../furbywin/w32_Globals.h"
#elif linux
#include "../furbylinux/lin32_Globals.h"
#endif

#include "State.h"
#include "Framework.h"
#include "Sprite.h"

#define ROT_SPEED		60.0f
//#define RELOAD_TIME		500
#define FIRE_SHOW_TIME	250

#define MSG_SHOW_TIME	2000

#define GAME_OVER_TIME	3000

enum EnemyTypes
{
	ET_SKULL = ENEMY_INDEX,
	ET_FURBY
};

enum PickupTypes
{
	PT_MEDPACK = PICKUP_INDEX,
	PT_AMMO,
	PT_KEY,
	PT_MACHINEGUN,
	PT_MGAMMO
};

struct TextMessage
{
	Uint32 sTime;
	char text[MAX_STR_LENGTH];
};

class Raycaster;
class Player;

class Game : public State
{
	public:
		Game(const char *mapFilename = NULL, bool lowQuality = false, bool badQuality = false);
		virtual ~Game() {}

		virtual void Init();
		virtual void Quit();

		virtual void Input();
		virtual void Update();
		virtual void Draw();


	private:
		void LoadResources();
		void FreeResources();
		void LoadSpriteMap(const char *filename);
		void DrawHUD();
		void Shoot();

		void NextMap();

		void UpdateMessages();
		void DrawMessages();
		void AddMessage(const char *str);

		void GameOver();

		SDL_Surface *crosshairImg, *hudBgImg, *gunHudImg, *gunImg, *fireImg, *keyImg, *MGgunImg, *MGfireImg, *MGgunHudImg;
		SDL_Surface *headImg, *headRedImg, *Loading1;

		Raycaster *raycaster;

		SDL_Surface *textures[MAX_TEXTURES][2], *spriteImgs[MAX_SPRITES];
		int numTextures;

		list<Sprite *> sprites;

		char mapFilename[MAX_STR_LENGTH];

		Player *player;

		bool lowQuality, badQuality;


		Uint32 fireStart;
		bool fireVisible;

		Mix_Music *music1, *music2, *music3, *music4, *music5;
		Mix_Chunk *startSnd, *shootSnd, *killSnd, *pickAmmoSnd, *EnemyHurt, *PlayerDead, *PlayerHurt, *pickKey, *pickHealth, *LVLClear, *HeartBeat, *MGshootSnd;



		int curMapNr;

		list<TextMessage> msgs;

		SDL_Rect crosshairRect;

		bool showExitConfirm;

		int RELOAD_TIME;
};

#endif
