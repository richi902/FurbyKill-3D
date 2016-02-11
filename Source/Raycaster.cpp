/*
	FurbyKill 3D
	============
	License: GPL
	Author: Andre Schnabel
	E-Mail: thefrogs@web.de
	Homepage: http://www.semler-service.de/andre/
*/

// Raycaster.cpp

#include "Raycaster.h"
#include "Sprite.h"
#include "Door.h"
#include "Furby.h"
#include <string.h>

//
// Raycaster
// Constructor
//
Raycaster::Raycaster(RaycasterSetup *setup)
{
	int i;

	posX = 0.0f;
	posY = 0.0f;

	alpha = 0.0f;

	lookDir.x = 1.0f;
	lookDir.y = 0.0f;

	camPlane.x = 0.0f;
	camPlane.y = COS(FOV);

	target = setup->target;
	lowQuality = setup->lowQuality;
	badQuality = setup->badQuality;

	numTextures = setup->numTextures;

	for(i=0; i<numTextures; i++)
	{
		textures[i][0] = setup->textures[i][0];
		textures[i][1] = setup->textures[i][1];
	}

	LoadMap(setup->mapFilename);

	sprites = setup->sprites;

	roofColor = framework->GetColor(50, 50, 200);
	floorColor = framework->GetColor(50, 50, 50);
#ifdef GEKKO
	if(!lowQuality)
	{
		skyImg = framework->LoadImage("images/sky.bmp", false);
		roofImg = framework->LoadImage("images/roof.bmp", false);
	}
#elif _WIN32
	if(!lowQuality)
	{
		skyImg = framework->LoadImage("images\\sky.bmp", false);
		roofImg = framework->LoadImage("images\\roof.bmp", false);
	}
#elif _XBOX
    if(!lowQuality)
	{
		skyImg = framework->LoadImage("images\\sky.bmp", false);
		roofImg = framework->LoadImage("images\\roof.bmp", false);
	}
#elif linux
    if(!lowQuality)
    {
        skyImg = framework->LoadImage("images/sky.bmp", false);
        roofImg = framework->LoadImage("images/roof.bmp", false);
    }
#endif
	else
	{
		skyImg = framework->LoadImage("images\\lowQuality\\sky.bmp", false);
		roofImg = framework->LoadImage("images\\lowQuality\\roof.bmp", false);
	}

	upperRect.x = upperRect.y = lowerRect.x = 0;
	upperRect.w = lowerRect.w = target->w;
	upperRect.h = lowerRect.h = target->w / 2;
	lowerRect.y = target->h / 2;

	zBuffer = new float[target->w];

	pplaneDist = (float)(target->w / 2) / TAN(FOV / 2);

	crosshairRect = setup->crosshairRect;

	CORR_ANGLE(alpha);
}

//
// ~Raycaster
// Destructor
//
Raycaster::~Raycaster()
{
	doors.clear();

	SDL_FreeSurface(skyImg);
	SDL_FreeSurface(roofImg);

	delete [] zBuffer;
}

//
// ClearMap
//
void Raycaster::ClearMap()
{
	int i, j;

	for(i=0; i<MAX_MAP_H; i++)
		for(j=0; j<MAX_MAP_W; j++)
			map[j][i] = 0;
}

#define IS_VALID_CHAR(a) \
	((a >= '0' && a <= '9') \
	|| (a >= 'A' && a <= 'Z') \
	|| (a >= 'a' && a <= 'z')) \

//
// LoadMap
//
void Raycaster::LoadMap(const char *filename)
{
	int i, j;
	FILE *fp;
	char c, errMsg[MAX_STR_LENGTH];

	mapChange = false;

	ClearMap();

	doors.clear();

	mapH = framework->CountRowsInFile(filename);
	mapW = framework->CountColsInFile(filename);

	fp = fopen(filename, "r");
	if(!fp)
	{
		sprintf(errMsg, "Can't load file: %s!", filename);
		framework->Error(errMsg);
	}

	for(i=0; i<mapH; i++)
	{
		for(j=0; j<mapW; j++)
		{
			c = fgetc(fp);

			while(!IS_VALID_CHAR(c) && !feof(fp))
				c = fgetc(fp);

			if(feof(fp) || !IS_VALID_CHAR(c))
				break;

			if(c >= '0' && c <= '9')
				map[i][j] = c - '0';
			else if(c >= 'A' && c <= 'Z')
				map[i][j] = c - 'A' + 10;
			else if(c >= 'a' && c <= 'z')
			{
				if(c == 'z')
				{
					posX = j + 0.5f;
					posY = i + 0.5f;
					map[i][j] = 0;
				}
				else
					map[i][j] = c - 'a' + 36;
			}

			if(map[i][j] == DOOR_INDEX)
			{
				Door door(j, i, this);
				doors.push_back(door);
			}
			else if(map[i][j] == LOCKED_DOOR_INDEX)
			{
				Door door(j, i, this, true);
				doors.push_back(door);
			}
		}
	}

	fclose(fp);
}

//
// UpdateDoors
//
void Raycaster::UpdateDoors()
{
	static list<Door>::iterator it;

	for(it = doors.begin(); it != doors.end(); it++)
	{
		(*it).Update(posX, posY);
	}
}

//
// Draw
//
void Raycaster::Draw()
{
	float angle;
	float horizDist, vertDist, distance;
	int wallH, x, texIndex, texIndexHor, texIndexVert;
	float texelX, texelXHor, texelXVert;
	bool dark;

	UpdateDoors();

	if(badQuality)
	{
		framework->FillRect(&upperRect, roofColor);
		framework->FillRect(&lowerRect, floorColor);
	}
	else
	{
		framework->Draw(skyImg);
		framework->Draw(roofImg, 0, skyImg->h);
	}

	framework->LockScreen();

	angle = alpha + FOV / 2;

	for(x = 0; x < target->w; x++)
	{
		if(angle < 0)
			angle += 360;
		else if(angle > 360)
			angle -= 360;

		if(angle == 90 || angle == 0 || angle == 270)
			angle += 0.00001f;

		horizDist = CastThroughIntersections(angle, ID_HORIZONTAL, &texIndexHor, &texelXHor);
		vertDist = CastThroughIntersections(angle, ID_VERTICAL, &texIndexVert, &texelXVert);

		if(horizDist < vertDist)
		{
			texIndex = texIndexHor;
			distance = horizDist;
			texelX = texelXHor;
			dark = true;
		}
		else
		{
			texIndex = texIndexVert;
			distance = vertDist;
			texelX = texelXVert;
			dark = false;
		}

		distance = CorrDistance(distance, angle);
		zBuffer[x] = distance;
		wallH = DistToWallHeight(distance);

		texelX *= (float)TEXTURE_W;
		texelX = TEXTURE_W - texelX;

		DrawWallSlice(x, wallH, texIndex, texelX, dark);

		if(badQuality)
		{
			x++;
			zBuffer[x] = distance;
			DrawWallSlice(x, wallH, texIndex, texelX, dark);
			angle -= (float)FOV / (float)target->w * 2.0f;
		}
		else
			angle -= (float)FOV / (float)target->w;
	}

	DrawSprites();

	framework->UnlockScreen();
}

//
// DrawSprites
//
// Note: My version of the calculations for the sprite distance and x-coord using only trigonometric functions
// was giving pretty ugly results. Of course it was working, but in small rooms you could see some wrong clipping
// and other messy things. Therefore I replaced these calculations by others using vectors and matrizes in order
// to get the right values for the x-coord of the projected sprite and the distance for correct clipping. And
// I have to say that this solution is very much based on what Lode Vandevenne explains in his raycasting tutorial.
//
void Raycaster::DrawSprites()
{
	static SDL_Rect sprRect;

	int i, j, startX, startY;
	float distance, height;
	float a, b;

	list<Sprite *>::iterator sprIt;
	list<SprDraw>::iterator it;
	SprDraw curSpr;
	Sprite *anotherSpr;
	Uint32 color;
	bool partlyDrawn = false;

	float sprX, sprY;
	float yStepWidth, xStepWidth;

	//float beta;
	float inverse, transX, transY;

	// Put sprites into list for sorting
	for(sprIt = sprites->begin(); sprIt != sprites->end(); sprIt++)
	{
		anotherSpr = (*sprIt);

		SprDraw spr;

		spr.x = anotherSpr->GetPosX();
		spr.y = anotherSpr->GetPosY();

		spr.original = anotherSpr;

		a = spr.x - posX;
		b = spr.y - posY;

		distance = sqrt(a*a+b*b);

		spr.image = anotherSpr->GetImage();

		spr.distance = distance;

		sprsToDraw.push_back(spr);
	}

	// Sort from far away to near
	sprsToDraw.sort();

	// Draw them from the back
	for(it = sprsToDraw.begin(); it != sprsToDraw.end(); it++)
	{
		// Draw sprite
		curSpr = *it;

		a = curSpr.x - posX;
		b = curSpr.y - posY;

		inverse = 1.0f / (camPlane.x * lookDir.y - lookDir.x * camPlane.y);
		transX = inverse * (lookDir.y * a - lookDir.x * b);
		transY = inverse * (-camPlane.y * a + camPlane.x * b);

		height = DistToWallHeight(transY) * 1.0f;

		startX = (int)(-height / 2 + (target->w / 2) * (1 + transX / transY));

		startY = (target->h - (int)height) / 2;

		sprX = sprY = 0;

		xStepWidth = (float)SPRITE_WIDTH / (float)height;
		yStepWidth = (float)SPRITE_HEIGHT / (float)height;

		if(startX + height < 0 || startX >= target->w || curSpr.distance < SPR_NEAR_CLIP_DIST)
			continue;

		partlyDrawn = false;

		// Horizontal
		for(i=0; i<height; i++)
		{
			if(startX + i < 0 || startX + i >= target->w || transY > zBuffer[i+startX])
			{
				sprX += xStepWidth;
				continue;
			}

			// Vertical
			for(j=0; j<height; j++)
			{
				if(!badQuality || j % 4 == 0)
					color = framework->GetPixelColor(curSpr.image, (int)sprX, (int)sprY);

				if(color != framework->GetMagenta())
					partlyDrawn |= framework->DrawPixel(startX + i, startY + j, color);

				sprY += yStepWidth;
			}

			sprY = 0;
			sprX += xStepWidth;
		}

		sprRect.w = (int)height;
		sprRect.x = startX;
		sprRect.h = (int)height;
		sprRect.y = startY;

		if(partlyDrawn && framework->Coll(&sprRect, crosshairRect))
			curSpr.original->SetLookingAt(true);
		else
			curSpr.original->SetLookingAt(false);

		if(curSpr.original->GetId() == ET_FURBY)//||!curSpr.original->GetId() == ET_SKULL)
		{
			Furby *fb = (Furby *)curSpr.original;
			fb->SetVisible(partlyDrawn);
		}

		if(curSpr.original->GetId() == ET_SKULL)//||!curSpr.original->GetId() == ET_FURBY);
		{
			Skull *sk = (Skull *)curSpr.original;
			sk->SetVisible(partlyDrawn);
		}

	}

	sprsToDraw.clear();
}

//
// SetPosX
//
void Raycaster::SetPosX(float x)
{
	posX = x;
}

//
// SetPosY
//
void Raycaster::SetPosY(float y)
{
	posY = y;
}

//
// GetPosX
//
float Raycaster::GetPosX() const
{
	return posX;
}

//
// GetPosY
//
float Raycaster::GetPosY() const
{
	return posY;
}

//
// SetPosition
//
void Raycaster::SetPosition(float x, float y)
{
	posX = x;
	posY = y;
}

//
// GetPosition
//
void Raycaster::GetPosition(float *x, float *y) const
{
	*x = posX;
	*y = posY;
}

//
// SetAlpha
//
void Raycaster::SetAlpha(float alpha)
{
	CORR_ANGLE(alpha);
	camPlane.Rotate(alpha - this->alpha);
	lookDir.Rotate(alpha - this->alpha);
	this->alpha = alpha;
}

//
// GetAlpha
//
float Raycaster::GetAlpha() const
{
	return alpha;
}

//
// Rotate
//
void Raycaster::Rotate(float angle)
{
	angle *= framework->GetFrameTime();
	alpha += angle;

	CORR_ANGLE(alpha);

	camPlane.Rotate(angle);
	lookDir.Rotate(angle);
}

#define CHECKS() \
	map[(int)posY][(int)posX] > 0 \
	|| map[(int)(posY+MIN_WALL_DIST)][(int)(posX+MIN_WALL_DIST)] > 0 \
	|| map[(int)(posY-MIN_WALL_DIST)][(int)(posX-MIN_WALL_DIST)] > 0 \
	|| map[(int)posY][(int)(posX+MIN_WALL_DIST)] > 0 \
	|| map[(int)(posY+MIN_WALL_DIST)][(int)posX] > 0 \
	|| map[(int)(posY-MIN_WALL_DIST)][(int)posX] > 0 \
	|| map[(int)(posY)][(int)(posX-MIN_WALL_DIST)] > 0 \

#define CHECKS_X() \
	map[(int)oldPosY][(int)posX] > 0 \
	|| map[(int)(oldPosY+MIN_WALL_DIST)][(int)(posX+MIN_WALL_DIST)] > 0 \
	|| map[(int)(oldPosY-MIN_WALL_DIST)][(int)(posX-MIN_WALL_DIST)] > 0 \
	|| map[(int)oldPosY][(int)(posX+MIN_WALL_DIST)] > 0 \
	|| map[(int)(oldPosY+MIN_WALL_DIST)][(int)posX] > 0 \
	|| map[(int)(oldPosY-MIN_WALL_DIST)][(int)posX] > 0 \
	|| map[(int)(oldPosY)][(int)(posX-MIN_WALL_DIST)] > 0 \

#define CHECKS_Y() \
	map[(int)posY][(int)oldPosX] > 0 \
	|| map[(int)(posY+MIN_WALL_DIST)][(int)(oldPosX+MIN_WALL_DIST)] > 0 \
	|| map[(int)(posY-MIN_WALL_DIST)][(int)(oldPosX-MIN_WALL_DIST)] > 0 \
	|| map[(int)posY][(int)(oldPosX+MIN_WALL_DIST)] > 0 \
	|| map[(int)(posY+MIN_WALL_DIST)][(int)oldPosX] > 0 \
	|| map[(int)(posY-MIN_WALL_DIST)][(int)oldPosX] > 0 \
	|| map[(int)(posY)][(int)(oldPosX-MIN_WALL_DIST)] > 0 \

#define DECLS() \
	static float movX, movY; \
	static float oldPosX, oldPosY;

#define DO_CLIPPING() \
	if(CheckCollWithSprite(wasPickup, pickupId)) \
	{ \
		posX = oldPosX; \
		posY = oldPosY; \
	} \
	if(CHECKS_X()) \
		posX = oldPosX; \
	if(CHECKS_Y()) \
		posY = oldPosY;

/*if(CHECKS())
{
	posX = oldPosX;
	posY = oldPosY;
}*/

#define SAVE_OLD_POS() \
	oldPosX = posX; \
	oldPosY = posY;

//
// MoveForward
//
void Raycaster::MoveForward(float distance, bool *wasPickup, int *pickupId)
{
	DECLS();

	movY = -SIN(alpha) * distance * framework->GetFrameTime();
	movX = COS(alpha) * distance * framework->GetFrameTime();

	SAVE_OLD_POS();

	posX += movX;
	posY += movY;

	DO_CLIPPING();
}

//
// MoveBackward
//
void Raycaster::MoveBackward(float distance, bool *wasPickup, int *pickupId)
{
	DECLS();

	movY = -SIN(alpha) * distance * framework->GetFrameTime();
	movX = COS(alpha) * distance * framework->GetFrameTime();

	SAVE_OLD_POS();

	posX -= movX;
	posY -= movY;

	DO_CLIPPING();
}

//
// StepRight
//
void Raycaster::StepRight(float distance, bool *wasPickup, int *pickupId)
{
	DECLS();
	static float movDir;

	movDir = alpha - 90.0f;

	movX = COS(movDir) * distance * framework->GetFrameTime();
	movY = -SIN(movDir) * distance * framework->GetFrameTime();

	SAVE_OLD_POS();

	posX += movX;
	posY += movY;

	DO_CLIPPING();
}

//
// StepLeft
//
void Raycaster::StepLeft(float distance, bool *wasPickup, int *pickupId)
{
	DECLS();
	static float movDir;

	movDir = alpha - 90.0f;

	movX = COS(movDir) * distance * framework->GetFrameTime();
	movY = -SIN(movDir) * distance * framework->GetFrameTime();

	SAVE_OLD_POS();

	posX -= movX;
	posY -= movY;

	DO_CLIPPING();
}

//
// OpenDoor
//
void Raycaster::OpenDoor(bool hasKey, bool *needAKey)
{
	static int doorPosX, doorPosY;
	static Uint32 lDoorOpen = 0;
	int i;

	// Don't close doors we're standing in
	if(map[(int)posY][(int)posX] == DOOR_INDEX)
		return;

	for(i=0; i<4; i++)
	{
		switch(i)
		{
			case 0:
				doorPosX = (int)posX;
				doorPosY = (int)posY + 1;
				break;
			case 1:
				doorPosX = (int)posX + 1;
				doorPosY = (int)posY;
				break;
			case 2:
				doorPosX = (int)posX;
				doorPosY = (int)posY - 1;
				break;
			case 3:
				doorPosX = (int)posX - 1;
				doorPosY = (int)posY;
				break;
		}

		if(doorPosX < 0 || doorPosY < 0)
			continue;

		if(map[doorPosY][doorPosX] == DOOR_INDEX || map[doorPosY][doorPosX] == LOCKED_DOOR_INDEX)
		{
			if(lDoorOpen == 0)
				lDoorOpen = SDL_GetTicks();
			else if(SDL_GetTicks() - lDoorOpen < DOOR_CHG_TIME)
				continue;

			Door *door = GetDoorAt(doorPosX, doorPosY);
			//door->SetOpen(!door->IsOpen());

			if(!door->IsLockedDoor() || hasKey)
			{
				if(!door->IsOpen())
					door->SetOpening(true);
				else
					door->SetClosing(true);

				lDoorOpen = SDL_GetTicks();
			}
			else if(door->IsLockedDoor())
				*needAKey = true;

			return;
		}
		else if(map[doorPosY][doorPosX] == EXIT_DOOR_INDEX)
			mapChange = true;
	}
}

//
// CastThroughIntersections
//
float Raycaster::CastThroughIntersections(float angle, IntersectionDir dir, int *texIndex, float *texelX)
{
	float fx, fy;
	float dx, dy;
	float distance;
	float a, b;
	int mapX, mapY;

	switch(dir)
	{
		case ID_HORIZONTAL:

			if(UP(angle))
			{
				fy = -(posY - (int)posY);
				dy = -1;
			}
			else
			{
				fy = (int)posY + 1 - posY;
				dy = 1;
			}

			fx = (float)(ABS(fy)) / (float)(TAN(angle));
			dx = (float)(ABS(dy)) / (float)(TAN(angle));

			fx = ABS(fx);
			dx = ABS(dx);

			if(LEFT(angle))
			{
				dx = -dx;
				fx = -fx;
			}

			fx = posX + fx;
			fy = posY + fy;

			break;

		case ID_VERTICAL:

			if(LEFT(angle))
			{
				fx = -(posX - (int)posX);
				dx = -1;
			}
			else
			{
				fx = (int)posX + 1 - posX;
				dx = 1;
			}

			fy = (float)(TAN(angle)) * (float)(ABS(fx));
			dy = (float)(TAN(angle)) * (float)(ABS(dx));

			fy = ABS(fy);
			dy = ABS(dy);

			if(UP(angle))
			{
				fy = -fy;
				dy = -dy;
			}

			fx = posX + fx;
			fy = posY + fy;

			break;
	}

	while(true)
	{
		mapY = (int)fy;
		mapX = (int)fx;

		if(dy == -1 && dir == ID_HORIZONTAL)
			mapY -= 1;
		else if(dx == -1 && dir == ID_VERTICAL)
			mapX -= 1;

		if(mapX < 0 || mapY < 0 || mapX >= mapW || mapY >= mapH)
			break;
		else if(map[mapY][mapX] > 0 && map[mapY][mapX] != DOOR_INDEX && map[mapY][mapX] != LOCKED_DOOR_INDEX)
		{
hit:
			if(dir == ID_HORIZONTAL)
				*texelX = fx - (float)mapX;
			else
				*texelX = fy - (float)mapY;

			*texIndex = map[mapY][mapX] - 1;
			break;
		}
		else if(map[mapY][mapX] == DOOR_INDEX || map[mapY][mapX] == LOCKED_DOOR_INDEX)
		{
			Door *door = GetDoorAt(mapX, mapY);

			if(door->GetOpening() || door->GetClosing())
			{
				float xval;

				if(dir == ID_HORIZONTAL)
					xval = fx - (float)mapX;
				else
					xval = fy - (float)mapY;

				if(door->GetOpenedWidth() < xval)
					goto hit;
			}
			else if(!door->IsOpen())
				goto hit;
		}

		fx += dx;
		fy += dy;
	}

	a = ABS((fy - posY));
	b = ABS((fx - posX));

	distance = sqrt(a*a+b*b);

	return distance;
}

//
// CorrDistance
//
float Raycaster::CorrDistance(float distance, float angle)
{
	float beta = ABS((angle - alpha));
	return COS(beta) * distance;
}

//
// DistToWallHeight
//
int Raycaster::DistToWallHeight(float distance)
{
	return (int)((float)WALL_HEIGHT / distance * pplaneDist);
}

//
// DrawWallSlice
//
void Raycaster::DrawWallSlice(int x, int height, int texNum, float texelX, bool dark)
{
	int startY, i;
	float y, texYStep;
	Uint32 color;
	SDL_Surface *texture;

	texYStep = (float)TEXTURE_H / (float)height;

	startY = (target->h - height) / 2;
	y = 0;

	if(texNum >= numTextures || texNum < 0)
		return;

	if(dark)
		texture = textures[texNum][0];
	else
		texture = textures[texNum][1];

	for(i=0; i<height; i++)
	{
		// Skip reading some pixel in low quality mode
		if(!badQuality || i % 4 == 0)
			color = framework->GetPixelColor(texture, (int)texelX, (int)y);

		framework->DrawPixel(x, startY + i, color);
		y += texYStep;
	}
}

//
// GetDoorAt
//
Door *Raycaster::GetDoorAt(int x, int y)
{
	list<Door>::iterator it;
	static int itsX, itsY;
	Door *door;

	for(it = doors.begin(); it != doors.end(); it++)
	{
		door = &(*it);
		door->GetPos(&itsX, &itsY);

		if(itsX == x && itsY == y)
			return door;
	}

	printf("No door at pos: %d, %d!\n ", x, y);
	framework->Error("Fatal!");
	return NULL;
}

//
// IsOpenDoor
// This function is pretty obsolete now, since doors get replaced by zeros in the
// map when open and with DOOR_INDEX when closed
//
bool Raycaster::IsOpenDoor()
{
	static int doorX, doorY;
	int i;

	for(i=0; i<1; i++)
	{
		switch(i)
		{
			case 0:
				doorX = (int)posX;
				doorY = (int)posY;
				break;
			case 1:
				doorX = (int)(posX+MIN_WALL_DIST);
				doorY = (int)(posY+MIN_WALL_DIST);
				break;
			case 2:
				doorX = (int)(posX-MIN_WALL_DIST);
				doorY = (int)(posY-MIN_WALL_DIST);
				break;
			case 3:
				doorX = (int)(posX+MIN_WALL_DIST);
				doorY = (int)posY;
				break;
			case 4:
				doorX = (int)posX;
				doorY = (int)(posY+MIN_WALL_DIST);
				break;
			case 5:
				doorX = (int)posX;
				doorY = (int)(posY-MIN_WALL_DIST);
				break;
			case 6:
				doorX = (int)(posX-MIN_WALL_DIST);
				doorY = (int)(posY);
				break;
		}

		if(map[doorY][doorX] == DOOR_INDEX)
		{
			Door *door = GetDoorAt(doorX, doorY);
			if(door->IsOpen() && !door->GetClosing())
				return true;
		}
	}

	return false;
}

//
// CheckCollWithSprite
//
bool Raycaster::CheckCollWithSprite(bool *wasPickup, int *pickupId)
{
	list<Sprite *>::iterator it;
	Sprite *spr;

	*wasPickup = false;

	for(it = sprites->begin(); it != sprites->end(); it++)
	{
		spr = *it;
		if(spr->PointCollWith(posX, posY) && spr->GetId() != CORPSE_INDEX)
		{
			if(spr->GetId() >= PICKUP_INDEX && spr->GetId() < ET_SKULL)
			{
				*wasPickup = true;
				*pickupId = spr->GetId();
				delete spr;
				it = sprites->erase(it);
				return false;
			}

			return true;
		}
	}

	return false;
}

//
// GetMapValueAt
//
int Raycaster::GetMapValueAt(int x, int y)
{
	if(x >= MAX_MAP_W || y >= MAX_MAP_H || x < 0 || y < 0)
		framework->Error("Ugly map index!");

	return map[y][x];
}

//
// MapChangeNeeded
//
bool Raycaster::MapChangeNeeded()
{
	return mapChange;
}

//
// XCoordToAngle
//
float Raycaster::XCoordToAngle(int x)
{
	return ((float)target->w / 2.0f) - (float)x * ((float)FOV / (float)target->w) + alpha;
}

//
// AngleToXCoord
//
int Raycaster::AngleToXCoord(float angle)
{
	return (int)((float)target->w / 2.0f - (angle - alpha) * ((float)target->w / 2.0f) / ((float)FOV / 2.0f));
}

//
// SetMapValueAt
//
void Raycaster::SetMapValueAt(int x, int y, int value)
{
	if(x < 0 || y < 0 || x >= mapW || y >= mapH)
		return;

	map[y][x] = value;
}
