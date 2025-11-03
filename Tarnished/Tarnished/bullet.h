//---------------------------------------------------------
// file:	bullet.h
// author:	Leonard
// email:	leonardjunyi.l@digipen.edu
//
// brief:	Main entry point for the sample project
//			of the CProcessing library
//
// Copyright © 2024 DigiPen, All rights reserved.
//---------------------------------------------------------

#pragma once
#include "cprocessing.h"
#include "levelObjects.h"

typedef struct baseProjectile {
    CP_Vector pos, direction;
    CP_BOOL collisionFlag, rotCW;
    CP_Color color;
    float rotation;
    float damage;
    float speed;
    float speedOffset, rotationOffset;
    ColliderComponent colliderComponent;
} BaseProjectile;

typedef struct projectileCollection {
    BaseProjectile projectiles[1000];
    int size;
    int index;
} ProjectileCollection;

void InitProjectiles(void);
void AddProjectile(float enemyX, float enemyY);
void FlashForwardProjectiles();
void UpdateProjectiles(void);
void RenderProjectiles(void);
