/*
	FurbyKill 3D
	============
	License: GPL
	Author: Andre Schnabel
	E-Mail: thefrogs@web.de
	Homepage: http://www.semler-service.de/andre/
*/

// Door.cpp

#include "Framework.h"
#include "Door.h"
#include "Raycaster.h"

//
// Door
//
Door::Door(int x, int y, Raycaster *caster, bool lockedDoor)
{
	this->x = x;
	this->y = y;
	
	open = false;
	openedWidth = 0.0f;
	lStep = openSince = SDL_GetTicks();
	opening = closing = false;
	empty = true;
	
	this->caster = caster;
	this->lockedDoor = lockedDoor;
	
	if(lockedDoor)
		itsIndex = LOCKED_DOOR_INDEX;
	else
		itsIndex = DOOR_INDEX;
}

//
// SetOpen
//
void Door::SetOpen(bool open)
{
	this->open = open;
	
}

//
// IsOpen
//
bool Door::IsOpen()
{
	return open;
}

//
// GetPos
//
void Door::GetPos(int *x, int *y)
{
	*x = this->x;
	*y = this->y;
}

//
// SetOpenedWidth
//
void Door::SetOpenedWidth(float width)
{
	openedWidth = width;
	
	if(openedWidth >= 1.0f && closing == false)
	{
		open = true;
		caster->SetMapValueAt(x, y, 0);
		openSince = SDL_GetTicks();
		opening = false;
	}
	else if(openedWidth <= 0.0f && opening == false)
	{
		caster->SetMapValueAt(x, y, itsIndex);
		open = false;
		closing = false;
	}
}

//
// GetOpenedWidth
//
float Door::GetOpenedWidth()
{
	return openedWidth;
}

//
// SetClosing
//
void Door::SetClosing(bool closing)
{
	if(closing)
		caster->SetMapValueAt(x, y, itsIndex);
	
	this->closing = closing;
	framework->PlaySound(framework->ClosingDoor);
}

//
// GetClosing
//
bool Door::GetClosing()
{
	return closing;
}

//
// SetOpening
//
void Door::SetOpening(bool opening)
{
	if(opening)
		caster->SetMapValueAt(x, y, itsIndex);
	
	this->opening = opening;
	framework->PlaySound(framework->OpeningDoor);
}

//
// GetOpening
//
bool Door::GetOpening()
{
	return opening;
}

//
// Update
//
void Door::Update(float playerX, float playerY)
{
	if(closing)
	{
		if(SDL_GetTicks() - lStep > DELAY)
		{
			SetOpenedWidth(openedWidth - CLOSE_SPEED);
			lStep = SDL_GetTicks();
		}
	}
	else if(opening)
	{
		if(SDL_GetTicks() - lStep > DELAY)
		{
			SetOpenedWidth(openedWidth + OPEN_SPEED);
			lStep = SDL_GetTicks();
		}
	}
	else if(open)
	{
		if(!((int)playerX == x && (int)playerY == y))
		{
			if(!empty)
			{
				empty = true;
				emptySince = SDL_GetTicks();
			}
		}
		else
		{
			if(empty)
				empty = false;
		}
		
		if(empty)
		{
			if(SDL_GetTicks() - emptySince > OPEN_TIME && SDL_GetTicks() - openSince > OPEN_TIME)
			{
				openedWidth = 1.0f;
				SetClosing(true);
			}
		}
	}
}

//
// IsLockedDoor
//
bool Door::IsLockedDoor()
{
	return lockedDoor;
}
