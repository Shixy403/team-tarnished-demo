#include "bullet.h"
#include "player.h"
#include "core.h"
#include "damageable.h"
#include <math.h>

//ProjectileCollection holds up to 1000 baseProjectile structs.
ProjectileCollection projectiles;


//---------------------------------------------------------
// file:	bullet.c
// author:	aaryan, leonard
// email:	aryan.b@digipen.edu, leonardjunyi.l@digipen.edu
//
// brief:	source file bullet.c defines the functionality for managing projectiles. 
//          the behaviour, components and purpose are described below.
//
// Copyright ? 2024 DigiPen, All rights reserved.
//---------------------------------------------------------

//Init ProjectileCollection to store and manage projectile that is stored in size of 1000.
void InitProjectiles(void)
{
    projectiles.size = 1000;
    projectiles.index = 0;
    memset(projectiles.projectiles, 0, sizeof(BaseProjectile) * projectiles.size);
    projectiles.projectiles->speedOffset = 1;
    //points to first baseProj  element only.
}

//remove projectiles without resizing the array
void RemoveProjectile(BaseProjectile* projectile) {
    //copy data of last projectile to projectile to delete
    *projectile = projectiles.projectiles[projectiles.index-1];
    //set index back by one, to overwrite last bullet
    --projectiles.index;
}

//Initialise and update the projectile to set direction, rotation, speed and collision.
static void UpdateProjectile(float enemyX, float enemyY, BaseProjectile* baseProjectile) {
    float angleRadians = baseProjectile->rotationOffset * (3.141592f / 180.f);
    baseProjectile->direction.x = currentActivePlayer->position.x - enemyX;
    baseProjectile->direction.y = currentActivePlayer->position.y - enemyY;
    baseProjectile->pos.x = enemyX;
    baseProjectile->pos.y = enemyY;
    baseProjectile->speed = 200.0f;
    baseProjectile->damage = 20.0f;
    baseProjectile->color = CP_Color_Create(255, 255, 255, 255);
    CP_Vector origin = CP_Vector_Normalize(CP_Vector_Set(CP_System_GetDisplayWidth() / 2.0f, 0));
    //set angle to between origin and bullet direction. (origin being top middle of screen, since angleCW)
    CP_Vector BulletDirection = CP_Vector_Normalize(CP_Vector_Set(baseProjectile->direction.x, baseProjectile->direction.y));
    baseProjectile->direction.x = BulletDirection.x;
    baseProjectile->direction.y = BulletDirection.y;
    float rotDeg = CP_Vector_AngleCW(origin, BulletDirection);
    baseProjectile->rotation = rotDeg;

    //Set collider component for collision checking between walls.
    ColliderComponent bulletsCollider = SetColliderComponent(TRUE);
    Collider sphereCollider1 = SetCollider(CP_Vector_Set(17.5, 7.5), SPHERE, (SizeUnion) { .diameter = 5 });
    Collider sphereCollider2 = SetCollider(CP_Vector_Set(2.5, 2.5), SPHERE, (SizeUnion) { .diameter = 5 });
    RotateColliderComponent(&bulletsCollider, baseProjectile->rotation);
    AddToColliderComponent(&bulletsCollider, sphereCollider1);
    AddToColliderComponent(&bulletsCollider, sphereCollider2);
    AttachColliderToObject(&baseProjectile, bulletsCollider, PROJECTILE_ACTOR);
}

void AddProjectile(float enemyX, float enemyY) {
    UpdateProjectile(enemyX, enemyY, &projectiles.projectiles[projectiles.index]);
    ++projectiles.index;
}

int ProjectileCollision(float playerX, float playerY, float playerdiameter, float projCenterX, float projCenterY, BaseProjectile* baseProj) {
    //collision with wall
    if (CheckIsGoingToHitWall(CP_Vector_Set(baseProj->pos.x, baseProj->pos.y),
        SPHERE, baseProj->colliderComponent.childColliders->dimension))
    {
        return 1;
    }
    //collision with player
    else if (IsCircleClicked(playerX, playerY, playerdiameter, projCenterX, projCenterY)) {
        return 2;
    }
    else
        return 0;

}

//Simulate projectile movement and resolve collisions.(local Function)
//Projectiles will not move if globalTimeScale is 0 / time is stopped.
void static FireProjectile(BaseProjectile* baseProjectile) {
    float dt = CP_System_GetDt() * globalTimeScale;
    baseProjectile->pos.x = baseProjectile->pos.x + (baseProjectile->direction.x * dt * baseProjectile->speed);
    baseProjectile->pos.y = baseProjectile->pos.y + (baseProjectile->direction.y * dt * baseProjectile->speed);
    baseProjectile->collisionFlag = ProjectileCollision(currentActivePlayer->position.x, currentActivePlayer->position.y,
        currentActivePlayer->diameter, baseProjectile->pos.x, baseProjectile->pos.y, baseProjectile);
    //if colliding with wall, delete this projectile
    if (baseProjectile->collisionFlag == 1) {
        RemoveProjectile(baseProjectile);
    }
    //if colliding with player, damage player and remove projectile.
    else if (baseProjectile->collisionFlag == 2) {
        RemoveProjectile(baseProjectile);
        DamagePlayer(currentActivePlayer, baseProjectile->damage);
    }
}

//All projectiles that exist projectileCollection will be "flash forwarded"
void FlashForwardProjectiles() {
    for (int i = 0; i < projectiles.index; ++i) {
        projectiles.projectiles[i].pos.x = projectiles.projectiles[i].pos.x + ((projectiles.projectiles[i].direction.x * CP_System_GetDt() * projectiles.projectiles[i].speed) * (TIME_SKIP_AMT / CP_System_GetDt()));
        projectiles.projectiles[i].pos.y = projectiles.projectiles[i].pos.y + ((projectiles.projectiles[i].direction.y * CP_System_GetDt() * projectiles.projectiles[i].speed) * (TIME_SKIP_AMT / CP_System_GetDt()));
    }
}

//Render projectiles in normal state and "forwarded" state with lower opacity (local function)
void RenderRectProjectile(BaseProjectile* baseProjectile) {
    CP_Settings_Fill(baseProjectile->color);
    CP_Graphics_DrawRectAdvanced(baseProjectile->pos.x, baseProjectile->pos.y, 20, 10, baseProjectile->rotation, 0);

    if (ab_FlashForward.state == ACTIVE) {
        CP_Settings_Fill(CP_Color_Create(baseProjectile->color.r, baseProjectile->color.g, baseProjectile->color.b, 60));
        CP_Graphics_DrawRectAdvanced(baseProjectile->pos.x + ((baseProjectile->direction.x * CP_System_GetDt() * baseProjectile->speed) * (TIME_SKIP_AMT / CP_System_GetDt())), 
            baseProjectile->pos.y + ((baseProjectile->direction.y * CP_System_GetDt() * baseProjectile->speed) * (TIME_SKIP_AMT / CP_System_GetDt())),
            20, 10, baseProjectile->rotation, 0);
    }
}

//Update all projectiles movement each frame 
void UpdateProjectiles(void)
{
    for (int i = 0; i < projectiles.index; ++i) {
        FireProjectile(&projectiles.projectiles[i]);
    }
}

//Render all projectiles on the screen.
void RenderProjectiles() {
    CP_Settings_NoStroke();
    for (int i = 0; i < projectiles.index; ++i) {
        RenderRectProjectile(&projectiles.projectiles[i]);
    }
}