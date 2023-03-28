#include "GameScene.h"
#include "main.h"
#include "input.h"
#include "camera.h"
#include "Player.h"
#include "polygon2D.h"
#include "TextPolygon2D.h"
#include "SkyDome.h"
#include "Plane.h"
#include "Enemy.h"
#include "TitleLogo.h"

bool GameScene::m_LoadFinish = false;

void GameScene::Init(UINT version)
{
	AddGameObject<Camera>(DRAW_CAMERA, version);
	//AddGameObject<Polygon2D>(DRAW_2D, version);
	//AddGameObject<TextPolygon2D>(DRAW_2D, version);
	AddGameObject<SkyDome>(DRAW_3D, version);
	AddGameObject<Plane>(DRAW_3D, version);
	AddGameObject<Player>(DRAW_3D, version);
	//AddGameObject<Enemy>(DRAW_3D, version);
}

void GameScene::UnInit(UINT version)
{
	Scene::UnInit(version);

	m_LoadFinish = false;
}

void GameScene::Update(UINT version)
{
	Scene::Update(version);

}

void GameScene::Load()
{
	SkyDome::Load(WindowMain::version % 2);
	Plane::Load(WindowMain::version % 2);
	Player::Load(WindowMain::version % 2);
	//Enemy::Load(WindowMain::version % 2);

	m_LoadFinish = true;
}
