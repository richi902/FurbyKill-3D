/*
	FurbyKill 3D
	============
	License: GPL
	Author: Andre Schnabel
	E-Mail: thefrogs@web.de
	Homepage: http://www.semler-service.de/andre/
*/

// Raycaster.h

#ifndef RAYCASTER_H
#define RAYCASTER_H


#ifdef _XBOX
#include "../furbyxbox/x_Globals.h"
#elif GEKKO
#include "../furbywii/wii_Globals.h"
#elif _WIN32
#include "../furbywin/w32_Globals.h"
#elif linux
#include "../furbylinux/lin32_Globals.h"
#endif

#include "Framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define FOV				60

#define UP(alpha)		(alpha > 0 && alpha < 180)
#define DOWN(alpha)		(alpha >= 180 && alpha <= 360)
#define LEFT(alpha)		(alpha > 90 && alpha < 270)
#define RIGHT(alpha)	(alpha >= 270 && alpha <= 360 \
						|| alpha >= 0 && alpha <= 90)

#define ARC2RAD(alpha)	alpha * M_PI / 180
#define RAD2ARC(radian) radian * 180 / M_PI
#define SIN(alpha)		sin(ARC2RAD(alpha))
#define COS(alpha)		cos(ARC2RAD(alpha))
#define TAN(alpha)		tan(ARC2RAD(alpha))

#define CORR_ANGLE(a)	{ if(a < 0.0f) \
							a += 360.0f; \
						else if(a > 360.0f) \
							a -= 360.0f; }

#define ABS(x)			x > 0 ? x : -x

#define WALL_HEIGHT		1
#define NUM_COLORS		5

#define	TEXTURE_W		128
#define TEXTURE_H		128

#define SPRITE_WIDTH	64
#define SPRITE_HEIGHT	64

#define MAX_MAP_W		100
#define MAX_MAP_H		100

#define MIN_WALL_DIST	0.35f

#define EXIT_DOOR_INDEX 10
#define DOOR_CHG_TIME	300

#define SPR_NEAR_CLIP_DIST	0.49f

struct Vec2
{
	float x, y;

	void Rotate(float angle)
	{
		static float temp;
		temp = x;
		x = x * COS(-angle) - y * SIN(-angle);
		y = temp * SIN(-angle) + y * COS(-angle);
	}
};

class Sprite;

struct RaycasterSetup
{
	char mapFilename[MAX_STR_LENGTH];

	int numTextures;
	SDL_Surface *textures[MAX_TEXTURES][2];

	list<Sprite *> *sprites;

	SDL_Surface *target;

	bool lowQuality, badQuality;

	SDL_Rect *crosshairRect;
};

class Door;

struct SprDraw
{
	SDL_Surface *image;
	float x, y;
	float distance;
	Sprite *original;

	bool operator <(const SprDraw &other) const
	{
		return (distance > other.distance);
	}
};

class Raycaster
{
	public:
		Raycaster(RaycasterSetup *setup);
		~Raycaster();

		void ClearMap();
		void LoadMap(const char *filename);

		void Draw();

		void SetPosX(float x);
		void SetPosY(float y);

		float GetPosX() const;
		float GetPosY() const;

		void SetPosition(float x, float y);
		void GetPosition(float *x, float *y) const;

		// Alpha stands for an angle, not transparency!
		void SetAlpha(float alpha);
		float GetAlpha() const;

		void Rotate(float angle);
		void MoveForward(float distance, bool *wasPickup, int *pickupId);
		void MoveBackward(float distance, bool *wasPickup, int *pickupId);
		void StepRight(float distance, bool *wasPickup, int *pickupId);
		void StepLeft(float distance, bool *wasPickup, int *pickupId);

		void OpenDoor(bool hasKey, bool *needAKey);

		int GetMapValueAt(int x, int y);

		bool MapChangeNeeded();

		void SetMapValueAt(int x, int y, int value);

	private:
		enum IntersectionDir {
			ID_HORIZONTAL,
			ID_VERTICAL
		};

		void UpdateDoors();
		void DrawSprites();

		float CastThroughIntersections(float angle, IntersectionDir dir, int *texIndex, float *texelX);
		float CorrDistance(float distance, float angle);
		int DistToWallHeight(float distance);
		void DrawWallSlice(int x, int height, int texNum, float texelX, bool dark);

		Door *GetDoorAt(int x, int y);
		bool IsOpenDoor();

		bool CheckCollWithSprite(bool *wasPickup, int *pickupId);

		float XCoordToAngle(int x);
		int AngleToXCoord(float angle);

		float pplaneDist;

		Uint32 floorColor, roofColor;
		SDL_Rect upperRect, lowerRect;
		SDL_Surface *skyImg, *roofImg;

		SDL_Surface *target;

		int map[MAX_MAP_H][MAX_MAP_W];
		int mapW, mapH;

		SDL_Surface *textures[MAX_TEXTURES][2];
		int numTextures;

		float posX, posY;
		float alpha;

		Vec2 lookDir, camPlane;

		bool texturesLoaded;

		float *zBuffer;

		list<Sprite *> *sprites;
		list<SprDraw> sprsToDraw;
		list<Door> doors;

		bool lowQuality, badQuality;
		bool mapChange;

		SDL_Rect *crosshairRect;
};

#endif
