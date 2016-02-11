/*
	FurbyKill 3D
	============
	License: GPL
	Author: Andre Schnabel
	E-Mail: thefrogs@web.de
	Homepage: http://www.semler-service.de/andre/
*/

// Sprite.h

#ifndef SPRITE_H
#define SPRITE_H

#include "Framework.h"

#define SPR_COLL_BOX_W 1.0f
#define SPR_COLL_BOX_H 1.0f

class Sprite
{
	public:
		Sprite(SDL_Surface *image = NULL, float x = 0, float y = 0, int id = 0);
		virtual ~Sprite() {}
		
		void SetPos(float x, float y);
		void SetPosX(float x);
		void SetPosY(float y);
		
		float GetPosX() const;
		float GetPosY() const;
		void GetPos(float *x, float *y);
		
		void SetImage(SDL_Surface *image);
		SDL_Surface *GetImage();
		
		void SetId(int id);
		int GetId();
		
		bool PointCollWith(float itsX, float itsY);
		
		void SetLookingAt(bool lookingAt);
		bool GetLookingAt();
		
		virtual void Update() {}
		
	protected:
		float x, y;
		
	private:
		int id;		
		SDL_Surface *image;
		bool lookingAt;
};

#endif
