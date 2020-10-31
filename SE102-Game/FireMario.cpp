#include "FireMario.h"

CFireMario::CFireMario(float x, float y) : CMario(x, y) {
	type = MarioType::FIRE;

	bullets.push_back(new CFireBullet(0.0f, 0.0f, 1));
	bullets.push_back(new CFireBullet(0.0f, 0.0f, 1));
}

bool CFireMario::ShootBullet() {
	for (int i = 0; i < bullets.size(); i++) {
		// Dang aanr thi dc ban
		if (bullets.at(i)->isDisable) {
			bullets.at(i)->PrepareForShooting();
			bullets.at(i)->UpdatePos(Vector2(this->x, this->y), this->nx);
			bullets.at(i)->isDisable = false;
			return true;
		}
	}
	return false;
}

void CFireMario::Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects)
{
	CMario::Update(dt, coObjects);
	// SKID

	
	
}

void CFireMario::Render(Vector2 finalPos)
{
	CMario::Render(finalPos);
}

void CFireMario::ProcessKeyboard(SKeyboardEvent kEvent)
{
	CMario::ProcessKeyboard(kEvent);

	

}