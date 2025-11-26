//
// Created by jack lewis on 11/7/25.
//


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "math.h"

#include "main.h"
#include "voxels.h"


typedef enum axis {X_AXIS, Y_AXIS, Z_AXIS} axis_t;

int checkCollision(fvec3 feetPos, double radius, double height, struct world * world) {
    int minX = floor(feetPos.x - radius);
    int minY = floor(feetPos.y);
    int minZ = floor(feetPos.z - radius);
    int maxX = floor(feetPos.x + radius);
    int maxY = floor(feetPos.y + height);
    int maxZ = floor(feetPos.z + radius);
    for (int x = minX; x <= maxX; x++) {
        for (int y = minY; y <= maxY; y++) {
            for (int z = minZ; z <= maxZ; z++) {
                if (getVoxelInWorld(*world, x, y, z)) return 1;
            }
        }
    }
    return 0;
}

void resolveAxis(player_t * p, fvec3 * v, axis_t axis, struct world * world) {
    fvec3 newPos = p->pos;
    switch (axis) {
        case X_AXIS: newPos.x += v->x; break;
        case Y_AXIS: newPos.y += v->y; break;
        default    : newPos.z += v->z; break;
    }
    int collision = checkCollision(newPos, p->radius, p->height, world);
    if (collision) {
        double centerPenetration, correctionAmount;
        switch (axis) {
            case X_AXIS:
                centerPenetration = newPos.x - (int)(newPos.x);
                correctionAmount = centerPenetration;// + p->radius;
                newPos.x -= v->x;
                v->x = 0;
                break;
            case Y_AXIS:
                if (v->y < 0) p->onGround = 1;
                centerPenetration = newPos.y - (int)(newPos.y);
                correctionAmount = centerPenetration;// + p->radius;
                newPos.y -= v->y;
                v->y = 0;
                break;
            case Z_AXIS:
                centerPenetration = newPos.z - (int)(newPos.z);
                correctionAmount = centerPenetration;// + p->radius;
                newPos.z -= v->z;
                v->z = 0;
                break;
        }
    } else if (axis == Y_AXIS) {p->onGround = 0;}
    p->pos = newPos;
}

void processPlayerMovement(player_t * player, double deltaTime) {
    // mouse look
    player->pitch += gInput.mouseDeltaY * 0.001;
    player->yaw += gInput.mouseDeltaX * 0.001;
    if (player->pitch > M_PI / 2) player->pitch = M_PI / 2;
    if (player->pitch < -M_PI / 2) player->pitch = -M_PI / 2;
    if (player->yaw > 2 * M_PI) player->yaw -= 2 * M_PI;
    // key input
    if (gInput.keys[GLFW_KEY_SPACE] && !player->move == FLY) {
        player->vel.y = player->jump;
        gInput.keys[GLFW_KEY_SPACE] = 0;
    }
    if (gInput.keys[GLFW_KEY_R])
    {
        ivec3 hit = {0,0,0};
        int hitfound = voxelRaycastPlace(*player->world, (fvec3){player->pos.x, player->pos.y+player->cameraHeight, player->pos.z}, player->yaw - M_PI * 0.5, player->pitch * -1, 5, &hit);
        if (hitfound) {
        if (
            hit.x == (int)player->pos.x &&
            hit.y == (int)player->pos.y &&
            hit.z == (int)player->pos.z) {} else // TODO : this is nasty
            setVoxelInWorld(*player->world, hit.x, hit.y, hit.z, player->client->blockType);
        }
        gInput.keys[GLFW_KEY_R] = 0;
    }
    if (gInput.keys[GLFW_KEY_E])
    {
        ivec3 hit = {0,0,0};
        int hitfound = voxelRaycastHit(*player->world,(fvec3){ player->pos.x, player->pos.y+player->cameraHeight, player->pos.z}, player->yaw - M_PI * 0.5, player->pitch * -1, 5, &hit);
        if (hitfound) {
            setVoxelInWorld(*player->world, hit.x, hit.y, hit.z, 0);
        }
        gInput.keys[GLFW_KEY_E] = 0;
    }
    
    // move
    float magnitude = player->speed * deltaTime;
    
    fvec3 dir = {0};
    if (gInput.keys[GLFW_KEY_W]) { dir.z -= 1; }
    if (gInput.keys[GLFW_KEY_A]) { dir.x -= 1; }
    if (gInput.keys[GLFW_KEY_S]) { dir.z += 1; }
    if (gInput.keys[GLFW_KEY_D]) { dir.x += 1; }
    if (player->move == FLY && gInput.keys[GLFW_KEY_SPACE]) { player->pos.y += magnitude; }
    if (player->move == FLY && gInput.keys[GLFW_KEY_LEFT_SHIFT]) { player->pos.y -= magnitude; }
    
    double vx = dir.x * cos(player->yaw) - dir.z * sin(player->yaw);
    double vz = dir.x * sin(player->yaw) + dir.z * cos(player->yaw);

    player->vel.x = vx * magnitude;
    player->vel.z = vz * magnitude;
    
    if (player->move != FLY) player->vel.y -= player->gravity * deltaTime;

    if (player->move == FLY) {
        player->pos.x += player->vel.x;
        player->pos.z += player->vel.z;
    }
    else {
        float yv = player->vel.y;
        player->vel.y *= deltaTime;
        resolveAxis(player, &player->vel, X_AXIS, player->world);
        resolveAxis(player, &player->vel, Y_AXIS, player->world);
        resolveAxis(player, &player->vel, Z_AXIS, player->world);
        player->vel.y =  player->vel.y ? yv : 0;
    }
}
