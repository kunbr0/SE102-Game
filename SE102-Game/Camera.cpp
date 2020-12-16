#include "Camera.h"
#include "Game.h"
#include "UIDrawer.h"
#include <iomanip>
#include "Mario.h"
#include "Mathf.h"

#define DELTA_CHANGE_CAM_BASE_POSITION			5

#define DetailsBoardHeight		200
#define YGROUND					710


CCamera::CCamera() {
	this->camPosition = Vector2(0, 0);
	this->camSize = Vector2(CGame::GetInstance()->GetScreenWidth(), CGame::GetInstance()->GetScreenHeight());
	this->positionController = NULL;
	mapData.timeRemaining = 300 * 1000;
}
CCamera::~CCamera() {

}

void CCamera::InitPositionController(CGameObject* player) {
	this->positionController = player;
}

void CCamera::LoadMap(std::string mapFilePath, vector<LPGAMEOBJECT>* staticObjects, vector<LPGAMEOBJECT>* dynamicObjects, vector<LPGAMEOBJECT>* dynamicObjectsBehindMap, vector<LPGAMEOBJECT>* tempObjects) {
	mMap = CGameMap().FromTMX(mapFilePath, staticObjects, dynamicObjects, dynamicObjectsBehindMap, tempObjects);
	mMap->GetMapSize(mapSize);
	
}

void CCamera::LoadMap(std::string mapFilePath, vector<LPGAMEOBJECT>* staticObjects, std::unordered_map<std::string, CSelectionNode*>* selectionNodes) {
	mMap = CGameMap().FromTMX(mapFilePath, staticObjects, selectionNodes);
	mMap->GetMapSize(mapSize);

}

Vector2 CCamera::GetCamPosition() {
	return camPosition;
}

void CCamera::SetMapData(int newTimeRemaning, int newScore, int playerLife) {
	if (newTimeRemaning != -1) mapData.timeRemaining = newTimeRemaning;
	if (newScore != -1) mapData.score = newScore;
	if (playerLife != -1) mapData.playerLife = playerLife;
}

void CCamera::AdjustTimeRemaining(int deltaTime) {
	mapData.timeRemaining -= deltaTime;
	if (mapData.timeRemaining < 0) mapData.timeRemaining = 0;
}

   void CCamera::AdjustScore(int deltaScore) {
	mapData.score += deltaScore;
	if (mapData.score < 0) mapData.score = 0;
}

Vector2 CCamera::ConvertPosition(Vector2 pos) {
	
	return Vector2(pos.x - camPosition.x, pos.y - camPosition.y);
}

bool CCamera::IsInCamera(Vector2 realPos, int outsideCam) {
	Vector2 finalPos = ConvertPosition((realPos));
	if (finalPos.x > 0 - outsideCam && finalPos.x < camSize.x + outsideCam 
		&& finalPos.y > 0 - outsideCam && finalPos.y < camSize.y + outsideCam)
		return true;
	return false;
}

void CCamera::ChangeCamArea(Vector2 ar1, Vector2 ar2) { 
	LeftTopLimit = ar1;
	RightBottomLimit = ar2; 
}

void CCamera::ChangeCamPosition(Vector2 newPos) {
	Vector2 newResult;
	newResult.x = newPos.x - camSize.x / 2;
	newResult.y = newPos.y - camSize.y / 2;

	/*newPos.y = newPos.y - camSize.y / 2;

	float deltaY = RightBottomLimit.y - camSize.y - newPos.y;
	if (deltaY > 300)
		newResult.y = newPos.y + deltaY * (1 - ((deltaY* deltaY) / (800*800)));
	else
		newResult.y = newPos.y + deltaY;*/
	SetCamPosition(newResult);
}


void CCamera::SetCamPosition(Vector2 pos) {
	

	if (pos.x < LeftTopLimit.x)
		pos.x = LeftTopLimit.x; // overflow left side
	if (pos.x + camSize.x > RightBottomLimit.x)
		pos.x = (int)(RightBottomLimit.x - camSize.x); // overflow right side
	
	if (pos.y < LeftTopLimit.y)
		pos.y = LeftTopLimit.y;
	MarioAction marioAction = ((CMario*)(positionController))->GetAction();
	bool isFlying = marioAction == MarioAction::FLY;
	bool isFalling = marioAction == MarioAction::FALL || marioAction == MarioAction::FALL_SLIGHTLY;
	
	if (isFlying) isLocked = false;
	if (!isLocked && marioAction == MarioAction::IDLE) isLocked = true;

	int BottomOffSet = 0;

	if (isLocked) BottomOffSet = 500;

	

	if (pos.y + BottomOffSet > RightBottomLimit.y - camSize.y) {
		pos.y = RightBottomLimit.y - camSize.y;
	}
	
	//if (((CMario*)(positionController))->GetAction() != MarioAction::FLY)
	//{
	//	auto val = Mathf::Min(LastLeftTopLimit.y, LastRightBottomLimit.y - camSize.y);
	//	if (pos.y >= val)
	//	{
	//		// DebugOut(L"Cam: %f, %f\n", pos.y, val);
	//		LeftTopLimit = LastLeftTopLimit;
	//		RightBottomLimit = LastRightBottomLimit;
	//	}
	//}

	
	pos.x = (int)pos.x;
	pos.y = (int)pos.y;
	camPosition = pos;
}


void CCamera::UpdateCamPosition() {
	float xMario, yMario;
	positionController->GetPosition(xMario, yMario);

	ChangeCamPosition(positionController->GetPosition());

	/*if (abs(CamBasePosition.y - yMario) < 300) {
		SetCamPosition(Vector2((int)(xMario - camSize.x / 2), (int)(CamBasePosition.y - camSize.y / 2) - 130));
	}
	else {
		SetCamPosition(Vector2((int)(xMario - camSize.x / 2), (int)(yMario - camSize.y / 2)));
	}*/
	//SetCamPosition(Vector2((int)(xMario - camSize.x / 2), (int)(CamBasePosition.y - camSize.y / 2) - 130));

	//SetCamPosition(Vector2((int)(xMario - camSize.x / 2), (int)(yMario - camSize.y / 2)));
	
}

void CCamera::Update(DWORD dt) {
	
	//CGame* game = CGame::GetInstance();
	UpdateCamPosition();
	mMap->UpdateCamPosition(camPosition);
	AdjustTimeRemaining(dt);
	
	
}

void CCamera::RenderDetailBoard() {

	LPDIRECT3DTEXTURE9 black = CTextures::GetInstance()->Get("black");
	RECT rect; rect.left = 0; rect.top = 0; rect.right = camSize.x; rect.bottom = DetailsBoardHeight;
	CGame::GetInstance()->DrawWithScaling(Vector2(camSize.x / 2, camSize.y - DetailsBoardHeight / 2 + 48), Vector2(0, 0), black, rect, 255);

	LPSPRITE hud = CSprites::GetInstance()->Get("spr-hud-0");
	LPSPRITE Mchar = CSprites::GetInstance()->Get("spr-m-tag-0");
	LPSPRITE Onechar = CSprites::GetInstance()->Get("spr-font-1");
	LPSPRITE Fourchar = CSprites::GetInstance()->Get("spr-font-4");
	Vector2 beginPos = Vector2(240, camSize.y + hud->getSize().y - DetailsBoardHeight + 20);

	

	hud->DrawWithScaling(beginPos);
	Mchar->DrawWithScaling(Vector2(beginPos.x-190, beginPos.y+14));
	Onechar->DrawWithScaling(Vector2(beginPos.x - 100, beginPos.y - 10));
	Fourchar->DrawWithScaling(Vector2(beginPos.x - 100, beginPos.y + 15));
	
	Vector2 beginScorePos = Vector2(beginPos.x - 65, beginPos.y + 15);
	int distance = 26;
	/*for (int i = 0; i < 7; i++) {
		LPSPRITE Zerochar = CSprites::GetInstance()->Get("spr-font-0");
		Zerochar->DrawWithScaling(Vector2(beginScorePos.x + i* distance, beginScorePos.y));

	}*/

	CUIDrawer::GetInstance()->DrawFixedLengthNumber(to_string(mapData.score), beginScorePos, '0', 7);
	if (positionController == NULL) return;
	for (int i = 0; i < 7; i++) {
		LPSPRITE Zerochar;
		if (i < positionController->powerX/1000) {
			Zerochar = CSprites::GetInstance()->Get("spr-triangle-icon-0");
		}
		else {
			Zerochar = CSprites::GetInstance()->Get("spr-triangle-icon-1");
		}
		
		if (i == 7 - 1) {
			Zerochar = CSprites::GetInstance()->Get("spr-p-icon-1");
			Zerochar->DrawWithScaling(Vector2(beginScorePos.x + i * distance + 15, beginScorePos.y - 25));
		}
		else
			Zerochar->DrawWithScaling(Vector2(beginScorePos.x + i * distance, beginScorePos.y - 25));

	}
	CUIDrawer::GetInstance()->DrawFixedLengthNumber(to_string(mapData.timeRemaining / 1000), beginScorePos + Vector2(221,0), '0', 3);
}

void CCamera::Render() {
	mMap->Render(DetailsBoardHeight+150);
}