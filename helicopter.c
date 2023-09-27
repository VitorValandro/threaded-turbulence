#include <SDL2/SDL.h>
#include <stdio.h>
#include <pthread.h>
#include "helicopter.h"

extern int currentHostages;
extern int rescuedHostages;
extern int HELICOPTER_WIDTH;
extern int BUILDING_WIDTH;
extern int NUM_HOSTAGES;
extern int SCREEN_WIDTH;
extern int BUILDING_WIDTH;
extern int SCREEN_HEIGHT;

// Função pra criar um objeto
HelicopterInfo createHelicopter(int x, int y, int w, int h, int speed, SDL_Rect** collisionRectArray) {
    HelicopterInfo helicopterInfo;
    helicopterInfo.rect.x = x;
    helicopterInfo.rect.y = y;
    helicopterInfo.rect.w = w;
    helicopterInfo.rect.h = h;
    helicopterInfo.speed = speed;
    helicopterInfo.fixed_collision_rects = collisionRectArray;
    helicopterInfo.missile_collision_rects = (MissileInfo **)malloc(sizeof(MissileInfo *) * 20);
    helicopterInfo.num_missile_collision_rects = 0;
    helicopterInfo.transportingHostage = false;
    return helicopterInfo;
}

void addHelicopterCollisionMissile(HelicopterInfo* helicopter, MissileInfo* missile) {
    helicopter->missile_collision_rects[helicopter->num_missile_collision_rects] = missile;
    helicopter->num_missile_collision_rects++;
}

void checkMissileCollisions(SDL_Rect helicopterRect, MissileInfo* missiles[], int missiles_length) {
    for (int i = 0; i < missiles_length; i++)
    {
        if (&missiles[i]->active) {
            SDL_Rect *collisionRect = &missiles[i]->rect;
            if (SDL_HasIntersection(&helicopterRect, collisionRect)) {   
                printf("Collision detected with missile %d\n", i);
            }
        }
    }
}

void checkHelicopterCollisions(SDL_Rect helicopterRect, SDL_Rect* rects[], int rects_length) {
    for (int i = 0; i < rects_length; i++)
    {
        SDL_Rect *collisionRect = rects[i];
            if (SDL_HasIntersection(&helicopterRect, collisionRect)) {
                printf("Collision detected with rect %d\n", i);
            }
    }
}

// Função concorrente para mover o helicóptero que é controlado pelo usuário
void* moveHelicopter(void* arg) {
    HelicopterInfo* helicopterInfo = (HelicopterInfo*)arg;

    while (1) {
        const Uint8* keystates = SDL_GetKeyboardState(NULL);

        // Checa o estado atual do teclado pra ver se está pressionado
        if (keystates[SDL_SCANCODE_LEFT]) {
            helicopterInfo->rect.x -= helicopterInfo->speed;
        }
        if (keystates[SDL_SCANCODE_RIGHT]) {
            helicopterInfo->rect.x += helicopterInfo->speed;
        }
        if (keystates[SDL_SCANCODE_UP]) {
            helicopterInfo->rect.y -= helicopterInfo->speed;
        }
        if (keystates[SDL_SCANCODE_DOWN]) {
            helicopterInfo->rect.y += helicopterInfo->speed;
        }

        checkHelicopterCollisions(
            helicopterInfo->rect,
            helicopterInfo->fixed_collision_rects,
            2
        );

        checkMissileCollisions(
            helicopterInfo->rect,
            helicopterInfo->missile_collision_rects,
            helicopterInfo->num_missile_collision_rects
        );

        if (currentHostages > 0 && helicopterInfo->rect.x + HELICOPTER_WIDTH < BUILDING_WIDTH && !helicopterInfo->transportingHostage) {
            helicopterInfo->transportingHostage = true;
            currentHostages--;
            printf("\nTRANSPORTANDO REFÉM\n");
        }

        if (rescuedHostages < NUM_HOSTAGES && helicopterInfo->rect.x > SCREEN_WIDTH - BUILDING_WIDTH && helicopterInfo->transportingHostage) {
            helicopterInfo->transportingHostage = false;
            rescuedHostages++;
            printf("\nRESGATOU REFÉM\n");
        }

        // Espera 10ms pra controlar a velocidade
        SDL_Delay(10);
    }

    return NULL;
}


// Função concorrente para mover os mísseis
void* moveMissiles(void* arg) {
    MissileInfo* missileInfo = (MissileInfo*)arg;

    while (1) {

        if (missileInfo->active) {
            // Atualiza as posições lógicas do míssil se estiver ativo
            missileInfo->rect.x += (int)(missileInfo->speed * cos(missileInfo->angle));
            missileInfo->rect.y -= (int)(missileInfo->speed * sin(missileInfo->angle));

            // Desativa o míssil se ele sair da tela
            if (missileInfo->rect.x < 0 || missileInfo->rect.x > SCREEN_WIDTH || missileInfo->rect.y < 0 || missileInfo->rect.y > SCREEN_HEIGHT) {
                missileInfo->active = false;
            }
        }

        SDL_Delay(10);
    }

    return NULL;
}