/*
	FurbyKill 3D
	============
	License: GPL
	Author: Andre Schnabel
	E-Mail: thefrogs@web.de
	Homepage: http://www.semler-service.de/andre/
*/

// Sprite.cpp

#include "Sprite.h"

//
// Sprite
// Constructor
//
Sprite::Sprite(SDL_Surface *image, float x, float y, int id)
{
	this->image = image;
	this->x = x;
	this->y = y;
	this->id = id;
	lookingAt = false;
}

//
// SetPos
//
void Sprite::SetPos(float x, float y)
{
	this->x = x;
	this->y = y;
}

//
// SetPosX
//
void Sprite::SetPosX(float x)
{
	this->x = x;
}

//
// SetPosY
// 
void Sprite::SetPosY(float y)
{
	this->y = y;
}

//
// GetPosX
//
float Sprite::GetPosX() const
{
	return x;
}

//
// GetPosY
//
float Sprite::GetPosY() const
{
	return y;
}

//
// GetPos
//
void Sprite::GetPos(float *x, float *y)
{
	*x = this->x;
	*y = this->y;
}

//
// SetImage
//
void Sprite::SetImage(SDL_Surface *image)
{
	this->image = image;
}

//
// GetImage
//
SDL_Surface *Sprite::GetImage()
{
	return image;
}

//
// SetId
//
void Sprite::SetId(int id)
{
	this->id = id;
}

//
// GetId
//
int Sprite::GetId()
{
	return id;
}

//
// PointCollWith
//
bool Sprite::PointCollWith(float itsX, float itsY)
{
	static float boxStartX, boxStartY, boxEndX, boxEndY;
	
	boxStartX = x - (SPR_COLL_BOX_W / 2.0f);
	boxEndX = x + (SPR_COLL_BOX_W / 2.0f);
	
	boxStartY = y - (SPR_COLL_BOX_H / 2.0f);
	boxEndY = y + (SPR_COLL_BOX_H / 2.0f);
	
	return (itsX >= boxStartX
			&& itsX <= boxEndX
			&& itsY >= boxStartY
			&& itsY <= boxEndY);
}

//
// SetLookingAt
//
void Sprite::SetLookingAt(bool lookingAt)
{
	this->lookingAt = lookingAt;
}

//
// GetLookingAt
//
bool Sprite::GetLookingAt()
{
	return lookingAt;
}
