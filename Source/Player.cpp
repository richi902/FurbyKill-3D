/*
	FurbyKill 3D
	============
	License: GPL
	Author: Andre Schnabel
	E-Mail: thefrogs@web.de
	Homepage: http://www.semler-service.de/andre/
*/

// Player.cpp

#include "Player.h"

//
// Player
// Constructor
//
Player::Player()
{
	health = 100;
	ammo = 50;
	mgammo = 0;
	weapon = 0;
	hasKey = false;
	hasMachineGun = false;
}

//
// GetHealth
//
int Player::GetHealth()
{
	return health;
}

//
// SetHealth
//
void Player::SetHealth(int health)
{
	this->health = health;
}

//
// GetAmmo
//
int Player::GetAmmo()
{
	return ammo;
}

//
// SetAmmo
//
void Player::SetAmmo(int ammo)
{
	this->ammo = ammo;
}

//
// DecrementAmmo
//
void Player::DecrementAmmo(int value)
{
	if(ammo - value >= 0)
		ammo -= value;
}

//
// IncrementAmmo
//
void Player::IncrementAmmo(int value)
{
	ammo += value;
}

//
// IncrementHealth
//
void Player::IncrementHealth(int value)
{
	if(health + value <= 200)
		health += value;
}

//
// DecrementHealth
//
void Player::DecrementHealth(int value)
{
	if(health - value >= 0)
		health -= value;
	
	else if(health - value <=5)
		health = 0;
		
}

//
// HasKey
//
bool Player::HasKey()
{
	return hasKey;
}
//
// SetHasKey
//
void Player::SetHasKey(bool hasKey)
{
	this->hasKey = hasKey;
}

//Has MachineGun
bool Player::HasMachineGun()
{
	return hasMachineGun;
}
//SetHasMachineGun
void Player::SetHasMachineGun(bool hasMachineGun)
{
	this->hasMachineGun = hasMachineGun;
}
//get the current weapon
int Player::GetWeapon()
{
	return weapon;
}
//set the current weapon
void Player::SetWeapon(int index)
{
	if(index==0)
	{
		if(weapon == 0 && hasMachineGun==true)
			weapon = 1;
		else
			weapon = 0;
	}
	else if(index==1)
		weapon = 0;
	else if(index==2)
		weapon =1;
}
int Player::GetMGAmmo()
{
	return mgammo;
}
void Player::SetMGAmmo(int value)
{
	this->mgammo = mgammo;
}
void Player::DecrementMGAmmo(int value)
{
	if(mgammo - value >= 0)
		mgammo -= value;
}
void Player::IncrementMGAmmo(int value)
{
	mgammo += value;
}