#include <iostream>
#include <fstream>

#include "PlayScene.h"
#include "Utils.h"
#include "TextureManager.h"
#include "Animation.h"

#include "Const.h"
#include "XmlReader/tinyxml.h"

#include "Brick.h"
#include "Goomba.h"
#include "Koopas.h"
#include "RedSmallMario.h"
#include "RedBigMario.h"
#include "RedRaccoonMario.h"
#include "FireMario.h"
#include "RectCollision.h"
#include "QuestionBlock.h"


CPlayScene::CPlayScene(std::string id, std::string filePath) :
	CScene(id, filePath)
{
	
	key_handler = new CPlayScenceKeyHandler(this);
	timeScale = DEFAULT_TIME_SCALE;
	
}

/*
	Load scene resources from scene file (textures, sprites, animations and objects)
	See scene1.txt, scene2.txt for detail format specification
*/



CMario* CPlayScene::GenerateMario(MarioType mType, Vector2 pos) {
	switch (mType)
	{
	case MarioType::RED_SMALL:
		return new CRedSmallMario(pos.x, pos.y);
	case MarioType::RED_BIG:
		return new CRedBigMario(pos.x, pos.y);
	case MarioType::RED_RACCON:
		return new CRedRaccoonMario(pos.x, pos.y);
	case MarioType::FIRE:
		return new CFireMario(pos.x, pos.y);
	default:
		return new CRedSmallMario(pos.x, pos.y);
	}
	
}


bool CPlayScene::LoadDataFromFile() {
	TiXmlDocument document(sceneFilePath.c_str());
	if (!document.LoadFile())
	{
		OutputDebugStringW(ToLPCWSTR(document.ErrorDesc()));
		return false;
	}

	TiXmlElement* root = document.RootElement();



	// Load Texture
	for (TiXmlElement* textures = root->FirstChildElement("textures"); textures != nullptr; textures = textures->NextSiblingElement("textures")) {
		for (TiXmlElement* texture = textures->FirstChildElement("texture"); texture != nullptr; texture = texture->NextSiblingElement("texture")) {
			std::string id = texture->Attribute("id");
			std::string filePath = texture->Attribute("filePath");
			auto transColor = texture->Attribute("transColor");
			if (transColor != NULL) {
				auto rgb = split(transColor, ",");
				CTextures::GetInstance()->Add(id, ToLPCWSTR(filePath), D3DCOLOR_XRGB(atoi(rgb[0].c_str()), atoi(rgb[1].c_str()), atoi(rgb[2].c_str())));
			}
			else {
				CTextures::GetInstance()->Add(id, ToLPCWSTR(filePath), D3DXCOLOR());
			}
			
		}
	}

	// Load Sprite
	for (TiXmlElement* sprites = root->FirstChildElement("sprites"); sprites != nullptr; sprites = sprites->NextSiblingElement("sprites")) {
		for (TiXmlElement* sprite = sprites->FirstChildElement("sprite"); sprite != nullptr; sprite = sprite->NextSiblingElement("sprite")) {
			//std::string texId = sprite->Attribute("texId");
			std::string filePath = sprite->Attribute("filePath");
			CSprites::GetInstance()->LoadSpriteFromFile(filePath);
		}
	}

	// Load Animations
	for (TiXmlElement* animations = root->FirstChildElement("animations"); animations != nullptr; animations = animations->NextSiblingElement("animations")) {
		for (TiXmlElement* animation = animations->FirstChildElement("animation"); animation != nullptr; animation = animation->NextSiblingElement("animation")) {
			std::string idSet = animation->Attribute("idSet");
			std::string filePath = animation->Attribute("filePath");
			CAnimations::GetInstance()->LoadAnimationsFromFile(filePath, idSet);

		}
	}

	// Load Objects
	for (TiXmlElement* objs = root->FirstChildElement("objects"); objs != nullptr; objs = objs->NextSiblingElement("objects")) {
		// Mario
		for (TiXmlElement* mario = objs->FirstChildElement("mario"); mario != nullptr; mario = mario->NextSiblingElement("mario")) {
			if (player != NULL) break;
			int x = atoi(mario->Attribute("x"));
			int y = atoi(mario->Attribute("y"));
			int iType = 0; 
			mario->QueryIntAttribute("type", &iType);
			playerLevel = iType;
			SwitchPlayer(GenerateMario((MarioType)playerLevel, Vector2(x,y)));
			
		}

		for (TiXmlElement* mario = objs->FirstChildElement("enemyBullets"); mario != nullptr; mario = mario->NextSiblingElement("enemyBullets")) {
			
			int quantity = atoi(mario->Attribute("quantity"));
			for (int i = 0; i < quantity; i++) {
				enemyBullets.push_back(new CFireBullet(0,0,1));
			}
			

		}
		
	}
	
	std::string mapFilePath = root->Attribute("mapFilePath");
	sceneCamera.InitPositionController(player);
	sceneCamera.LoadMap(mapFilePath, &staticObjects, &dynamicObjects, &dynamicObjectsBehindMap, &tempObjects);
	
	sceneCamera.ChangeCamArea(Vector2(0, 0), Vector2(sceneCamera.GetMapSize().x, sceneCamera.GetMapSize().y - 520));
	return true;
}


void CPlayScene::Load()
{

	LoadDataFromFile();
	
	DebugOut(L"[INFO] Done loading scene resources %s\n", sceneFilePath);
	
}

void CPlayScene::Update(DWORD dt)
{
	// We know that Mario is the first object in the list hence we won't add him into the colliable object list
	// TO-DO: This is a "dirty" way, need a more organized way 
	
	if (player == NULL) return;

	if ( ((CMario*)(player))->GetAction() == MarioAction::EXPLODE ) {
		timeScale = 0;
	}
	else {
		timeScale = DEFAULT_TIME_SCALE;
	}
	
	dt = dt * timeScale;

	vector<LPGAMEOBJECT> coObjects;
	for (size_t i = 0; i < staticObjects.size(); i++)
	{
		coObjects.push_back(staticObjects[i]);
	}

	for (size_t i = 0; i < dynamicObjects.size(); i++)
	{
		if (sceneCamera.IsInCamera(Vector2(dynamicObjects[i]->x, dynamicObjects[i]->y)))
			dynamicObjects[i]->Update(dt, &coObjects);
		/*else
			dynamicObjects[i]->isDisable = true;*/
		
	}

	for (size_t i = 0; i < dynamicObjectsBehindMap.size(); i++)
	{
		if (sceneCamera.IsInCamera(Vector2(dynamicObjectsBehindMap[i]->x, dynamicObjectsBehindMap[i]->y)))
			dynamicObjectsBehindMap[i]->Update(dt, &coObjects);
		/*else
			dynamicObjects[i]->isDisable = true;*/

	}
	
	for (size_t i = 0; i < dynamicObjects.size(); i++)
	{
		coObjects.push_back(dynamicObjects[i]);
	}

	for (size_t i = 0; i < dynamicObjectsBehindMap.size(); i++)
	{
		coObjects.push_back(dynamicObjectsBehindMap[i]);
	}

	if (((CMario*)player)->GetAction() == MarioAction::DIE) {
		if (playerLevel - 1 >= 0) playerLevel--;

		CMario* newMario = GenerateMario(
		(MarioType)(playerLevel),
			Vector2(player->x, player->y));
		newMario->ChangeAction(MarioAction::EXPLODE);
		SwitchPlayer(newMario);
	}

	for (size_t i = 0; i < mainObjects.size(); i++)
	{
		if(!(dynamic_cast<CMario*>(mainObjects[i])) && !sceneCamera.IsInCamera(Vector2(mainObjects[i]->x, mainObjects[i]->y)))
			mainObjects[i]->isDisable = true;
		else
			mainObjects[i]->Update(dt, &coObjects);
	}

	
	for (size_t i = 0; i < tempObjects.size(); i++)
	{
		if (!tempObjects[i]->isTemp) {
			tempObjects.erase(tempObjects.begin() + i);
		}
		else if (sceneCamera.IsInCamera(Vector2(tempObjects[i]->x, tempObjects[i]->y)))
			tempObjects[i]->Update(dt, &mainObjects);
		else
			if (!dynamic_cast<CMario*>(tempObjects[i]))
				tempObjects[i]->isDisable = true;
		

	}

	for (size_t i = 0; i < enemyBullets.size(); i++)
	{
		if (sceneCamera.IsInCamera(Vector2(enemyBullets[i]->x, enemyBullets[i]->y)))
			enemyBullets[i]->Update(dt, &mainObjects);
		else
			enemyBullets[i]->isDisable = true;
	}

	for (size_t i = 0; i < effects.size(); i++)
	{
		if (!effects[i]->GetActiveState()) {
			effects.erase(effects.begin() + i);
			i--;
		}
		else
			effects[i]->Update(dt);
	}
	
	sceneCamera.Update(dt); // Update Map in Camera
}

void CPlayScene::Render()
{
	Vector2 camSize = sceneCamera.GetCamSize();

	for (int i = 0; i < dynamicObjectsBehindMap.size(); i++)
		if (!dynamicObjectsBehindMap[i]->isDisable) {
			Vector2 finalPos = sceneCamera.ConvertPosition(Vector2(dynamicObjectsBehindMap[i]->x, dynamicObjectsBehindMap[i]->y));
			if (sceneCamera.IsInCamera(Vector2(dynamicObjectsBehindMap[i]->x, dynamicObjectsBehindMap[i]->y)))
				dynamicObjectsBehindMap[i]->Render(finalPos);

		}

	sceneCamera.Render();
	for (int i = 0; i < staticObjects.size(); i++)
		staticObjects[i]->Render(sceneCamera.ConvertPosition(Vector2(staticObjects[i]->x, staticObjects[i]->y)));
	
	for (int i = 0; i < dynamicObjects.size(); i++)
		if (!dynamicObjects[i]->isDisable) {
			Vector2 finalPos = sceneCamera.ConvertPosition(Vector2(dynamicObjects[i]->x, dynamicObjects[i]->y));
			if (sceneCamera.IsInCamera(Vector2(dynamicObjects[i]->x, dynamicObjects[i]->y)))
				dynamicObjects[i]->Render(finalPos);

		}

	for (int i = 0; i < mainObjects.size(); i++)
		if (!mainObjects[i]->isDisable) {
			if (sceneCamera.IsInCamera(Vector2(mainObjects[i]->x, mainObjects[i]->y))) {
				Vector2 finalPos = sceneCamera.ConvertPosition(Vector2(mainObjects[i]->x, mainObjects[i]->y));
				mainObjects[i]->Render(finalPos);
			}
		}

	for (int i = 0; i < tempObjects.size(); i++) {
		if (sceneCamera.IsInCamera(Vector2(tempObjects[i]->x, tempObjects[i]->y))) {
			Vector2 finalPos = sceneCamera.ConvertPosition(Vector2(tempObjects[i]->x, tempObjects[i]->y));
			tempObjects[i]->Render(finalPos);
		}
			
	}
		
	for (int i = 0; i < enemyBullets.size(); i++) {
		if (sceneCamera.IsInCamera(Vector2(enemyBullets[i]->x, enemyBullets[i]->y))) {
			Vector2 finalPos = sceneCamera.ConvertPosition(Vector2(enemyBullets[i]->x, enemyBullets[i]->y));
			enemyBullets[i]->Render(finalPos);
		}

	}



	for (size_t i = 0; i < effects.size(); i++)
	{
		Vector2 finalPos = sceneCamera.ConvertPosition(effects[i]->GetCurrentPosition());
		effects[i]->Render(finalPos);
	}
	sceneCamera.RenderDetailBoard();
}


void CPlayScene::ChangeCameraArea(Vector2 playerPos, Vector2 LeftTopLimit, Vector2 RightBottomLimit) {
	player->SetPosition(playerPos.x, playerPos.y);
	sceneCamera.ChangeCamArea(LeftTopLimit, RightBottomLimit);
}

/*
	Unload current scene
*/


void CPlayScene::Unload()
{
	for (int i = 0; i < staticObjects.size(); i++)
		delete staticObjects[i];

	staticObjects.clear();
	SetPlayer(NULL);

	DebugOut(L"[INFO] Scene %s unloaded! \n", sceneFilePath);
}

void CPlayScene::SwitchPlayer(LPGAMEOBJECT newPlayer) {
	if (!newPlayer) return;

	if (player != NULL) {
		float oldLeft, oldTop, oldRight, oldBottom;
		player->GetBoundingBox(oldLeft, oldTop, oldRight, oldBottom);
		float newLeft, newTop, newRight, newBottom;
		newPlayer->GetBoundingBox(newLeft, newTop, newRight, newBottom);
		newPlayer->y -= (newBottom - oldBottom);
		newPlayer->x -= (newRight - oldRight);
	}
	
	
	// Delete pointer of Old Mario in List Objects
	
	while (mainObjects.size() > 0) {
		delete mainObjects.at(0);
		mainObjects.erase(mainObjects.begin());
	}
	
	player = newPlayer;
	mainObjects.push_back(newPlayer);
	// Add Fire Bullet
	if (dynamic_cast<CFireMario*>(newPlayer)) {
		mainObjects.push_back(((CFireMario*)newPlayer)->GetBullet(0));
		mainObjects.push_back(((CFireMario*)newPlayer)->GetBullet(1));
	}

	sceneCamera.InitPositionController(newPlayer);
}

void CPlayScenceKeyHandler::OnKeyDown(int KeyCode)
{
	//DebugOut(L"[INFO] KeyDown: %d\n", KeyCode);

	CMario* currentPlayer = (CMario * )(((CPlayScene*)scence)->GetPlayer());
	CGame* gameInstance = CGame::GetInstance();

	currentPlayer->ProcessKeyboard(gameInstance->GenerateKeyboardEvent(KeyCode, false));
	switch (KeyCode)
	{
		case DIK_0:
			currentPlayer->Reset();
			break;
		case DIK_1: {
			((CPlayScene*)scence)->SwitchPlayer(
				new CRedSmallMario(currentPlayer->x, currentPlayer->y)
			);
		}	break;

		case DIK_2: {
			((CPlayScene*)scence)->SwitchPlayer(
				new CRedBigMario(currentPlayer->x, currentPlayer->y)
			);
		}	break;

		case DIK_3: {
			((CPlayScene*)scence)->SwitchPlayer(
				new CRedRaccoonMario(currentPlayer->x, currentPlayer->y)
			);
		}	break;
	
		case DIK_4: {
			((CPlayScene*)scence)->SwitchPlayer(
				new CFireMario(currentPlayer->x, currentPlayer->y)
			);
		}	break;

		/*case DIK_P: {
			((CPlayScene*)scence)->ChangeCameraArea();
		}	break;*/

	}

}

void CPlayScenceKeyHandler::OnKeyUp(int KeyCode)
{
	CMario* currentPlayer = (CMario*)(((CPlayScene*)scence)->GetPlayer());
	CGame* gameInstance = CGame::GetInstance();
	currentPlayer->ProcessKeyboard(gameInstance->GenerateKeyboardEvent(KeyCode, false, true));
}

void CPlayScenceKeyHandler::KeyState(BYTE* states)
{
	CGame* gameInstance = CGame::GetInstance();
	CMario* currentPlayer = (CMario * )(((CPlayScene*)scence)->GetPlayer());
	if (currentPlayer == NULL)return;
	std::vector<int> UnOrderProcessKey = { DIK_A, DIK_S };
	std::vector<int> OrderProcessKey = { DIK_RIGHT, DIK_LEFT, DIK_DOWN };
	

	for (int i = 0; i < UnOrderProcessKey.size(); i++) {
		if (gameInstance->IsKeyDown(UnOrderProcessKey[i])) {
			currentPlayer->ProcessKeyboard(gameInstance->GenerateKeyboardEvent(UnOrderProcessKey[i], true));
		}
	}

	for (int i = 0; i < OrderProcessKey.size(); i++) {
		if (gameInstance->IsKeyDown(OrderProcessKey[i])) {
			currentPlayer->ProcessKeyboard(gameInstance->GenerateKeyboardEvent(OrderProcessKey[i], true));
			return;
		}
				
	}
	currentPlayer->ProcessKeyboard(gameInstance->GenerateKeyboardEvent(-1, true));
	
	
	
}