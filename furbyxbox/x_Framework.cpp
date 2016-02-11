/*
	FurbyKill 3D
	============
	License: GPL
	Author: Andre Schnabel
	E-Mail: thefrogs@web.de
	Homepage: http://www.semler-service.de/andre/
*/

// Framework.cpp

#include "../source/Framework.h"
#include "../source/State.h"
#include "../source/Player.h"
#include "../source/Game.h"
#include "x_Globals.h"

Framework *framework = NULL;
SDL_Joystick *joy;

int  PosH = 85;
bool UP = false;
int  PosW = 165;
bool LEFT = true;

//
// ToUint32
//
Uint32 Color::ToUint32()
{
	return SDL_MapRGB(framework->GetScreen()->format, r, g, b);
}

//
// Framework
// Constructor
//
Framework::Framework(FrameworkSetup *setup)
{
	//SCALE = 0.945;
	int result;

	Uint32 flags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_NOPARACHUTE, scrFlags = SDL_SWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN;

	char fullCaption[MAX_STR_LENGTH];
	
	Output("Initializing framework...");
	
	ownSetup = false;
	
	if(!setup)
	{
		setup = new FrameworkSetup;
		ownSetup = true;
	}
	
	this->setup = setup;
	noSound = setup->noSound;
	
	if(!noSound)
		flags |= SDL_INIT_AUDIO;
	
	result = SDL_Init(flags);
	if(result == -1)
		Error("Can't initialize the SDL multimedia interface!");
	
	if(setup->fullscreen)
		scrFlags |= SDL_FULLSCREEN;
	
	screen = SDL_SetVideoMode(setup->scrW, setup->scrH, setup->scrBpp, scrFlags);
	if(!screen)
		Error("Can't set video mode!");
	
	framework->xbox_SetScreenStretch( -(setup->scrW - (setup->scrW * SCALE)), -(setup->scrH - (setup->scrH * SCALE))); 
       
	framework->xbox_SetScreenPosition( (setup->scrW - (setup->scrW * SCALE)) / 2, (setup->scrH - (setup->scrH * SCALE)) / 2); 

	sprintf(fullCaption, "%s - BUILD: date(" __DATE__ ") time(" __TIME__ ")", setup->caption);
	
	SDL_WM_SetCaption(fullCaption, NULL);
	SDL_ShowCursor(SDL_FALSE);
	
	keys = SDL_GetKeyState(&numKeys);
	oldKeys = new Uint8[numKeys];
	memcpy(oldKeys, keys, sizeof(Uint8) * numKeys);

	magenta = SDL_MapRGB(screen->format, 255, 0, 255);
	black = SDL_MapRGB(screen->format, 0, 0, 0);
	
	TTF_Init();
	

    font = TTF_OpenFont(DATA_PREFIX "fonts\\font.ttf", setup->smallFontSize);
	if(!font)
		Error("Can't open font!");
	
	bigFont = TTF_OpenFont(DATA_PREFIX "fonts\\Kill_your_darlings.ttf", setup->bigFontSize);
	if(!bigFont)
		Error("Can't open bigFont!");
	
	frameTime = 0.0f;
	
	if(!noSound)
	{
		result = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024);
		if(result == -1)
			Error("Can't open audio!");
		
		Mix_VolumeMusic(MIX_MAX_VOLUME / 1);	
	}
	
	if(framework)
		Error("Only one framework instance allowed!");
	
	framework = this;
	
	Exit = true;

	Output("Framework initialized.");
}

//
// ~Framework
// Destructor
//
Framework::~Framework()
{
	list<State *>::iterator it;
	State *curState;
	
	Output("Shutting down framework...");
	
	if(!noSound)
		Mix_CloseAudio();
	
	TTF_CloseFont(bigFont);
	TTF_CloseFont(font);
	TTF_Quit();
	
	delete [] oldKeys;
	
	
	for(it = states.begin(); it != states.end(); it++)
	{
		curState = *it;
		delete curState;
	}
	
	if(setup->fullscreen)
	{
		SDL_ShowCursor(SDL_TRUE);
		SDL_WM_ToggleFullScreen(screen);
	}
	SDL_JoystickClose(joy);
	
	XLaunchNewImage(NULL, NULL);

	if(ownSetup)
		delete setup;
	
	Output("Framework is down.");
}

//
// Run
//
void Framework::Run()
{
	list<State *>::iterator it;
	State *curState;
	Uint32 sTime;
		
	allDone = false;
	
	for(it = states.begin(); it != states.end(); it++)
	{
		if(allDone)
			break;
		
		curState = *it;
restartLbl:	
		curState->Init();		
		curState->SetDone(false);
		curState->SetRestartMe(false);
		
		while(!curState->GetDone())
		{
			if(allDone)
				break;
			
			sTime = SDL_GetTicks();
			
			UpdateKeys();

			//JButtonState(NULL, NULL);

			curState->Input();
			
			curState->Update();
			
			if(curState->GetRestartMe())
			{
				curState->Quit();
				goto restartLbl;
			}
			
			curState->Draw();
			
			//SDL_Flip(screen);
			SDL_UpdateRect(screen,0 ,0 ,0, 0);
			
			if(SDL_GetTicks() - sTime < 1000 / MAX_FPS)
				SDL_Delay(1000 / MAX_FPS - (SDL_GetTicks() - sTime));
			
			frameTime = (float)(SDL_GetTicks() - sTime) / 1000.0f;
		}

		Exit = true;
		curState->Quit();
	}
}

//
// AddState
//
void Framework::AddState(State *state)
{
	states.push_back(state);
}

//
// Error
//
void Framework::Error(const char *msg)
{
	printf("Error: %s\n", msg);
	fflush(stdout);
	exit(1);
}

//
// Output
//
void Framework::Output(const char *msg)
{
	printf("Framework (\"%s\")\n", msg);
	//fflush(stdout);
}

//
// DrawPixel
//
bool Framework::DrawPixel(int x, int y, Uint32 color)
{
	Uint8 *ptr;
	
	if(x < 0 || y < 0 || x >= screen->w || y >= screen->h)
		return false;
	 
	ptr = (Uint8 *)screen->pixels + y * screen->pitch + x * (setup->scrBpp / 8);
	
	switch(setup->scrBpp)
	{
		case 16:
			*(Uint16 *)ptr = color;
			break;
		case 24:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
			{
				ptr[0] = (color >> 16) & 0xff;
				ptr[1] = (color >> 8) & 0xff;
				ptr[2] = color & 0xff;
			}
			else
			{
				ptr[0] = color & 0xff;
				ptr[1] = (color >> 8) & 0xff;
				ptr[2] = (color >> 16) & 0xff;
			}
			break;
		case 32:
			*(Uint32 *)ptr = color;
			break;
		default:
			Error("Please set a valid bits per pixel value!");
			return false;
	}
	
	return true;
}

//
// GetPixelColor
// Copied out of SDLDocs
//
Uint32 Framework::GetPixelColor(SDL_Surface *surf, int x, int y) const
{
	int bpp = surf->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	Uint8 *p;
	 
	if(x < 0 || y < 0 || x >= surf->w || y >= surf->h)
		return black;
	
	p = (Uint8 *)surf->pixels + y * surf->pitch + x * bpp;

	switch(bpp) {
		case 1:
			return *p;

		case 2:
			return *(Uint16 *)p;

		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
				return p[0] << 16 | p[1] << 8 | p[2];
			else
				return p[0] | p[1] << 8 | p[2] << 16;

		case 4:
			return *(Uint32 *)p;

		default:
			return 0;       /* shouldn't happen, but avoids warnings */
	}
}

//
// LoadImage
//
SDL_Surface *Framework::LoadImage(const char *filename, bool useAlpha)
{
	SDL_Surface *temp, *image;
	char errMsg[MAX_STR_LENGTH], completeFilename[MAX_STR_LENGTH];
	
	sprintf(completeFilename, "%s%s", DATA_PREFIX, filename);
	
	temp = IMG_Load(completeFilename);
	if(!temp)
	{
		sprintf(errMsg, "Unable to load image: %s!", completeFilename);
		Error(errMsg);
	}
	else
		printf("Framework::LoadImage: Success: %s!\n", completeFilename);
	
	image = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);
	
	if(useAlpha)
		SDL_SetColorKey(image, SDL_SRCCOLORKEY, magenta);
	
	return image;
}

//
// LockScreen
//
void Framework::LockScreen(void)
{
	if(SDL_MUSTLOCK(screen))
		SDL_LockSurface(screen);
}

//
// UnlockScreen
//
void Framework::UnlockScreen(void)
{
	if(SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);
}

//
// GetScreen
//
SDL_Surface *Framework::GetScreen()
{
	return screen;
}

//
// GetScrW
//
int Framework::GetScrW() const
{
	return screen->w;
}

//
// GetScrH
//
int Framework::GetScrH() const
{
	return screen->h;
}

//
// FillRect
//
void Framework::FillRect(SDL_Rect *rect, Uint32 color)
{
	SDL_FillRect(screen, rect, color);
}

//
// UpdateKeys
//
bool Framework::UpdateKeys()
{
	static SDL_Event event;
	
	memcpy(oldKeys, keys, sizeof(Uint8) * numKeys);
	int State;
	
	SDL_PumpEvents();
	SDL_JoystickUpdate();

	return State;
}

//
// KeyPressed
//
bool Framework::KeyPressed(int keyNum, bool wasReleased)
{
	return wasReleased ? keys[keyNum] && !oldKeys[keyNum] : keys[keyNum];
}
//Get Joypad Buttons
int Framework::GetJButton(int Index,int Button)
{
	joy = SDL_JoystickOpen(Index);
	return(SDL_JoystickGetButton(joy, Button));
}
//Get Current Axis Value
float Framework::GetAxis(int Index, int Axis)
{
	joy = SDL_JoystickOpen(Index);
	float x = SDL_JoystickGetAxis(joy, Axis);
	
	if(x > -8000 && x < 8000)
		x=0;

	return x;
}
//Get Always Positive Axis Value
float Framework::GetAxisAWPositive(int Index, int Axis)
{
	joy = SDL_JoystickOpen(Index);
	float x = SDL_JoystickGetAxis(joy, Axis);

	if(x < 0)
		x = x/12000*-1.0;
	else
		x = x/12000;
	
	return x;

}
//Get Joystick Button state
int Framework::JButtonState(int Index, int Which)
{
	static SDL_Event event;
	int State;

	while(SDL_PollEvent (&event))
	{
		switch(event.type)
		{
			case SDL_JOYBUTTONDOWN:
			{
				if(event.jbutton.button == Which)
					State = 0;
			}
			case SDL_JOYBUTTONUP:
			{
				if(event.jbutton.button == Which)
					State =	1;
			}
	
		}
		
	}

    return State;	
}
bool Framework::GetHatCentered(int Index)
{
	bool centered;

	joy = SDL_JoystickOpen(Index);

	if(SDL_JoystickGetHat(joy, SDL_HAT_CENTERED))
		centered = true;
	else
		centered = false;
	
	return centered;
}
int Framework::GetHatWhich(int Index, int WhichHat)
{
	int returnHat;
	//joy = SDL_JoystickOpen(Index);

	returnHat = SDL_JoystickGetHat(joy, WhichHat);

	return returnHat;
}
//
// GetColor
//
Uint32 Framework::GetColor(int r, int g, int b)
{
	return SDL_MapRGB(screen->format, r, g, b);
}

//
// GetMagenta
//
Uint32 Framework::GetMagenta() const
{
	return magenta;
}

//
// Draw
//
void Framework::Draw(SDL_Surface *image, int x, int y)
{
	static SDL_Rect rect;
	
	rect.w = image->w;
	rect.h = image->h;
	
	rect.x = x;
	rect.y = y;
	
	SDL_BlitSurface(image, NULL, screen, &rect);
}

//
// Draw
//
void Framework::Draw(SDL_Surface *image, SDL_Rect *srcRect, int x, int y)
{
	static SDL_Rect destRect;
	
	destRect.w = srcRect->w;
	destRect.h = srcRect->h;
	destRect.x = x;
	destRect.y = y;
	
	SDL_BlitSurface(image, srcRect, screen, &destRect);
}

//
// DrawCentered
//
void Framework::DrawCentered(SDL_Surface *image)
{
	static SDL_Rect rect;
	
	rect.w = image->w;
	rect.h = image->h;
	
	rect.x = (screen->w - image->w) / 2;
	rect.y = (screen->h - image->h) / 2;
	
	SDL_BlitSurface(image, NULL, screen, &rect);
}

//
// DrawText
//
void Framework::DrawText(const char *text, int x, int y, Color color, bool useBigFont)
{
	SDL_Surface *textSurf;
	textSurf = DrawTextToSurface(text, color, useBigFont);
	Draw(textSurf, x, y);
	SDL_FreeSurface(textSurf);
}

//
// DrawTextCentered
//
void Framework::DrawTextCentered(const char *text, Color color, bool useBigFont)
{
	SDL_Surface *textSurf;
	textSurf = DrawTextToSurface(text, color, useBigFont);
	DrawCentered(textSurf);
	SDL_FreeSurface(textSurf);
}

//
// DrawTextToSurface
//
SDL_Surface *Framework::DrawTextToSurface(const char *text, Color color, bool useBigFont)
{
	SDL_Surface *textSurf;
	TTF_Font *thisFont;
	
	if(!useBigFont)
		thisFont = font;
	else
		thisFont = bigFont;
	
	textSurf = TTF_RenderText_Blended(thisFont, text, color.ToSDLCol());
	
	return textSurf;
}

//
// CountRowsInFile
//
int Framework::CountRowsInFile(const char *filename)
{
	FILE *fp;
	char c, errMsg[MAX_STR_LENGTH];
	int rows = 0;
	
	fp = fopen(filename, "r");
	if(!fp)
	{
		sprintf(errMsg, "Can't load file: %s!", filename);
		framework->Error(errMsg);
	}
	
	while(!feof(fp))
	{
		c = fgetc(fp);
		
		if(c == '\n')
			rows++;
	}
	
	fclose(fp);
	
	return rows + 1;
}

//
// CountColsInFile
//
int Framework::CountColsInFile(const char *filename)
{
	FILE *fp;
	char c, errMsg[MAX_STR_LENGTH];
	int cols = 0;
	
	fp = fopen(filename, "r");
	if(!fp)
	{
		sprintf(errMsg, "Can't load file: %s!", filename);
		framework->Error(errMsg);
	}
	
	while(!feof(fp))
	{
		c = fgetc(fp);
		
		if(c != '\n')
			cols++;
		else
			break;
	}
	
	fclose(fp);
	
	return cols;
}

//
// GetMouseMovement
//
void Framework::GetMouseMovement(int *x, int *y)
{
	SDL_GetRelativeMouseState(x, y);
}

//
// FileExists
//
bool Framework::FileExists(const char *filename)
{
	FILE *fp;
	
	fp = fopen(filename, "r");
	if(!fp)
		return false;
	
	fclose(fp);
	
	return true;
}

//
// ShowFPS
//
void Framework::ShowFPS()
{
	static Uint32 lCall = SDL_GetTicks(), timeGone;
	static float curFps;
	static char showStr[MAX_STR_LENGTH];
	
	timeGone = SDL_GetTicks() - lCall;
	lCall = SDL_GetTicks();
	
	if(timeGone == 0)
		curFps = 1000.0f / 0.001f;
	else	
		curFps = 1000.0f / timeGone;

	sprintf(showStr, "FPS: %d", (int)curFps);
	
	framework->DrawText(showStr, screen->w - 100, 5, Colors::black);
}
void Framework::Debug(bool showdebug)
{
	static int joyaxisNr;
	static char joyaxisLXStr[MAX_STR_LENGTH], joyaxisLYStr[MAX_STR_LENGTH], joyaxisRXStr[MAX_STR_LENGTH], joyaxisRYStr[MAX_STR_LENGTH], joyaxisTRStr[MAX_STR_LENGTH], joynameStr[MAX_STR_LENGTH], joyaxisNrStr[MAX_STR_LENGTH], curweaponStr[MAX_STR_LENGTH];
	
	if(showdebug==true)
	{
		if(joy)
		{
			if(SDL_JoystickNumAxes(joy)<=4)
			{
				sprintf(joyaxisLXStr, "AxisLX: %f", GetAxisAWPositive(0, JOY_AXIS_LX));
				sprintf(joyaxisLYStr, "AxisLY: %f", GetAxisAWPositive(0, JOY_AXIS_LY));
				sprintf(joyaxisRXStr, "AxisRX: %f", GetAxis(0, JOY_AXIS_RX));
				sprintf(joyaxisRYStr, "AxisRY: %f", GetAxis(0, JOY_AXIS_RY));
				framework->DrawText(joyaxisLXStr, 0, 0, Colors::white, false);
				framework->DrawText(joyaxisLYStr, 0, 20, Colors::white, false);
				framework->DrawText(joyaxisRXStr, 0, 40, Colors::white, false);
				framework->DrawText(joyaxisRYStr, 0, 60, Colors::white, false);
			}
			if(SDL_JoystickNumAxes(joy)>=5)
			{
				sprintf(joyaxisLXStr, "AxisLX: %f", GetAxisAWPositive(0, JOY_AXIS_LX));
				sprintf(joyaxisLYStr, "AxisLY: %f", GetAxisAWPositive(0, JOY_AXIS_LY));
				sprintf(joyaxisRXStr, "AxisRX: %f", GetAxis(0, JOY_AXIS_RX));
				sprintf(joyaxisRYStr, "AxisRY: %f", GetAxis(0, JOY_AXIS_RY));
				framework->DrawText(joyaxisLXStr, 0, 0, Colors::white, false);
				framework->DrawText(joyaxisLYStr, 0, 20, Colors::white, false);
				framework->DrawText(joyaxisRXStr, 0, 40, Colors::white, false);
				framework->DrawText(joyaxisRYStr, 0, 60, Colors::white, false);
				sprintf(joyaxisTRStr, "AxisTR: %f", GetAxis(0, 2));
				framework->DrawText(joyaxisTRStr, 0, 80, Colors::white, false);
			}
			sprintf(joynameStr, "Joypad Name: %s", SDL_JoystickName(0));
			sprintf(joyaxisNrStr, "Axis: %i", SDL_JoystickNumAxes(joy));

			framework->DrawText(joynameStr, 0, 100, Colors::white, false);
			framework->DrawText(joyaxisNrStr, 0, 120, Colors::white, false);
		}

		sprintf(curweaponStr,"CurWeapon: %i",&Player::GetWeapon);
		framework->DrawText(curweaponStr, 0, 140, Colors::white, false);
	}
	
}

//
// GetFrameTime
//
float Framework::GetFrameTime()
{
	return frameTime;
}

//
// ButtonPressed
//
bool Framework::ButtonPressed(MouseButton button)
{
	return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button);
}

//
// LoadMusic
//
Mix_Music *Framework::LoadMusic(const char *filename)
{
	Mix_Music *mus;
	char errMsg[MAX_STR_LENGTH], completeFilename[MAX_STR_LENGTH];
	
	if(noSound)
		return NULL;
	
	sprintf(completeFilename, "%s%s", DATA_PREFIX, filename);
	
	mus = Mix_LoadMUS(completeFilename);
	if(!mus)
	{
		sprintf(errMsg, "Can't load music: %s!\n", completeFilename);
		Error(errMsg);
	}
	else
		printf("Framework::LoadMusic: Success: %s!\n", completeFilename);
	
	return mus;
}

//
// LoadSound
//
Mix_Chunk *Framework::LoadSound(const char *filename)
{
	Mix_Chunk *snd;
	char errMsg[MAX_STR_LENGTH], completeFilename[MAX_STR_LENGTH];
	
	if(noSound)
		return NULL;
	
	sprintf(completeFilename, "%s%s", DATA_PREFIX, filename);
	
	snd = Mix_LoadWAV(completeFilename);
	if(!snd)
	{
		sprintf(errMsg, "Can't load sound: %s!\n", completeFilename);
		Error(errMsg);
	}
	else
		printf("Framework::LoadSound: Success: %s!\n", completeFilename);
	
	return snd;
}

//
// Coll
//
bool Framework::Coll(SDL_Rect *rect1, SDL_Rect *rect2)
{
	return (	rect1->x + rect1->w > rect2->x
			&&	rect1->x < rect2->x + rect2->w
			&& 	rect1->y + rect1->h > rect2->y
			&& 	rect1->y < rect2->y + rect2->h);
}

//
// PlaySound
//
int Framework::PlaySound(Mix_Chunk *sound, int loops)
{
	if(noSound)
		return -1;
	return  Mix_PlayChannel(-1, sound, loops);;
}

//
// PlayMusic
//
void Framework::PlayMusic(Mix_Music *music, int loops)
{
	if(noSound)
		return;
	
	Mix_PlayMusic(music, loops);
}

//
// GetNoSound
//
bool Framework::GetNoSound() const
{
	return noSound;
}

void Framework::GlobalSounds() 
{
	EnemyShoot = LoadSound("sounds\\EnemyShoot.ogg");
	ClosingDoor = LoadSound("sounds\\ClosingDoor.ogg");
	OpeningDoor = LoadSound("sounds\\OpeningDoor.ogg");

}
int Framework::ArmMovmeantH()
{
	if(GetAxis(0, 0)||GetAxis(0, 1)||KeyPressed(SDLK_UP, false)||KeyPressed(SDLK_DOWN, false)||KeyPressed(SDLK_w, false)||KeyPressed(SDLK_a, false)||KeyPressed(SDLK_s, false)||KeyPressed(SDLK_d, false)||KeyPressed(SDLK_LEFT, false)&&KeyPressed(SDLK_LALT, false)||KeyPressed(SDLK_RIGHT, false)&&KeyPressed(SDLK_LALT, false))
	{	
		if(PosH<95&&UP == true)
		{
			PosH++;
			if(PosH==95||PosH>95)
			{
				UP = false;
				return PosH;
			}
			return PosH;
		}
		if(PosH>75&&UP == false)
		{
			PosH--;
			if(PosH==75||PosH<75)
			{
				UP = true;
				return PosH;
			}
			return PosH;
		}
	}
	return PosH;
}

int Framework::ArmMovmeantW()
{
	if(GetAxis(0, 0)||GetAxis(0, 1)||KeyPressed(SDLK_UP, false)||KeyPressed(SDLK_DOWN, false)||KeyPressed(SDLK_w, false)||KeyPressed(SDLK_a, false)||KeyPressed(SDLK_s, false)||KeyPressed(SDLK_d, false)||KeyPressed(SDLK_LEFT, false)&&KeyPressed(SDLK_LALT, false)||KeyPressed(SDLK_RIGHT, false)&&KeyPressed(SDLK_LALT, false))
	{	
		if(PosW>152&&LEFT == false)
		{
			PosW--;
			if(PosW==152||PosW<152)
			{
				LEFT = true;
				return PosW;
			}
			return PosW;
		}
		if(PosW<168&&LEFT == true)
		{
			PosW++;
			if(PosW==168||PosW>168)
			{
				LEFT = false;
				return PosW;
			}
			return PosW;
		}
	}
	return PosW;
}
#ifdef _XBOX
int Framework::xbox_SetScreenPosition(float x, float y)
{
	float x2, y2;

	SDL_XBOX_SetScreenPosition(x, y);
	SDL_XBOX_GetScreenPosition(&x2, &y2);

	if(x != x2 || y != y2)
		return -1;
	return 0;
}
int Framework::xbox_SetScreenStretch(float xs, float ys)
{
	float xs2, ys2;

	SDL_XBOX_SetScreenStretch(xs, ys);
	SDL_XBOX_GetScreenStretch(&xs2, &ys2);

	if(xs != xs2 || ys != ys2)
		return -1;
	return 0;
}
#endif
	
