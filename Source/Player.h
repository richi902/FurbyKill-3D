/*
	FurbyKill 3D
	============
	License: GPL
	Author: Andre Schnabel
	E-Mail: thefrogs@web.de
	Homepage: http://www.semler-service.de/andre/
*/

// Player.h

#ifndef PLAYER_H
#define PLAYER_H


#include "Framework.h"
#include "Sprite.h"

// Stores everything about the player except position and looking dir, since the raycasting
// class is storing this informations for us already!
class Player
{
	public:
		Player();
		
		int GetHealth();
		void SetHealth(int health);
		void IncrementHealth(int value);
		void DecrementHealth(int value);
		
		int GetAmmo();
		void SetAmmo(int ammo);
		void IncrementAmmo(int value);
		void DecrementAmmo(int value);

		int GetMGAmmo();
		void SetMGAmmo(int ammo);
		void IncrementMGAmmo(int value);
		void DecrementMGAmmo(int value);
		
		bool HasKey();
		void SetHasKey(bool hasKey);

		int GetWeapon();
		void SetWeapon(int index);
		
		bool HasMachineGun();
		void SetHasMachineGun(bool hasMachineGun);

	private:
		int health;
		int ammo;
		int mgammo;
		int weapon;
		bool hasKey;
		bool hasMachineGun;
};

#endif
