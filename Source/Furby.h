/*
	FurbyKill 3D
	============
	License: GPL
	Author: Andre Schnabel
	E-Mail: thefrogs@web.de
	Homepage: http://www.semler-service.de/andre/
*/

// Furby.h

#ifndef FURBY_H
#define FURBY_H

#include "Sprite.h"
#include "Game.h"

#define FURBY_MOV_SPEED 		0.4f
#define FB_UPDATE_TIME			500
#define FURBY_RELOAD_TIME		2945
#define SKULL_RELOAD_TIME		2500
#define SHOT_INDEX				99
#define SK_SHOT_INDEX			100
#define SHOT_UPDATE_TIME		150
#define SHOT_MOV_SPEED			0.8f
#define NUM_FB_IMGS				5
#define NUM_SK_IMGS				8
#define NUM_DEATH_ANIM_FRAMES	3
#define DA_UPDATE_TIME			250


class Skull :public Sprite
{
	public:
		Skull(SDL_Surface *images[NUM_SK_IMGS], SDL_Surface *shotImg, float x, float y, list<Sprite *> *sprites);
		virtual void Update(Raycaster *caster);
		
		void SetVisible(bool visible);
		bool IsVisible();
		
		void NextImage();
		void SetCurImage(int curImg);
		int GetCurImage(void);
		
		bool IsDead();
		
	private:
		SDL_Surface *images[NUM_SK_IMGS];
		list<Sprite *> *sprites;
		SDL_Surface *shotImg;
		Uint32 lUpdate, lShot;
		bool visible, dead;
		int curImg;

};
class SkullShot : public Sprite
{
	public:
		SkullShot(SDL_Surface *image, float x, float y, float targetX, float targetY);
		virtual void Update(Raycaster *caster);
		
		bool IsVanished();
		
	private:
		float movAngle;
		Uint32 lUpdate;
		bool vanished;
};
class Furby : public Sprite
{
	public:
		Furby(SDL_Surface *images[NUM_FB_IMGS], SDL_Surface *shotImg, float x, float y, list<Sprite *> *sprites);
		virtual void Update(Raycaster *caster);
		
		void SetVisible(bool visible);
		bool IsVisible();
		
		void NextImage();
		void SetCurImage(int curImg);
		int GetCurImage(void);
		
		bool IsDead();
		
	private:
		SDL_Surface *images[NUM_FB_IMGS];
		list<Sprite *> *sprites;
		SDL_Surface *shotImg;
		Uint32 lUpdate, lShot;
		bool visible, dead;
		int curImg;
};

class Shot : public Sprite
{
	public:
		Shot(SDL_Surface *image, float x, float y, float targetX, float targetY);
		virtual void Update(Raycaster *caster);
		
		bool IsVanished();
		
	private:
		float movAngle;
		Uint32 lUpdate;
		bool vanished;
		
};

class DeathAnimation : public Sprite
{
	public:
		DeathAnimation(SDL_Surface *frames[NUM_DEATH_ANIM_FRAMES], float x, float y);
		virtual void Update();
		
		bool HasEnded();
		
	private:
		SDL_Surface *frames[NUM_DEATH_ANIM_FRAMES];
		int curFrame;
		bool ended;
		Uint32 lUpdate;
};
//Skull DeathAnim
class SKDeathAnimation : public Sprite
{
	public:
		SKDeathAnimation(SDL_Surface *frames[NUM_DEATH_ANIM_FRAMES], float x, float y);
		virtual void Update();

		bool HasEnded();

	private:
		SDL_Surface *frames[NUM_DEATH_ANIM_FRAMES];
		int curFrame;
		bool ended;
		Uint32 lUpdate;
};

#endif
