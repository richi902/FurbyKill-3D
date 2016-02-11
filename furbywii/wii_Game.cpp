/*
	FurbyKill 3D
	============
	License: GPL
	Author: Andre Schnabel
	E-Mail: thefrogs@web.de
	Homepage: http://www.semler-service.de/andre/
*/

// Game.cpp

#include "../source/Game.h"
#include "../source/Raycaster.h"
#include "../source/Sprite.h"
#include "../source/Player.h"
#include "../source/Furby.h"
#include "wii_Globals.h"

//
// Game
// Constructor
//

Game::Game(const char *mapFilename, bool lowQuality, bool badQuality)
{
#
	if(mapFilename)
	{
		strcpy(this->mapFilename, DATA_PREFIX "maps/");
		strcat(this->mapFilename, DATA_PREFIX);
	}
	else
		strcpy(this->mapFilename, DATA_PREFIX "maps/map1");

	curMapNr = 1;

	this->lowQuality = lowQuality;
	this->badQuality = badQuality;
}

//
// Init
//
 void Game::Init()
{
	int i;
	RaycasterSetup setup;
	char spriteFilename[MAX_STR_LENGTH];
	
	strcpy(setup.mapFilename, mapFilename);
	
	strcpy(spriteFilename, mapFilename);
	strcat(spriteFilename, "sprites");
	
	if(framework->Exit==true)
	LoadResources();

	LoadSpriteMap(spriteFilename);
	
	setup.numTextures = numTextures;
	
	for(i=0; i<setup.numTextures; i++)
	{
		setup.textures[i][0] = textures[i][0];
		setup.textures[i][1] = textures[i][1];
	}
	
	setup.sprites = &sprites;
	
	setup.target = framework->GetScreen();
	setup.lowQuality = lowQuality;
	setup.badQuality = badQuality;
	
	setup.crosshairRect = &crosshairRect;
	
	fireVisible = false;
	fireStart = SDL_GetTicks();
	
	raycaster = new Raycaster(&setup);
	player = new Player();
	
	if(!framework->GetNoSound())
	{
		framework->PlayMusic(music1, -1);
		framework->PlaySound(startSnd);
	}
	
	showExitConfirm = false;
}
//
// Quit
//
void Game::Quit()
{
	list<Sprite *>::iterator it;
	Sprite *curSprite;
	
	delete player;
	
	for(it = sprites.begin(); it != sprites.end(); it++)
	{
		curSprite = *it;
		delete curSprite;
	}
	
	sprites.clear();
	
	if(framework->Exit==true)
	{
		FreeResources();
	}

	delete raycaster;
}

#define REACT_ON_PICKING() \
	if(wasPickup) \
	{ \
		switch(pickupId) \
		{ \
			case PT_MEDPACK: \
				player->IncrementHealth(35); \
				framework->PlaySound(pickHealth); \
				AddMessage("You picked up a Medpack!"); \
				break; \
			case PT_AMMO: \
				player->IncrementAmmo(15); \
				framework->PlaySound(pickAmmoSnd); \
				AddMessage("You picked up an Ammopack!"); \
				break; \
			case PT_KEY: \
				player->SetHasKey(true); \
				framework->PlaySound(pickKey); \
				AddMessage("You picked up the key!"); \
				break; \
			case PT_MACHINEGUN: \
				player->SetHasMachineGun(true); \
				player->IncrementMGAmmo(45); \
				framework->PlaySound(pickAmmoSnd); \
				AddMessage("You picked up the Machine Gun!"); \
				break; \
			case PT_MGAMMO: \
				player->IncrementMGAmmo(45); \
				framework->PlaySound(pickAmmoSnd); \
				AddMessage("You picked up an MG Ammopack!"); \
		} \
		wasPickup = false; \
	}

//
// Input
//
void Game::Input()
{
	static int mouseMovX, pickupId;
	static float movSpeed;
	static bool wasPickup = false, needAKey = false;
	float LX, LY;
	static Uint32 lNeedKeyMsg = SDL_GetTicks();
	int i;
	
	movSpeed = 2.0f;
	
	LX = framework->GetAxisAWPositive(0, JOY_AXIS_LX);
	LY = framework->GetAxisAWPositive(0, JOY_AXIS_LY);

	if(framework->GetJButton(0, JOY_BTTN_B))
	{
		if(framework->JButtonState(0, JOY_BTTN_B)==1)
			player->SetWeapon(0);
	}

	if(framework->KeyPressed(SDLK_1, true))
		player->SetWeapon(1);
	if(framework->KeyPressed(SDLK_2, true)&&player->HasMachineGun()==true)
		player->SetWeapon(2);

	if(framework->GetJButton(0, JOY_BTTN_SELECT)||framework->KeyPressed(SDLK_ESCAPE))
	{
		if(framework->JButtonState(0, JOY_BTTN_SELECT)==1||framework->KeyPressed(SDLK_ESCAPE))
			showExitConfirm = !showExitConfirm;
		else if(framework->JButtonState(0, JOY_BTTN_SELECT)==0)
			showExitConfirm = false;
	}
	
	if(showExitConfirm)
	{
		if(framework->KeyPressed(SDLK_y)||framework->GetJButton(0, JOY_BTTN_Y))
			SetDone(true);
		else if(framework->KeyPressed(SDLK_n)||framework->GetJButton(0, JOY_BTTN_B))
			showExitConfirm = false;
				
		return;
	}
	
	if(framework->KeyPressed(SDLK_F1, true))
	{
		int i;
		char shotFilename[MAX_STR_LENGTH];
		for(i=0; i<MAX_SCREENSHOTS; i++)
		{
			sprintf(shotFilename, "screenshots/shot%d.bmp", i+1);
			
			if(!framework->FileExists(shotFilename))
			{
				SDL_SaveBMP(framework->GetScreen(), shotFilename);
				break;
			}
		}
	}
	


	if(framework->KeyPressed(SDLK_LSHIFT, false)||framework->GetJButton(0, JOY_BTTN_ZL))
		movSpeed *= 2.0f, LX *= 2.0f, LY *= 2.0f;


	if(framework->KeyPressed(SDLK_LEFT, false)&&!framework->KeyPressed(SDLK_LALT, false))
		raycaster->Rotate(60);

	if(framework->KeyPressed(SDLK_RIGHT, false)&&!framework->KeyPressed(SDLK_LALT, false))
		raycaster->Rotate(-60);
	
	if(framework->GetAxis(0, JOY_AXIS_RX))
		raycaster->Rotate(framework->GetAxis(0, JOY_AXIS_RX)/200*-1.0);
	
	if(framework->KeyPressed(SDLK_UP, false) || framework->KeyPressed(SDLK_w, false))
		raycaster->MoveForward(movSpeed, &wasPickup, &pickupId);
	
	if(framework->GetAxis(0, JOY_AXIS_LY)<=-8000)
		raycaster->MoveForward(LY, &wasPickup, &pickupId);

	if(framework->GetHatCentered(0) == true && framework->GetHatWhich(0, 0) == SDL_HAT_UP||framework->GetHatCentered(0) == true && framework->GetHatWhich(0, 0)==SDL_HAT_LEFTUP||framework->GetHatCentered(0) == true && framework->GetHatWhich(0, 0)==SDL_HAT_RIGHTUP)
		raycaster->MoveForward(movSpeed, &wasPickup, &pickupId);

	REACT_ON_PICKING();

	if(framework->KeyPressed(SDLK_DOWN, false) || framework->KeyPressed(SDLK_s, false))
		raycaster->MoveBackward(movSpeed, &wasPickup, &pickupId);

	if(framework->GetHatCentered(0) == true && framework->GetHatWhich(0, 0) == SDL_HAT_DOWN||framework->GetHatCentered(0) == true && framework->GetHatWhich(0, 0)==SDL_HAT_LEFTDOWN||framework->GetHatCentered(0) == true && framework->GetHatWhich(0, 0)==SDL_HAT_RIGHTDOWN)
		raycaster->MoveBackward(movSpeed, &wasPickup, &pickupId);

	if(framework->GetAxis(0, JOY_AXIS_LY)>=8000)
		raycaster->MoveBackward(LY, &wasPickup, &pickupId);

	REACT_ON_PICKING();
	
	if(framework->KeyPressed(SDLK_a, false)||framework->KeyPressed(SDLK_LEFT, false)&&framework->KeyPressed(SDLK_LALT, false))
		raycaster->StepLeft(movSpeed / 2.0, &wasPickup, &pickupId);
		
	if(framework->GetHatCentered(0) == true && framework->GetHatWhich(0, 0) == SDL_HAT_LEFT||framework->GetHatCentered(0) == true && framework->GetHatWhich(0, 0) == SDL_HAT_LEFTUP||framework->GetHatCentered(0) == true && framework->GetHatWhich(0, 0)==SDL_HAT_LEFTDOWN)
		raycaster->StepLeft(movSpeed / 2.0f, &wasPickup, &pickupId);

	if(framework->GetAxis(0, JOY_AXIS_LX)<=-8000)
		raycaster->StepLeft(LX / 2.0f, &wasPickup, &pickupId);

	REACT_ON_PICKING();
	
	if(framework->KeyPressed(SDLK_d, false)||framework->KeyPressed(SDLK_RIGHT, false)&&framework->KeyPressed(SDLK_LALT, false))
		raycaster->StepRight(movSpeed / 2.0f, &wasPickup, &pickupId);

	if(framework->GetHatCentered(0) == true && framework->GetHatWhich(0, 0) == SDL_HAT_RIGHT||framework->GetHatCentered(0) == true && framework->GetHatWhich(0, 0)==SDL_HAT_RIGHTUP||framework->GetHatCentered(0) == true && framework->GetHatWhich(0, 0)==SDL_HAT_RIGHTDOWN)
		raycaster->StepRight(movSpeed / 2.0f, &wasPickup, &pickupId);

	if(framework->GetAxis(0, JOY_AXIS_LX)>=8000)
		raycaster->StepRight(LX / 2.0f, &wasPickup, &pickupId);

	REACT_ON_PICKING();
	
	needAKey = false;
	
	if(framework->KeyPressed(SDLK_SPACE, false)||framework->GetJButton(0, JOY_BTTN_X))
		raycaster->OpenDoor(player->HasKey(), &needAKey);
		
	if(needAKey && SDL_GetTicks() - lNeedKeyMsg > 2000)
	{
		AddMessage("You need a key to unlock this door!");
		lNeedKeyMsg = SDL_GetTicks();
	}

	
	if(framework->KeyPressed(SDLK_LCTRL, false) || framework->ButtonPressed(BUTTON_LEFT)||framework->GetJButton(0, JOY_BTTN_ZR))
		Shoot();


	
	framework->GetMouseMovement(&mouseMovX, NULL);
	raycaster->Rotate((float)mouseMovX/10.0f*-1.0f*ROT_SPEED);
}

//
// Update
//
void Game::Update()
{
	list<Sprite *>::iterator it;
	Sprite *curSpr;
	static bool HPwarning = false;
	static int chan = NULL;

	if(showExitConfirm)
		return;
	
	UpdateMessages();
	
	if(player->GetWeapon()==0)
		RELOAD_TIME = 650;
	if(player->GetWeapon()==1)
		RELOAD_TIME = 280;

	// Update sprites
	for(it = sprites.begin(); it != sprites.end(); it++)
	{
		curSpr = *it;
		
		if(curSpr->GetId() == ET_FURBY)
		{
			Furby *fb = (Furby *)curSpr;
			fb->Update(raycaster);
		
		}
		else if(curSpr->GetId() == SHOT_INDEX)
		{
			Shot *shot = (Shot *)curSpr;	
			shot->Update(raycaster);
			
			if(shot->PointCollWith(raycaster->GetPosX(), raycaster->GetPosY()))
			{
				if(player->GetHealth() != 0)
				{
					framework->PlaySound(PlayerHurt);
				}
			AddMessage("You got hit by a shot from a furby!");
				player->DecrementHealth(10);

				it = sprites.erase(it);
				
			}
			if(shot->IsVanished())
				it = sprites.erase(it);
		}
		else if(curSpr->GetId() == DEATH_ANIMATION_INDEX)
		{
			DeathAnimation *da = (DeathAnimation *)curSpr;
			da->Update();
			
			if(da->HasEnded())
			{
				float itsX, itsY;
				itsX = da->GetPosX();
				itsY = da->GetPosY();
				
				delete da;
				it = sprites.erase(it);
				
				
				Sprite *corpse = new Sprite(spriteImgs[2], itsX, itsY, 2);
				sprites.push_back(corpse);				
			}
		}

		if(curSpr->GetId() == ET_SKULL)
		{
			Skull *sk = (Skull *)curSpr;
			sk->Update(raycaster);
		}
		else if(curSpr->GetId() == SK_SHOT_INDEX)
		{
			SkullShot *skullshot = (SkullShot *)curSpr;
			skullshot->Update(raycaster);

			if(skullshot->PointCollWith(raycaster->GetPosX(), raycaster->GetPosY()))
			{
				if(player->GetHealth() !=0)
				{
					framework->PlaySound(PlayerHurt);
				}
			
				AddMessage("You got hit by a shot from a Skull!");
				player->DecrementHealth(20);

				it = sprites.erase(it);
			}
			if(skullshot->IsVanished())
				it = sprites.erase(it);
		}
		else if(curSpr->GetId() == SKDEATH_ANIMATION_INDEX)
		{
			SKDeathAnimation *skda = (SKDeathAnimation *)curSpr;
			skda->Update();
			
			if(skda->HasEnded())
			{
				float itsX, itsY;
				itsX = skda->GetPosX();
				itsY = skda->GetPosY();
				
 				delete skda;
				it = sprites.erase(it);
				
				Sprite *skcorpse = new Sprite(spriteImgs[20], itsX, itsY, 2);
				sprites.push_back(skcorpse);				
			}
		}

		
	
	}

	if(player->GetHealth() <= 30 && HPwarning == false)
	{
		chan = framework->PlaySound(HeartBeat, -1);
		Mix_Volume(chan, MIX_MAX_VOLUME);
		HPwarning = true;
	}

	if(player->GetHealth() > 30 && HPwarning == true)
	{
			Mix_HaltChannel(chan);
			HPwarning = false;
	}

	if(player->GetHealth() == 0)
	{
		
		framework->PlaySound(PlayerDead);
		GameOver();
	}
	
	if(raycaster->MapChangeNeeded())
        NextMap();	
}

//
// Draw
//
void Game::Draw()
{
	if(!showExitConfirm)
	{
		raycaster->Draw();
		DrawHUD();
		DrawMessages();
		framework->ShowFPS();
		//if(framework->GetJButton(0, JOY_BTTN_START))
			framework->Debug(true);
			
	}
	else
	{
		framework->FillRect(NULL, Colors::black.ToUint32());
		framework->DrawTextCentered("Do you really want to quit this game?"
				" Press Y for Yes and N For No...", Colors::white);
	}
}

#define UPDATE_STATUS(str) \
	framework->FillRect(NULL, Colors::black.ToUint32()); \
	framework->DrawTextCentered(str, Colors::white); \
	SDL_UpdateRect(framework->GetScreen(),0 ,0 ,0, 0);
	//SDL_Flip(framework->GetScreen());

//
// LoadResources
//
void Game::LoadResources()
{
	int i;
	static bool Loaded = false;
	char filename[MAX_STR_LENGTH], completeFilename[MAX_STR_LENGTH];
	
	if(!Loaded)
	{

		UPDATE_STATUS("Loading resources...");
	
		for(i=0; i<MAX_SPRITES; i++)
			spriteImgs[i] = NULL;
		
		for(i=0; i<MAX_TEXTURES; i++)
		{
			textures[i][0] = NULL;	
			textures[i][1] = NULL;
		}
	
		UPDATE_STATUS("Loading textures...");
	
	
		for(i=0; i<MAX_TEXTURES; i++)
		{
			sprintf(filename, "textures/map1/tex%d.bmp",i+1);
			sprintf(completeFilename, "%s%s", DATA_PREFIX, filename);
			if(framework->FileExists(completeFilename))
			{
				textures[i][0] = framework->LoadImage(filename, false);
				sprintf(filename, "textures/map1/tex%dDark.bmp", i+1);
				textures[i][1] = framework->LoadImage(filename, false);
			}
			else
				break;
		}
	
		numTextures = i;
	
		UPDATE_STATUS("Loading sprites...");
	

		for(i=0; i<MAX_SPRITES; i++)
		{
			sprintf(filename, "sprites/map1/sprite%d.bmp", i+1);
			sprintf(completeFilename, "%s%s", DATA_PREFIX, filename);
			if(framework->FileExists(completeFilename))
				spriteImgs[i] = framework->LoadImage(filename, true);
			else
				break;
		}

	}
	UPDATE_STATUS("Loading ui images...");
	
	if(lowQuality)
	{
		hudBgImg = framework->LoadImage("images\\lowQuality\\hudBg.bmp", false);
		crosshairImg = framework->LoadImage("images\\lowQuality\\crosshair.bmp", true);
		headImg = framework->LoadImage("images\\lowQuality\\head.bmp", true);
		headRedImg = framework->LoadImage("images\\lowQuality\\headRed.bmp", true);
		gunHudImg = framework->LoadImage("images\\lowQuality\\gunHud.bmp", true);
		gunImg = framework->LoadImage("images\\lowQuality\\gun.bmp", true);
		fireImg = framework->LoadImage("images\\lowQuality\\fire.bmp", true);
		keyImg = framework->LoadImage("images\\lowQuality\\key.bmp", true);
	}

	else
	{		
		hudBgImg = framework->LoadImage("images/hudBg.bmp", false);
		crosshairImg = framework->LoadImage("images/crosshair.bmp", true);
		headImg = framework->LoadImage("images/head.bmp", true);
		headRedImg = framework->LoadImage("images/headRed.bmp", true);
		gunHudImg = framework->LoadImage("images/gunHud.bmp", true);	
		gunImg = framework->LoadImage("images/gun.bmp", true);
		fireImg = framework->LoadImage("images/fire.bmp", true);
		keyImg = framework->LoadImage("images/key.bmp", true);
		MGgunImg = framework->LoadImage("images/mg.bmp", true);
		MGfireImg = framework->LoadImage("images/mg_fire.bmp", true);
		MGgunHudImg = framework->LoadImage("images/mg_Hud.bmp", true);
		
		SDL_SetAlpha(hudBgImg, SDL_SRCALPHA, SDL_ALPHA_OPAQUE / 2);		
		SDL_SetAlpha(headRedImg, SDL_SRCALPHA, SDL_ALPHA_OPAQUE / 2);
	}

	crosshairRect.x = (framework->GetScrW() - crosshairImg->w) / 2;
	crosshairRect.y = (framework->GetScrH() - crosshairImg->h) / 2;
	crosshairRect.w = crosshairImg->w;
	crosshairRect.h = crosshairImg->h;	
	
	UPDATE_STATUS("Loading Music resources...");



	if(!framework->GetNoSound())
	{

		if(!Loaded)
		{
			music1 = framework->LoadMusic("music/music1.ogg");
			Loaded = true;
		}

		UPDATE_STATUS("Loading Sounds resources 1...");

		startSnd = framework->LoadSound("sounds/startround.ogg");	

		UPDATE_STATUS("Loading Sounds resources 1...");
		shootSnd = framework->LoadSound("sounds/shot.ogg");
		UPDATE_STATUS("Loading Sounds resources 2...");
		killSnd = framework->LoadSound("sounds/scream.ogg");
		UPDATE_STATUS("Loading Sounds resources 3...");
		pickAmmoSnd = framework->LoadSound("sounds/pickammo.ogg");
		UPDATE_STATUS("Loading Sounds resources 4...");
		PlayerHurt = framework->LoadSound("sounds/PlayerHurt.ogg");
		UPDATE_STATUS("Loading Sounds resources 5...");
		EnemyHurt = framework->LoadSound("sounds/EnemyHurt.ogg");
		UPDATE_STATUS("Loading Sounds resources 6...");
		PlayerDead = framework->LoadSound("sounds/PlayerDead.ogg");
		UPDATE_STATUS("Loading Sounds resources 7...");
		pickKey = framework->LoadSound("sounds/pickKey.ogg");
		UPDATE_STATUS("Loading Sounds resources 8...");
		pickHealth = framework->LoadSound("sounds/pickHealth.ogg");
		UPDATE_STATUS("Loading Sounds resources 9...");
		LVLClear = framework->LoadSound("sounds/LVLClear.ogg");
		UPDATE_STATUS("Loading Sounds resources 10...");
		MGshootSnd = framework->LoadSound("sounds/MGshoot.ogg");
		UPDATE_STATUS("Loading Sounds resources 11...");
		HeartBeat = framework->LoadSound("sounds/Heartbeat.ogg");
		
		framework->GlobalSounds();
	}
	
}

//
// FreeResources
//
void Game::FreeResources()
{
	int i;
	
	if(!framework->GetNoSound())
	{
		Mix_HaltChannel(-1);
		Mix_FreeChunk(pickAmmoSnd);
		Mix_FreeChunk(killSnd);
		Mix_FreeChunk(shootSnd);
		Mix_FreeChunk(MGshootSnd);
		Mix_FreeChunk(startSnd);
		Mix_FreeChunk(PlayerHurt);
		Mix_FreeChunk(EnemyHurt);
		Mix_FreeChunk(PlayerDead);
		Mix_FreeChunk(pickKey);
		Mix_FreeChunk(pickHealth);
		Mix_FreeChunk(framework->EnemyShoot);
		Mix_FreeChunk(framework->ClosingDoor);
		Mix_FreeChunk(framework->OpeningDoor);
		Mix_FreeChunk(HeartBeat);
		Mix_FreeChunk(LVLClear);
		Mix_HaltMusic();
		Mix_FreeMusic(music1);
	}
	
	SDL_FreeSurface(keyImg);
	SDL_FreeSurface(fireImg);
	SDL_FreeSurface(gunImg);
	SDL_FreeSurface(gunHudImg);
	SDL_FreeSurface(headRedImg);
	SDL_FreeSurface(hudBgImg);
	SDL_FreeSurface(headImg);
	SDL_FreeSurface(crosshairImg);
	SDL_FreeSurface(MGgunHudImg);
	SDL_FreeSurface(MGgunImg);
	SDL_FreeSurface(MGfireImg);
	
	for(i=0; i<MAX_SPRITES; i++)
	{
		if(textures[i] != NULL)
			SDL_FreeSurface(spriteImgs[i]);
		else
			break;
	}
	
	for(i=0; i<MAX_TEXTURES; i++)
	{
		if(textures[i][0] != NULL)
		{
			SDL_FreeSurface(textures[i][0]);
			SDL_FreeSurface(textures[i][1]);
		}
		else
			break;
	}
}


//
// LoadSpriteMap
//
void Game::LoadSpriteMap(const char *filename)
{
	RaycasterSetup setup;
	int i, sprType, numSprites;
	float x, y;
	FILE *fp;
	SDL_Surface *image;
	Sprite *sprite;
	
	numSprites = framework->CountRowsInFile(filename);
	
	fp = fopen(filename, "r");
	
	for(i=0; i<numSprites; i++)
	{
		fscanf(fp, "%d, %f, %f\n", &sprType, &x, &y);
		
		// Static object
		if(sprType < PICKUP_INDEX)
		{
			image = spriteImgs[sprType];
			sprite = new Sprite(image, x, y, sprType);
		}
		// Object you can pick up
		else if(sprType < ENEMY_INDEX)
		{
			switch(sprType)
			{
				case PT_MEDPACK:
					image = spriteImgs[3];
					sprite = new Sprite(image, x, y, PT_MEDPACK);
					break;				
				case PT_AMMO:
					image = spriteImgs[4];
					sprite = new Sprite(image, x, y, PT_AMMO);
					break;
				case PT_KEY:
					image = spriteImgs[10];
					sprite = new Sprite(image, x, y, PT_KEY);
					break;
				case PT_MACHINEGUN:
					image = spriteImgs[22];
					sprite = new Sprite(image, x, y, PT_MACHINEGUN);
					break;
				case PT_MGAMMO:
					image = spriteImgs[23];
					sprite = new Sprite(image, x, y, PT_MGAMMO);
					break;
			}
		}
		// Enemy
		else
		{
			switch(sprType)
			{
				case ET_FURBY:
					SDL_Surface *furbyImgs[NUM_FB_IMGS];					
					furbyImgs[0] = spriteImgs[0];
					furbyImgs[1] = spriteImgs[8];
					furbyImgs[2] = spriteImgs[8];
					furbyImgs[3] = spriteImgs[9];
					furbyImgs[4] = spriteImgs[9];
					sprite = new Furby(furbyImgs, spriteImgs[7], x, y, &sprites);
					break;
				case ET_SKULL:
					SDL_Surface *skullImgs[NUM_SK_IMGS];
					skullImgs[0] = spriteImgs[14];
					skullImgs[1] = spriteImgs[15];
					skullImgs[2] = spriteImgs[15];
					skullImgs[3] = spriteImgs[15];
					skullImgs[4] = spriteImgs[16];
					skullImgs[5] = spriteImgs[16];
					skullImgs[6] = spriteImgs[16];
					skullImgs[7] = spriteImgs[16];
					sprite = new Skull(skullImgs, spriteImgs[21], x, y, &sprites);
					break;
			}
		}
		
		sprites.push_back(sprite);
	}
	
	fclose(fp);
}

//
// DrawHUD
//
void Game::DrawHUD()
{
	static int health, ammunition, mgammunition;
	static char healthStr[MAX_STR_LENGTH], ammoStr[MAX_STR_LENGTH], mgammoStr[MAX_STR_LENGTH];
	
	health = player->GetHealth();
	ammunition = player->GetAmmo();
	mgammunition = player->GetMGAmmo(); 
	
	sprintf(healthStr, "%d", health);
	sprintf(ammoStr, "%d", ammunition);
	sprintf(mgammoStr, "%d", mgammunition); 
	
	framework->DrawCentered(crosshairImg);
	
	if(fireVisible)
	{
		if(SDL_GetTicks() - fireStart < FIRE_SHOW_TIME)
		{
			/*if(!lowQuality)
				framework->Draw(fireImg, (framework->GetScrW() - fireImg->w) / 2 + 100, framework->GetScrH() - fireImg->h - 50);
			else*/
			/*if(player->HasMachineGun())
				framework->Draw(MGfireImg, framework->ArmMovmeantW(), framework->ArmMovmeantH()); 
			else
				framework->Draw(fireImg, framework->ArmMovmeantW(), framework->ArmMovmeantH());*///(framework->GetScrW() - fireImg->w) / 2 + 50, framework->GetScrH() - fireImg->h - 25);
			
			//set the current gun fire icon to draw
			if(player->GetWeapon() == 0)
					framework->Draw(fireImg, framework->ArmMovmeantW(), framework->ArmMovmeantH());
			else if(player->GetWeapon() == 1 && player->HasMachineGun())
					framework->Draw(MGfireImg, framework->ArmMovmeantW(), framework->ArmMovmeantH());
					
			
		}
		else
			fireVisible = false;
	}

	//set the current weapon icon to draw
	if(player->GetWeapon()== 0)
		framework->Draw(gunImg, framework->ArmMovmeantW(), framework->ArmMovmeantH());
	else if(player->GetWeapon() == 1 && player->HasMachineGun())		
		framework->Draw(MGgunImg, framework->ArmMovmeantW(), framework->ArmMovmeantH());
					
	framework->Draw(hudBgImg, (framework->GetScrW() - hudBgImg->w) / 2, framework->GetScrH() - hudBgImg->h);
	framework->Draw(headImg, (framework->GetScrW() - headImg->w) / 2, framework->GetScrH() - headImg->h);
	
	if(health < 100)
	{
		static SDL_Rect srcRect, destRect;
		
		srcRect.x = 0;
		srcRect.y = (int)((float)headRedImg->h * ((float)health / 100.0f));
		srcRect.w = headRedImg->w;
		srcRect.h = headRedImg->h - srcRect.y;
				
		destRect.w = srcRect.w;
		destRect.h = srcRect.h;
		
		destRect.x = (framework->GetScrW() - headImg->w) / 2;
		destRect.y = framework->GetScrH() - headImg->h + srcRect.y;
		
		SDL_BlitSurface(headRedImg, &srcRect, framework->GetScreen(), &destRect);
	}
	
	/*if(player->HasMachineGun())
		framework->Draw(MGgunHudImg, (framework->GetScrW() - hudBgImg->w) /2 + 2, framework->GetScrH() - MGgunHudImg->h);
	else
		framework->Draw(gunHudImg, (framework->GetScrW() - hudBgImg->w) / 2 + 2, framework->GetScrH() - gunHudImg->h);*/

	//set the current hud weapon icon to draw
	if(player->GetWeapon()==0)
		framework->Draw(gunHudImg,(framework->GetScrW() - hudBgImg->w) /2 + 2, framework->GetScrH() - gunHudImg->h);
	else if(player->GetWeapon() == 1 && player->HasMachineGun())	
		framework->Draw(MGgunHudImg, (framework->GetScrW() - hudBgImg->w) /2 + 2, framework->GetScrH() - MGgunHudImg->h);
			
	if(player->HasKey())
	{
		if(!lowQuality)
			framework->Draw(keyImg, framework->GetScrW() - keyImg->w - 100, framework->GetScrH() - keyImg->h - 10);
		else
			framework->Draw(keyImg, framework->GetScrW() - keyImg->w - 50, framework->GetScrH() - keyImg->h - 5);
	}
	
	if(player->GetWeapon() == 0)
	{
		framework->DrawText(ammoStr,
			(framework->GetScrW() - hudBgImg->w) / 2 + gunHudImg->w + 8,
			framework->GetScrH() - gunHudImg->h,
			Colors::white, true);
	}
	else if(player->GetWeapon() == 1)
	{
		framework->DrawText(mgammoStr,
			(framework->GetScrW() - hudBgImg->w) / 2 + gunHudImg->w + 8,
			framework->GetScrH() - gunHudImg->h,
			Colors::white, true);
	}
	framework->DrawText(healthStr,
		(framework->GetScrW() - headImg->w) / 2 + headImg->w + 4,
		framework->GetScrH() - gunHudImg->h,
		Colors::white, true);
}

//
// Shoot
//
void Game::Shoot()
{
	static Uint32 lShot = SDL_GetTicks() - RELOAD_TIME;
	static list<SprDraw> killableSprites;
	static list<SprDraw>::iterator ksIt;
	static float distance, a, b, c, d;
	
	if(SDL_GetTicks() - lShot > RELOAD_TIME)
	{
		list<Sprite *>::iterator it;
		Sprite *spr;
		
		if(player->GetWeapon() == 0)
		{
			player->DecrementAmmo(1);
			if(player->GetAmmo() == 0)
			{
				AddMessage("Out of ammo!");
				return;
			}
		}
		else if(player->GetWeapon() == 1)
		{
			player->DecrementMGAmmo(1);
			if(player->GetMGAmmo() == 0)
			{
				AddMessage("Out of ammo!");
				return;
			}
		}
		
		
		
		fireStart = SDL_GetTicks();
		fireVisible = true;
		
		if(player->GetWeapon()==0)
			framework->PlaySound(shootSnd);
		else if(player->GetWeapon()==1)
			framework->PlaySound(MGshootSnd);
		
		for(it = sprites.begin(); it != sprites.end(); it++)
		{
			spr = *it;
			if(spr->GetLookingAt() && spr->GetId() == ET_FURBY)
			{				
				SprDraw ks;
				ks.original = spr;
				ks.x = spr->GetPosX();
				ks.y = spr->GetPosY();
				ks.image = spr->GetImage();
				a = raycaster->GetPosX() - ks.x;
				b = raycaster->GetPosY() - ks.y;
				distance = sqrt(a*a+b*b);
				ks.distance = distance;
				killableSprites.push_back(ks);				
			}
			
			if(spr->GetLookingAt() && spr->GetId() == ET_SKULL)
			{
				SprDraw ks;
				ks.original = spr;
				ks.x = spr->GetPosX();
				ks.y = spr->GetPosY();
				ks.image = spr->GetImage();
				c = raycaster->GetPosX() - ks.x;
				d = raycaster->GetPosY() - ks.y;
				distance = sqrt(a*a+b*b);
				ks.distance = distance;
				killableSprites.push_back(ks);	
			}
		}
			
		

		if(!killableSprites.empty())
		{
			killableSprites.sort();
			killableSprites.reverse();
		
			ksIt = killableSprites.begin();

			Furby *thisFurby = (Furby *)(*ksIt).original;
		    Skull *thisSkull = (Skull*)(*ksIt).original;

			if(!thisFurby->IsDead()&&thisFurby->GetId()==ET_FURBY)
			{
				thisFurby->NextImage();
				framework->PlaySound(EnemyHurt);
			}	
			else if(thisFurby->IsDead() && thisFurby->GetId()==ET_FURBY)
			{
				SDL_Surface *daFrames[NUM_DEATH_ANIM_FRAMES];
			
				daFrames[0] = spriteImgs[11];
				daFrames[1] = spriteImgs[12];
				daFrames[2] = spriteImgs[13];
			
				DeathAnimation *deathAnim = new DeathAnimation(daFrames, (*ksIt).original->GetPosX(), (*ksIt).original->GetPosY());
				
				sprites.push_back(deathAnim);
				
				delete (*ksIt).original;				
				sprites.remove((*ksIt).original);
			
				framework->PlaySound(killSnd);
				
				
					AddMessage("You killed a furby!");
			}
			
			if(!thisSkull->IsDead() && thisSkull->GetId()==ET_SKULL)
			{
				thisSkull->NextImage();
				framework->PlaySound(EnemyHurt);

			}
			else if(thisSkull->IsDead() && thisSkull->GetId()==ET_SKULL)
			{
				SDL_Surface *skdaFrames[NUM_DEATH_ANIM_FRAMES];
				
				skdaFrames[0] = spriteImgs[17];
				skdaFrames[1] = spriteImgs[18];
				skdaFrames[2] = spriteImgs[19];
			
				SKDeathAnimation *skdeathAnim = new SKDeathAnimation(skdaFrames, (*ksIt).original->GetPosX(), (*ksIt).original->GetPosY());
				
				sprites.push_back(skdeathAnim);
				
				delete (*ksIt).original;				
				sprites.remove((*ksIt).original);
				
				framework->PlaySound(killSnd);
				
				AddMessage("You killed a skull!");
			}
			
		//}
		killableSprites.clear();		
		}

		lShot = SDL_GetTicks();
	}
}

//
// NextMap
//
void Game::NextMap()
{	
	RaycasterSetup setup;
	int i;
	char spriteMapFilename[MAX_STR_LENGTH], filename[MAX_STR_LENGTH], completeFilename[MAX_STR_LENGTH];
	
	Mix_HaltMusic();
	framework->PlaySound(LVLClear);
	Loading1 = framework->LoadImage("Images/loading.bmp"); 

	framework->Draw(Loading1, 0, 0);
	framework->DrawTextCentered("Loading next map...", Colors::white, true);
	SDL_Flip(framework->GetScreen());
	Mix_FreeMusic(music1);

	for(i=0; i<MAX_SPRITES; i++)
	{
		if(textures[i] != NULL)
			SDL_FreeSurface(spriteImgs[i]);
		else
			break;
	}
	for(i=0; i<MAX_TEXTURES; i++)
	{
		if(textures[i][0] != NULL)
		{
			SDL_FreeSurface(textures[i][0]);
			SDL_FreeSurface(textures[i][1]);
		}
		else
			break;
	}
	for(i=0; i<MAX_SPRITES; i++)
		spriteImgs[i] = NULL;
	
	for(i=0; i<MAX_TEXTURES; i++)
	{
		textures[i][0] = NULL;	
		textures[i][1] = NULL;
	}
	
	curMapNr++;

buildNameLbl:

	sprintf(mapFilename, DATA_PREFIX "maps/map%d", curMapNr);
	strcpy(spriteMapFilename, mapFilename);
	strcat(spriteMapFilename, "sprites");
	
	if(!framework->FileExists(mapFilename))
	{
		curMapNr = 1;
		goto buildNameLbl;
	}
	
	if(curMapNr == 1)
		music1 = framework->LoadMusic("music/music1.ogg");
	else if(curMapNr == 2)
		music1 = framework->LoadMusic("music/music2.ogg");
	else if(curMapNr == 3)
		music1 = framework->LoadMusic("music/music3.ogg");
	else if(curMapNr == 4)
		music1 = framework->LoadMusic("music/music4.ogg");
	else if(curMapNr == 5)
		music1 = framework->LoadMusic("music/music5.ogg");
	else
		music1 = framework->LoadMusic("music/music_orig.ogg");

	for(i=0; i<MAX_TEXTURES; i++)
	{
		sprintf(filename, "textures/map%d/tex%d.bmp", curMapNr, i+1);
		sprintf(completeFilename, "%s%s", DATA_PREFIX, filename);
		if(framework->FileExists(completeFilename))
		{
			textures[i][0] = framework->LoadImage(filename, false);
			sprintf(filename, "textures/map%d/tex%dDark.bmp", curMapNr, i+1);
			textures[i][1] = framework->LoadImage(filename, false);
		}
		else
			break;
	}

	numTextures = i;
	
	for(i=0; i<MAX_SPRITES; i++)
	{
		sprintf(filename, "sprites/map%d/sprite%d.bmp", curMapNr, i+1);
		sprintf(completeFilename, "%s%s", DATA_PREFIX, filename);
		if(framework->FileExists(completeFilename))
			spriteImgs[i] = framework->LoadImage(filename, true);
		else
			break;
	}


	strcpy(setup.mapFilename, mapFilename);
	
	setup.numTextures = numTextures;
	
	for(i=0; i<setup.numTextures; i++)
	{
		setup.textures[i][0] = textures[i][0];
		setup.textures[i][1] = textures[i][1];
	}
	
	setup.sprites = &sprites;
	
	setup.target = framework->GetScreen();
	setup.lowQuality = lowQuality;
	setup.badQuality = badQuality;
	
	setup.crosshairRect = &crosshairRect;
	
	fireVisible = false;
	fireStart = SDL_GetTicks();
	
	delete raycaster;
	raycaster = new Raycaster(&setup);

	raycaster->LoadMap(mapFilename);
	sprites.clear();
	
	LoadSpriteMap(spriteMapFilename);
	raycaster->SetAlpha(0.0f);
	
	player->SetHasKey(false);
	
	if(player->GetHealth()<100)
		player->SetHealth(100);

	if(player->GetAmmo()<50)
		player->SetAmmo(50);

	if(player->GetMGAmmo()<70)
		player->SetMGAmmo(70);

	/*delete player;
	player = new Player();*/
	SDL_Delay(3000);
	framework->FillRect(NULL, Colors::black.ToUint32());
	SDL_FreeSurface(Loading1);
	framework->PlayMusic(music1, -0);
}
//
// UpdateMessages
//
void Game::UpdateMessages()
{
	list<TextMessage>::iterator it;
	TextMessage *curMsg;
	
	for(it = msgs.begin(); it != msgs.end(); it++)
	{
		curMsg = &(*it);
		
		if(SDL_GetTicks() - curMsg->sTime > MSG_SHOW_TIME)
			it = msgs.erase(it);
	}
}

//
// DrawMessages
//
void Game::DrawMessages()
{
	list<TextMessage>::iterator it;
	int drawY = 0;
	TextMessage *curMsg;
	
	for(it = msgs.begin(); it != msgs.end(); it++)
	{
		curMsg = &(*it);
		
		framework->DrawText(curMsg->text, 0, drawY, Colors::white);
		
		if(!lowQuality)
			drawY += 20;
		else
			drawY += 10;
	}
}

//
// AddMessage
//
void Game::AddMessage(const char *str)
{
	TextMessage msg;
	
	msg.sTime = SDL_GetTicks();
	strcpy(msg.text, str);
	
	msgs.push_back(msg);
}

//
// GameOver
//
void Game::GameOver()
{
	framework->DrawTextCentered("Game Over!", Colors::red, true);
	SDL_Flip(framework->GetScreen());
	SDL_Delay(GAME_OVER_TIME);
	restartMe = true;
	framework->Exit = false;
}
