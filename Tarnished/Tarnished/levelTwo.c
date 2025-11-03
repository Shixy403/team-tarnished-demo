//---------------------------------------------------------
// file:	bullet.c
// author:	leonard
// email:	leonardjunyi.l@digipen.edu
//
// brief:	Source file containing the "LevelTwo" design containing objects like walls, pitfalls, lava and boundaries.
//			Has player interaction and movement, hud and camera functionality and events for enemy and boss spawning, 
//
// Copyright ? 2024 DigiPen, All rights reserved.
//---------------------------------------------------------

#include <stdio.h>
#include "cprocessing.h"
#include "widgets.h"
#include "damageable.h"
#include "utils.h"
#include "dialogueManager.h"
#include "player.h"
#include "levelObjects.h"
#include "enemy.h"
#include "core.h"
#include "hud.h"
#include "bullet.h"
#include "camera.h"
#include "simpleSpace_Sheet.h"
#include "drops.h"

float levelTimer;
Player player;

//non-static function to set bossSpawn behaviour in enemy.c
void SpawnDaBoss() {
	if (/*levelTimer >= 25.f &&*/ !isBossSpawned) SpawnEnemy = SpawnBoss;
	HUD_SetBossReference(&enemies.enemies[enemies.index - 1]);
}

// Inits World and level objects.
void static Init_World() {
	ClearAllLevelObjects();
	int i = 0;

	// WALLS
	AddLevelObjectToList(CreateWall(CP_Vector_Set(-300, -270), (SizeUnion) { .width = 50, .height = 150 }, BOX, FALSE));
	AddLevelObjectToList(CreateWall(CP_Vector_Set(990, -500), (SizeUnion) { .width = 220, .height = 50 }, BOX, FALSE));
	AddLevelObjectToList(CreateWall(CP_Vector_Set(700, 200), (SizeUnion) { .width = 50, .height = 150 }, BOX, FALSE));
	AddLevelObjectToList(CreateWall(CP_Vector_Set(-155, 460), (SizeUnion) { .width = 100, .height = 40 }, BOX, FALSE));
	AddLevelObjectToList(CreateWall(CP_Vector_Set(-175, 600), (SizeUnion) { .width =70, .height = 25 }, BOX, FALSE));
	AddLevelObjectToList(CreateWall(CP_Vector_Set(50, 460), (SizeUnion) { .width = 100, .height = 40 }, BOX, FALSE));
	AddLevelObjectToList(CreateWall(CP_Vector_Set(150, 500), (SizeUnion) { .width = 40, .height = 45 }, BOX, FALSE));
	AddLevelObjectToList(CreateWall(CP_Vector_Set(-300, 500), (SizeUnion) { .width = 40, .height = 45 }, BOX, FALSE));

	// MUD
	//AddLevelObjectToList(CreateMud(CP_Vector_Set(990, -600), (SizeUnion) { .width = 125, .height = 125 }, BOX, FALSE));

	// PITFALLS
	AddLevelObjectToList(CreatePitfall(CP_Vector_Set(-300, -500), (SizeUnion) { .width = 150, .height = 150 }, BOX, FALSE));
	AddLevelObjectToList(CreatePitfall(CP_Vector_Set(990,200), (SizeUnion) { .width = 200, .height = 200 }, BOX, FALSE));

	// LAVA
	//AddLevelObjectToList(CreateLava(CP_Vector_Set(-85, 500), (SizeUnion) { .width = 90, .height = 40 }, BOX, FALSE));
	AddLevelObjectToList(CreateLava(CP_Vector_Set(220, 630), (SizeUnion) { .width = 115, .height = 180 }, BOX, FALSE));
	AddLevelObjectToList(CreateLava(CP_Vector_Set(-380, 630), (SizeUnion) { .width = 115, .height = 180 }, BOX, FALSE));

	// LEVEL BOUNDARY
	AddLevelObjectToList(CreateWall(CP_Vector_Set(0, -750), (SizeUnion) { .width = 2700, .height = 50 }, BOX, FALSE));
	spawnPoints[i++] = CP_Vector_Set(600, -400);
	AddLevelObjectToList(CreateWall(CP_Vector_Set(-1350, 0), (SizeUnion) { .width = 50, .height = 1550 }, BOX, FALSE));
	spawnPoints[i++] = CP_Vector_Set(-600, -400);
	AddLevelObjectToList(CreateWall(CP_Vector_Set(0, 750), (SizeUnion) { .width = 2700, .height = 50 }, BOX, FALSE));
	spawnPoints[i++] = CP_Vector_Set(600, 400);
	AddLevelObjectToList(CreateWall(CP_Vector_Set(1350, 0), (SizeUnion) { .width = 50, .height = 1550 }, BOX, FALSE));
	spawnPoints[i++] = CP_Vector_Set(-600, 400);
	spawnPointsSize = i;
}

// Inits the player and then sets it as the currentactiveplayer for the player controller
void static Init_Player() {
	InitBasePlayer(&player);

	UpdatePlayerPosition(&player, 0, 0);

	ColliderComponent playerMainCollider = SetColliderComponent(TRUE);
	AddToColliderComponent(&playerMainCollider, SetCollider(CP_Vector_Set(0, 0), SPHERE, (SizeUnion) { .diameter = player.diameter }));

	AttachColliderToObject(&player, playerMainCollider, PLAYER_ACTOR);
	currentActivePlayer = &player;
	EnableInput(currentActivePlayer, TRUE);
}

void LevelTwo_Init() {
	// Inits Text related stuff
	facingDirectionArrow = CP_Image_Load("./Assets/UI/arrow_b.png");
	playerHurt = CP_Sound_Load("./Assets/Sounds/hitImpact.wav");
	simpleSpace_Sheet = CP_Image_Load("./Assets/Spritesheets/simpleSpace_Sheet.png");

	levelTimer = 0.f;
	currentLevel = 2;

	InitCore();
	SetDebugMode(FALSE);

	//add level-specific objects and player functionality.
	Init_World();
	Init_Player();

	//Set level enemy variables
	enemies.spawnInterval = 4.f;
	//Set spawn pattern
	SpawnEnemy = SpawnEnemyAtPoints;
	InitEnemies();
	SetRequiredKills(6);
	InitDrops();
	InitProjectiles();
	InitHUD();
	InitCamera();
	HUD_SetShowCustomCursor(TRUE);
}

void LevelTwo_Update() {

	CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 255));

	levelTimer += CP_System_GetDt() * globalTimeScale;
	//if doing kills for boss need seperate requiredBossKills var and removeEnemy need seperate both
	if (levelTimer >= 10.f && !isBossSpawned && SpawnEnemy != SpawnBoss) SpawnEnemy = SpawnEnemyPointsWave;

	UpdateEnemies();
	UpdateProjectiles();
	UpdateDrops();

	UpdateCamera();

	RenderLevelObjects();
	RenderEnemies();
	RenderProjectiles();
	RenderDrops();
	RenderPlayer(&player);
	RenderOverPlayerAttacks();
	UpdateCooldowns();
	HandleLevelCollisions();

	//Reset camera transform for UI
	CP_Settings_ResetMatrix();
	CP_Settings_ImageMode(CP_POSITION_CORNER);

	RenderHUD();

	HandleInputs();
	DashAttack();
}

//clean up resources and reset camera.
void LevelTwo_Exit() {
	CP_Settings_Stroke(CP_Color_Create(0, 0, 0, 255));
	CP_Settings_StrokeWeight(3.0f);
	ResetCamera();
	CP_Image_Free(&simpleSpace_Sheet);
	ClearHUD();
	ClearPlayers();
	ClearCore();
	ClearEnemies();
	HUD_SetShowCustomCursor(FALSE);
	currentLevel = 0;
}