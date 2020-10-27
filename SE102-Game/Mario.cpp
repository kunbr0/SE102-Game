#include <algorithm>
#include <assert.h>
#include "Utils.h"

#include "Mario.h"
#include "Game.h"

#include "Goomba.h"
#include "Portal.h"

CMario::CMario(float x, float y) : CGameObject()
{

	untouchable = 0;

	start_x = x;
	start_y = y;
	this->x = x;
	this->y = y;
	isShowingSpecialAni = "-1";
}

void CMario::Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects)
{
	dt = 20;


	float vxmax = isBoostedSpeed ? VELOCITY_X_SPEEDUP_MAX : VELOCITY_X_MAX;
	
	// Increase velocity if in limit
	if (abs(vx) < vxmax)
		vx += ax * ( isBoostedSpeed ? ACCELERATION_X_RUN_RATIO : 1);


	if (vx > 0) {
		vx += -ACCELERATION_FRICTION;
		if (vx < 0) vx = 0;
	}
	if (vx < 0) {
		vx += ACCELERATION_FRICTION;
		if (vx > 0) vx = 0;
	}
	
	// Calculate dx, dy 
	CGameObject::Update(dt);

	// Simple fall down
	applyGravity();

	vector<LPCOLLISIONEVENT> coEvents;
	vector<LPCOLLISIONEVENT> coEventsResult;

	coEvents.clear();

	// turn off collision when die 
	if (state != MARIO_STATE_DIE)
		CalcPotentialCollisions(coObjects, coEvents);

	// reset untouchable timer if untouchable time has passed
	if (GetTickCount() - untouchable_start > MARIO_UNTOUCHABLE_TIME)
	{
		untouchable_start = 0;
		untouchable = 0;
	}

	// No collision occured, proceed normally
	if (coEvents.size() == 0)
	{
		x += dx;
		y += dy;
		if (dy > 7) {
			ChangeAction(MarioAction::FALL);
		}
	}
	else
	{
		float min_tx, min_ty, nx = 0, ny;
		float rdx = 0;
		float rdy = 0;

		// TODO: This is a very ugly designed function!!!!
		FilterCollision(coEvents, coEventsResult, min_tx, min_ty, nx, ny, rdx, rdy);

		// how to push back Mario if collides with a moving objects, what if Mario is pushed this way into another object?
		//if (rdx != 0 && rdx!=dx)
		//	x += nx*abs(rdx); 

		// block every object first!
		x += min_tx * dx + nx * 0.4f;

		y += min_ty * dy + ny * 0.4f;

		if (nx != 0) vx = 0;

		// Co va cham theo chieu Y
		if (ny != 0) {
			vy = 0;
			if (ny < 0) ChangeAction(MarioAction::IDLE);
		}



		//
		// Collision logic with other objects
		//
		for (UINT i = 0; i < coEventsResult.size(); i++)
		{
			LPCOLLISIONEVENT e = coEventsResult[i];

			if (dynamic_cast<CGoomba*>(e->obj)) // if e->obj is Goomba 
			{
				CGoomba* goomba = dynamic_cast<CGoomba*>(e->obj);

				// jump on top >> kill Goomba and deflect a bit 
				if (e->ny < 0)
				{
					if (goomba->GetState() != GOOMBA_STATE_DIE)
					{
						goomba->SetState(GOOMBA_STATE_DIE);
						vy = -MARIO_JUMP_DEFLECT_SPEED;
					}
				}
				else if (e->nx != 0)
				{
					if (untouchable == 0)
					{
						/*if (goomba->GetState() != GOOMBA_STATE_DIE)
						{
							if (level > MARIO_LEVEL_SMALL)
							{
								level = MARIO_LEVEL_SMALL;
								StartUntouchable();
							}
							else
								SetState(MARIO_STATE_DIE);
						}*/
					}
				}
			} // if Goomba
			else if (dynamic_cast<CPortal*>(e->obj))
			{
				CPortal* p = dynamic_cast<CPortal*>(e->obj);
				CGame::GetInstance()->SwitchScene(p->GetSceneId());
			}
		}
	}

	
	
	// clean up collision events
	for (UINT i = 0; i < coEvents.size(); i++) delete coEvents[i];

	if (vx != 0) {
		if(abs(vx) < VELOCITY_X_MIN_FOR_RUN)
			ChangeAction(MarioAction::WALK);
		else
			ChangeAction(MarioAction::RUN);
	}
	// SKID
	if (vx * nx < 0 && abs(vx) > VELOCITY_X_MIN_FOR_SKID) {
		vx = -nx * VELOCITY_X_AFTER_SKID;
		isShowingSpecialAni = GetAnimationId(MarioAction::SKID);
	}

	ResetTempValues(); // Set all temp values to initial value;
}

void CMario::Render(Vector2 finalPos) {
	if (state == MARIO_STATE_DIE)
		ani = MARIO_ANI_DIE;
	else {
		/*if (vx == 0)
			ChangeAction(MarioAction::IDLE);
		else
			ChangeAction(MarioAction::WALK);*/
		if (action == MarioAction::JUMP)
			ani = GetAnimationId(MarioAction::JUMP);
		if (action == MarioAction::FLY && type == MarioType::RED_RACCON)
			ani = GetAnimationId(MarioAction::FLY);
	}

	ani = GetAnimationId(action);

	SHORT exceptionalNX = 1;

	if (isShowingSpecialAni != "-1") {
		ani = isShowingSpecialAni;
		if (ani == GetAnimationId(MarioAction::SKID))
			exceptionalNX = -1;
	}

	int alpha = 255;
	if (untouchable) alpha = 128;


	bool isFinishAni = CAnimations::GetInstance()->Get(ani)->Render(finalPos, 255, !(nx * exceptionalNX > 0));
	if (isFinishAni) isShowingSpecialAni = "-1";


	//RenderBoundingBox(finalPos);
};

void CMario::GetBoundingBox(float& left, float& top, float& right, float& bottom) {}


void CMario::SetState(int state)
{
	CGameObject::SetState(state);

	switch (state)
	{
	case MARIO_STATE_PRESS_A:
		isBoostedSpeed = true;
		break;

	case MARIO_STATE_WALKING_RIGHT:
		ax = ACCELERATION_X_WALK;
		nx = 1;
		break;

	case MARIO_STATE_WALKING_LEFT:
		ax = -ACCELERATION_X_WALK;
		nx = -1;
		break;

	case MARIO_STATE_JUMP_X:
		// TODO: need to check if Mario is *current* on a platform before allowing to jump again
		if (action == MarioAction::IDLE || action == MarioAction::WALK) {
			vy = -MARIO_JUMP_SPEED_Y;
			ChangeAction(MarioAction::JUMP);
		}
		break;
		
	case MARIO_STATE_JUMP_S:
		ChangeAction(MarioAction::JUMP);
		break;

	case MARIO_STATE_IDLE:
		vx = 0;
		break;

	case MARIO_STATE_DIE:
		vy = -MARIO_DIE_DEFLECT_SPEED;
		break;

	case MARIO_STATE_NONE_PRESS_KEY:
		//friction = ACCELERATION_FRICTION; 
		break;
	}

}


std::string CMario::GetAnimationId(MarioAction action) {
	std::string typeId;
	std::string actionId;
	switch (type)
	{	
	case MarioType::RED_SMALL:
		typeId = "ani-small-mario";
		break;
	case MarioType::RED_BIG:
		typeId = "ani-big-mario";
		break;
	case MarioType::RED_RACCON:
		typeId = "ani-raccoon-mario";
		break;
	case MarioType::FIRE:
		typeId = "ani-fire-mario";
		break;

	default:
		typeId = "ani-small-mario";
		break;
	}

	switch (action)
	{
	case MarioAction::IDLE:
		actionId = "idle";
		break;
	case MarioAction::WALK:
		actionId = "walk";
		break;
	case MarioAction::RUN:
		actionId = "run";
		break;
	case MarioAction::JUMP:
		actionId = "jump";
		break;
	case MarioAction::FLY:
		actionId = "fly";
		break;
	case MarioAction::FALL:
		actionId = "fall";
		break;
	case MarioAction::SKID:
		actionId = "skid";
		break;
	case MarioAction::THROW:
		actionId = "throw";
		break;

	default:
		actionId = "idle";
		break;
	}
	
	return typeId + "-" + actionId;
}


/*
	Reset Mario status to the beginning state of a scene
*/
void CMario::Reset()
{
	SetState(MARIO_STATE_IDLE);
	SetPosition(start_x, start_y);
	SetSpeed(0, 0);
}

void CMario::ResetTempValues() {
	isBoostedSpeed = false;
	ax = 0;
}

void CMario::ChangeAction(MarioAction newAction) {
	switch (newAction)
	{
	case CMario::MarioAction::IDLE:
			action = newAction;
		break;

	case CMario::MarioAction::WALK:
		if (action == MarioAction::IDLE || action == MarioAction::RUN) action = newAction;
		break;

	case CMario::MarioAction::RUN:
		if (action == MarioAction::IDLE) action = newAction;
		break;

	case CMario::MarioAction::JUMP:
		if (action == MarioAction::IDLE || action == MarioAction::WALK || action == MarioAction::RUN) {
			vy = -MARIO_JUMP_SPEED_Y * 1.20f;
			action = newAction;
		}
		break;

	case CMario::MarioAction::FLY:
		action = newAction;
		break;

	case CMario::MarioAction::FALL:
		action = newAction;
		break;

	case CMario::MarioAction::SKID:
		if (action == MarioAction::RUN) action = newAction;
		break;

	case CMario::MarioAction::THROW:
		break;

	default:
		break;
	}
}