/*
	FurbyKill 3D
	============
	License: GPL
	Author: Andre Schnabel
	E-Mail: thefrogs@web.de
	Homepage: http://www.semler-service.de/andre/
*/

// Door.h

#ifndef DOOR_H
#define DOOR_H

#ifdef _XBOX
#include "../furbyxbox/x_Globals.h"
#elif GEKKO
#include "../furbywii/wii_Globals.h"
#elif _WIN32
#include "../furbywin/w32_Globals.h"
#elif linux
#include "../furbylinux/lin32_Globals.h"
#endif

class Raycaster;

#define OPEN_SPEED	0.12f
#define CLOSE_SPEED	0.08f
#define DELAY		100
#define OPEN_TIME	5000

class Door
{
	public:
		Door(int x, int y, Raycaster *caster, bool lockedDoor = false);

		void SetOpen(bool open);
		bool IsOpen();
		void GetPos(int *x, int *y);

		void SetOpenedWidth(float width);
		float GetOpenedWidth();

		void SetClosing(bool closing);
		bool GetClosing();

		void SetOpening(bool opening);
		bool GetOpening();

		void Update(float playerX, float playerY);

		bool IsLockedDoor();

	private:
		int x, y, itsIndex;
		bool open, opening, closing, empty;
		float openedWidth;
		Uint32 lStep, emptySince, openSince;
		Raycaster *caster;
		bool lockedDoor;
};

#endif
