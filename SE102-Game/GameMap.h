
#pragma once
#ifndef __GAME_MAP__
#define __GAME_MAP__

#include <d3dx9.h>
#include <d3d9.h>
#include <vector>
#include "TextureManager.h"
#include "Sprite.h"
#include "MapReader/Tmx.h.in"
#include "Utils.h"
#include "GameObject.h"
//#include "GameGlobal.h"

class GameMap
{
public:
    GameMap(const char* filePath, std::vector<LPGAMEOBJECT>* listObjects);

    Tmx::Map* GetMap();
    
    int GetWidth();
    int GetHeight();
    int GetTileWidth();
    int GetTileHeight();

    

    void Draw();

    ~GameMap();

private:
    void LoadMap(const char* filePath);
    std::vector<LPGAMEOBJECT>* listObjects;
    std::list<int> listNoCollision = {40 };
    Tmx::Map* mMap;
    std::vector<int> mListTilesetId;
    bool hasLoaded = false;
    bool isExistInList(int a);
};

#endif

