#pragma once
#include "Goomba.h"

#define GOOMBA_HAS_WING_BBOX_WIDTH				48
#define GOOMBA_HAS_WING_BBOX_HEIGHT				65
#define GOOMBA_BBOX_WILL_DIE_HEIGHT				18


#define GOOMBA_BBOX_HEIGHT_DIE 9

#define REDGOOMBA_OPEN_WING_WALKING				"ani-red-para-goomba-fly" 
#define REDGOOMBA_CLOSE_WING_WALKING			"ani-red-para-goomba-idle" 
#define REDGOOMBA_NO_WING_WALKING				"ani-red-goomba-walk"
#define REDGOOMBA_WILLDIE						"ani-red-goomba-die"


struct SHasWingBehavior {
	short step = 0;
	int time = 0;
};

class CRedGoomba : public CGoomba
{
	SHasWingBehavior hasWingBehavior;


public:
	CRedGoomba(float x, float y);
	void Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects) override;
	void ChangeStepHasWingBehavior(DWORD);
	void ProcessHasWingBehavior();
	void CollidedTop(vector<LPCOLLISIONEVENT>*) override;
	void BeingCollidedTop(LPGAMEOBJECT) override;

	Vector2 GetBoundingBoxSize(EEnemyState) override;
	std::string GetAnimationIdFromState() override;
};