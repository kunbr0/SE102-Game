#pragma once
#include "Game.h"
#include "TextureManager.h"
#include "Scene.h"
#include "GameObject.h"
#include "Mario.h"
#include "Effect.h"
#include <map>

//#include "GameMap2.h"

class CPlayScene : public CScene
{

protected:
	LPGAMEOBJECT player; // Player controls the position of camera.
	int playerLevel = 1;
	//shared_ptr<CGameMap> kMap;
	vector<LPGAMEOBJECT> staticObjects;
	vector<LPGAMEOBJECT> dynamicObjects;
	vector<LPGAMEOBJECT> dynamicObjectsBehindMap;
	vector<LPGAMEOBJECT> mainObjects;
	vector<LPGAMEOBJECT> tempObjects;
	vector<CEffect*> effects;

	vector<LPGAMEOBJECT> enemyBullets;

	//std::map<std::string, Vector2> miniPortals;

	float standingY;


public:
	CPlayScene(std::string id, std::string, std::string);
	void SwitchPlayer(LPGAMEOBJECT newPlayer);
	virtual void Load();
	virtual void Update(DWORD dt);
	virtual void Render();
	virtual void Unload();

	virtual bool LoadDataFromFile();
	/*virtual bool ParseTextures();
	virtual bool ParseSprites();*/

	vector<LPGAMEOBJECT> GetEnemyBullets() { return enemyBullets; }

	LPGAMEOBJECT GetPlayer() { return player; }
	void SetPlayer(LPGAMEOBJECT obj) { player = obj; }
	void PushDynamicObjects(LPGAMEOBJECT obj) { dynamicObjects.push_back(obj); }
	void PushTempObjects(LPGAMEOBJECT obj) { tempObjects.push_back(obj); }
	void PushEffects(CEffect* eff) { effects.push_back(eff); }

	void HandleSceneTime(DWORD&);

	void UpdateEffects(DWORD);
	void HandleMarioDie();

	CMario* GenerateMario(MarioType, Vector2);

	void ChangeCameraArea(Vector2, Vector2, Vector2);


	//friend class CPlayScenceKeyHandler;
};

class CPlayScenceKeyHandler : public CScenceKeyHandler
{
public:
	virtual void KeyState(BYTE* states);
	virtual void OnKeyDown(int KeyCode);
	virtual void OnKeyUp(int KeyCode);
	CPlayScenceKeyHandler(CScene* s) :CScenceKeyHandler(s) {};
};

