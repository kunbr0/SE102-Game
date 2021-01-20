#include "BoomerangBrother.h"
#include "PlayScene.h"
#include "Boomerang.h"
#define DELTA_BOOMERANG_PREPARE_ATTACK			Vector2(-22, -28)

CBoomerangBrother::CBoomerangBrother(float x, float y) : CEnemy(x,y,-1) {
	ChangeState(EEnemyState::LIVE, 5000);
	useChangeDirectionAfterAxisCollide = true;
	walkingScope.y = 6400;
}

std::string CBoomerangBrother::GetAnimationIdFromState() {
	switch (state.type)
	{
	case EEnemyState::LIVE:
		return "ani-boomerang-brother-move";
	case EEnemyState::PREPARE_ATTACK:
		return "ani-boomerang-brother-move";
	case EEnemyState::ATTACK:
		return "ani-boomerang-brother-attack";
	case EEnemyState::WILL_DIE:
		return "";
	default:
		return "";
	}
}

Vector2 CBoomerangBrother::GetBoundingBoxSize(EEnemyState st) {
	return Vector2(48, 66);
	switch (st)
	{
		case EEnemyState::LIVE:
			return Vector2(48, 96);
		case EEnemyState::WILL_DIE:
			return Vector2(0, 0);
		case EEnemyState::DIE:
		case EEnemyState::ONESHOTDIE:
			return Vector2(0, 0);
		default:
			return Vector2(48, 48);
	}
}

void CBoomerangBrother::Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects)
{
	if (state.type == EEnemyState::DIE) return;
	
	int playerX = ((CPlayScene*)(CGame::GetInstance()->GetCurrentScene()))->GetPlayer()->GetPosition().x;
	if (playerX > this->x) renderNX = 1; else renderNX = -1;

	ApplyGravity();
	CEnemy::Update(dt, coObjects);

	ChangeDirectionAfterAxisCollide();
	UpdateWithCollision(coObjects);


	if (state.type == EEnemyState::LIVE) ChangeState(EEnemyState::PREPARE_ATTACK, 600);
	else if (state.type == EEnemyState::PREPARE_ATTACK) {
		if(ChangeState(EEnemyState::ATTACK, 200)) ShootBullet();
	} 
	else if (state.type == EEnemyState::ATTACK) ChangeState(EEnemyState::LIVE, 5000);
}

void CBoomerangBrother::Render(Vector2 finalPos) {
	if (state.type == EEnemyState::DIE) return;
	//RenderBoundingBox(finalPos);
	CAnimations::GetInstance()->Get(GetAnimationIdFromState())->Render(finalPos, Vector2(-renderNX, ny), 255);
	
	if (state.type == EEnemyState::PREPARE_ATTACK) {
		CSprites::GetInstance()->Get("spr-boomerang-spin-0")->DrawWithScaling(finalPos + DELTA_BOOMERANG_PREPARE_ATTACK);
	}
	
	RenderExtraEffect(finalPos);
}


bool CBoomerangBrother::ShootBullet() {
	vector<LPGAMEOBJECT> bullets = ((CPlayScene*)(CGame::GetInstance()->GetCurrentScene()))->GetEnemyBullets();
	for (int i = 0; i < bullets.size(); i++) {
		if (dynamic_cast<CBoomerang*>(bullets.at(i))) {
			Vector2 playerPos = ((CPlayScene*)(CGame::GetInstance()->GetCurrentScene()))->GetPlayer()->GetPosition();
			Vector2 bulletBeginPos = this->GetPosition() + DELTA_BOOMERANG_PREPARE_ATTACK;
			Vector2 direction;
			direction.x = (playerPos.x - x) > 0 ? 1 : -1;
			direction.y = (playerPos.y - y) > 0 ? 1 : -1;
			if (bullets.at(i)->isDisable) {
				bullets.at(i)->vx = 0.48f;
				bullets.at(i)->vy = -0.23f;
				((CBoomerang*)(bullets.at(i)))->SetOwner(this);
				bullets.at(i)->SetPosition(bulletBeginPos);
				((CBoomerang*)bullets.at(i))->SetVenusBulletDirection(direction);
				bullets.at(i)->nx = nx;
				bullets.at(i)->isDisable = false;
				return true;
			}
		}

	}
	return false;
}