/*
	FurbyKill 3D
	============
	License: GPL
	Author: Andre Schnabel
	E-Mail: thefrogs@web.de
	Homepage: http://www.semler-service.de/andre/
*/

// Furby.cpp

#include "Furby.h"
#include "Raycaster.h"
#include "Game.h"

//
// Furby
// Constructor
//
Furby::Furby(SDL_Surface *images[NUM_FB_IMGS], SDL_Surface *shotImg, float x, float y, list<Sprite *> *sprites)
{
	int i;
	
	dead = false;
	curImg = 0;
	
	for(i=0; i<NUM_FB_IMGS; i++)
		this->images[i] = images[i];
	
	SetImage(images[curImg]);
	SetPos(x, y);
	SetId(ET_FURBY);
	SetLookingAt(false);
	lUpdate = lShot = SDL_GetTicks();
	visible = false;
	this->sprites = sprites;
	this->shotImg = shotImg;
}

//
// Update
//
void Furby::Update(Raycaster *caster)
{	
	static float playerX, playerY;
	
	if(SDL_GetTicks() - lUpdate < FB_UPDATE_TIME || !visible)
		return;
	
	playerX = caster->GetPosX();
	playerY = caster->GetPosY();
	
	if(SDL_GetTicks() - lShot > FURBY_RELOAD_TIME)
	{
		Shot *newShot;

		newShot = new Shot(shotImg, x, y, playerX, playerY);		
		sprites->push_back(newShot);
		
		lShot = SDL_GetTicks();
	}
	

	if(ABS((playerX - x)) > 1.0f)
	{
		if(x < playerX)
		{
			x += FURBY_MOV_SPEED;
			
			if(caster->GetMapValueAt((int)x, (int)y) > 0)
				x -= FURBY_MOV_SPEED;
		}
		else
		{
			x -= FURBY_MOV_SPEED;
			
			if(caster->GetMapValueAt((int)x, (int)y) > 0)
				x += FURBY_MOV_SPEED;
		}
	}
	
	if(ABS((playerY - y)) > 1.0f)
	{
		if(y < playerY)
		{
			y += FURBY_MOV_SPEED;
			
			if(caster->GetMapValueAt((int)x, (int)y) > 0)
				y -= FURBY_MOV_SPEED;
		}
		else
		{
			y -= FURBY_MOV_SPEED;
			
			if(caster->GetMapValueAt((int)x, (int)y) > 0)
				y += FURBY_MOV_SPEED;
		}
	}
	
	lUpdate = SDL_GetTicks();
}

//
// SetVisible
// 
void Furby::SetVisible(bool visible)
{
	this->visible = visible;
}

//
// IsVisible
//
bool Furby::IsVisible()
{
	return visible;
}

//
// NextImage
//
void Furby::NextImage()
{
	curImg++;
	SetCurImage(curImg);
	
	if(curImg + 1 >= NUM_FB_IMGS)
		dead = true;
}

//
// SetCurImage
//
void Furby::SetCurImage(int curImg)
{
	if(curImg >= NUM_FB_IMGS)
		return;
	
	this->curImg = curImg;
	SetImage(images[curImg]);
}

//
// GetCurImage
//
int Furby::GetCurImage(void)
{
	return curImg;
}

//
// IsDead
//
bool Furby::IsDead()
{
	return dead;

}
//
// Shot
// Constructor
//
Shot::Shot(SDL_Surface *image, float x, float y, float targetX, float targetY)
{
	float a, b;
	
	SetId(SHOT_INDEX);
	SetImage(image);
	SetPos(x, y);
	lUpdate = SDL_GetTicks();
	vanished = false;
	
	framework->PlaySound(framework->EnemyShoot);

	a = targetX - x;
	b = y - targetY;
	
	movAngle = RAD2ARC(atan2(b, a));

	
}

//
// Update
//
void Shot::Update(Raycaster *caster)
{
	static float movX, movY;
	
	if(SDL_GetTicks() - lUpdate > SHOT_UPDATE_TIME)
	{
		movX = COS(movAngle);
		movY = -SIN(movAngle);
		
		x += movX;
		y += movY;
		
		if(caster->GetMapValueAt((int)x, (int)y) > 0)
			vanished = true;
		
		lUpdate = SDL_GetTicks();
	}
}

//
// IsVanished
//
bool Shot::IsVanished()
{
	return vanished;
}
//
// DeathAnimation
// Constructor
//
DeathAnimation::DeathAnimation(SDL_Surface *frames[NUM_DEATH_ANIM_FRAMES], float x, float y)
{
	int i;
	
	for(i=0; i<NUM_DEATH_ANIM_FRAMES; i++)
		this->frames[i] = frames[i];
	
	ended = false;
	curFrame = 0;
	
	SetPos(x, y);	
	SetImage(frames[curFrame]);
	SetId(DEATH_ANIMATION_INDEX);
		
	lUpdate = SDL_GetTicks();
}
//
// Update
//
void DeathAnimation::Update()
{
	if(SDL_GetTicks() - lUpdate > DA_UPDATE_TIME)
	{
		curFrame++;
		
		if(curFrame >=  NUM_DEATH_ANIM_FRAMES)
			ended = true;
		else
			SetImage(frames[curFrame]);
		
		lUpdate = SDL_GetTicks();
	}
}
//
// HasEnded
//
bool DeathAnimation::HasEnded()
{
	return ended;
}
//Skull DeathAnimation Constructor
SKDeathAnimation::SKDeathAnimation(SDL_Surface *frames[NUM_DEATH_ANIM_FRAMES], float x, float y)
{
	int i;
	
	for(i=0; i<NUM_DEATH_ANIM_FRAMES; i++)
		this->frames[i] = frames[i];
	
	ended = false;
	curFrame = 0;
	
	SetPos(x, y);	
	SetImage(frames[curFrame]);
	SetId(SKDEATH_ANIMATION_INDEX);
		
	lUpdate = SDL_GetTicks();
};
//
// Update Skull DeathAnimation
//
void SKDeathAnimation::Update()
{
	if(SDL_GetTicks() - lUpdate > DA_UPDATE_TIME)
	{
		curFrame++;
		
		if(curFrame >=  NUM_DEATH_ANIM_FRAMES)
			ended = true;
		else
			SetImage(frames[curFrame]);
		
		lUpdate = SDL_GetTicks();
	}
}
//
// Skull DA HasEnded
//
bool SKDeathAnimation::HasEnded()
{
	return ended;
}
//Skull Enemy Class
Skull::Skull(SDL_Surface *images[NUM_SK_IMGS], SDL_Surface *shotImg, float x, float y, list<Sprite *> *sprites)
{
	int i;
	
	dead = false;
	curImg = 0;
	
	for(i=0; i<NUM_SK_IMGS; i++)
		this->images[i] = images[i];
	
	SetImage(images[curImg]);
	SetPos(x, y);
	SetId(ET_SKULL);
	SetLookingAt(false);
	lUpdate = lShot = SDL_GetTicks();
	visible = false;
	this->sprites = sprites;
	this->shotImg = shotImg;
}

//
// Update
//
void Skull::Update(Raycaster *caster)
{	
	static float playerX, playerY;
	
	if(SDL_GetTicks() - lUpdate < FB_UPDATE_TIME || !visible)
		return;
	
	playerX = caster->GetPosX();
	playerY = caster->GetPosY();
	
	if(SDL_GetTicks() - lShot > SKULL_RELOAD_TIME)
	{
		SkullShot *newShot;


		newShot = new SkullShot(shotImg, x, y, playerX, playerY);		
		sprites->push_back(newShot);
		
		lShot = SDL_GetTicks();
	}
	

	if(ABS((playerX - x)) > 1.0f)
	{
		if(x < playerX)
		{
			x += FURBY_MOV_SPEED;
			
			if(caster->GetMapValueAt((int)x, (int)y) > 0)
				x -= FURBY_MOV_SPEED;
		}
		else
		{
			x -= FURBY_MOV_SPEED;
			
			if(caster->GetMapValueAt((int)x, (int)y) > 0)
				x += FURBY_MOV_SPEED;
		}
	}
	
	if(ABS((playerY - y)) > 1.0f)
	{
		if(y < playerY)
		{
			y += FURBY_MOV_SPEED;
			
			if(caster->GetMapValueAt((int)x, (int)y) > 0)
				y -= FURBY_MOV_SPEED;
		}
		else
		{
			y -= FURBY_MOV_SPEED;
			
			if(caster->GetMapValueAt((int)x, (int)y) > 0)
				y += FURBY_MOV_SPEED;
		}
	}
	
	lUpdate = SDL_GetTicks();
}

//
// SetVisible
// 
void Skull::SetVisible(bool visible)
{
	this->visible = visible;
}

//
// IsVisible
//
bool Skull::IsVisible()
{
	return visible;
}

//
// NextImage
//
void Skull::NextImage()
{
	curImg++;
	SetCurImage(curImg);
	
	if(curImg + 1 >= NUM_SK_IMGS)
		dead = true;
}
//
// SetCurImage
//
void Skull::SetCurImage(int curImg)
{
	if(curImg >= NUM_SK_IMGS)
		return;
	
	this->curImg = curImg;
	SetImage(images[curImg]);
}

//
// GetCurImage
//
int Skull::GetCurImage(void)
{
	return curImg;
}

//
// IsDead
//
bool Skull::IsDead()
{
	return dead;

}
//
// Shot
// Constructor
//
SkullShot::SkullShot(SDL_Surface *image, float x, float y, float targetX, float targetY)
{
	float a, b;
	
	SetId(SK_SHOT_INDEX);
	SetImage(image);
	SetPos(x, y);
	lUpdate = SDL_GetTicks();
	vanished = false;
	
	framework->PlaySound(framework->EnemyShoot);

	a = targetX - x;
	b = y - targetY;
	
	movAngle = RAD2ARC(atan2(b, a));

	
}
//
// Update
//
void SkullShot::Update(Raycaster *caster)
{
	static float movX, movY;
	
	if(SDL_GetTicks() - lUpdate > SHOT_UPDATE_TIME)
	{
		movX = COS(movAngle);
		movY = -SIN(movAngle);
		
		x += movX;
		y += movY;
		
		if(caster->GetMapValueAt((int)x, (int)y) > 0)
			vanished = true;
		
		lUpdate = SDL_GetTicks();
	}
}
//
// IsVanished
//
bool SkullShot::IsVanished()
{
	return vanished;
}