#include "raylib.h"
#include <stdbool.h>

typedef struct Projectile {
  Vector2 position;
  Vector2 speed;
  int radius;
  int lifeTime;
  bool enabled;
} Projectile;

void doDraw(int mapUpper,
	    int mapLeft,
	    int mapWidth,
	    int mapHeight,
	    Vector2 playerPos,
	    int playerRadius,
	    Projectile projectiles[],
	    int maxProjectiles) {
  /*
    Helper function to (re)draw everything, in the following order
    - background
    - map
    - player
    - projectiles
   */
  BeginDrawing();
  ClearBackground(RAYWHITE);      
  DrawRectangle(mapUpper, mapLeft, mapWidth, mapHeight, RED);
  DrawCircleV(playerPos, playerRadius - 1, GREEN);
  for (int i = 0; i < maxProjectiles; i++) {      
    Projectile p = projectiles[i];
    if (p.enabled) DrawCircleV(p.position, p.radius, BLUE);
  }
  EndDrawing();  
}

void shoot(float xSpeed,
	   float ySpeed,
	   Vector2 origin,
	   Projectile projectiles[],
	   int* projectileAddIdx,
	   int maxProjectiles) {
  /*
    Register a new projectile
   */
  Projectile* p = &projectiles[*projectileAddIdx];
  p->position = origin;
  p->speed = (Vector2) {xSpeed, ySpeed};
  p->radius = 5;
  p->lifeTime = 60;	
  p->enabled = 1;
  *projectileAddIdx = (*projectileAddIdx + 1) % maxProjectiles;	
}


int main(void) {
  // init map values
  const int screenWidth  = 800;
  const int screenHeight = 450;
  const int mapWidth     = screenWidth -  20;
  const int mapHeight    = screenHeight - 20;
  const int mapUpper     = 10;
  const int mapLower     = mapUpper + mapHeight;
  const int mapLeft      = 10;
  const int mapRight     = mapLeft + mapWidth;

  // init player values
  Vector2 playerPos        = { (float)screenWidth/2, (float)screenHeight/2 };
  const int playerRadius   = 20;
  const float playerSpeed  = 2.0f;
  const int playerFireRate = 8;
  unsigned int shotTimer   = playerFireRate;

  // init projectile values
  const int maxProjectiles = 50;
  int projectileAddIdx = 0;  
  Projectile projectiles[maxProjectiles];
  for (int i = 0; i < maxProjectiles; i++) {
    projectiles[i] = (Projectile){(Vector2){0,0}, (Vector2){0,0}, 0, 0, 0};
  }

  // set up raylib
  InitWindow(screenWidth, screenHeight, "Sprutte Game");
  SetTargetFPS(60);
  
  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
    {
      // Player movement
      if (IsKeyDown(KEY_D) && playerPos.x + playerRadius <= mapRight) playerPos.x += playerSpeed;
      if (IsKeyDown(KEY_A) && playerPos.x - playerRadius >= mapLeft)  playerPos.x -= playerSpeed;
      if (IsKeyDown(KEY_S) && playerPos.y + playerRadius <= mapLower) playerPos.y += playerSpeed;
      if (IsKeyDown(KEY_W) && playerPos.y - playerRadius >= mapUpper) playerPos.y -= playerSpeed;

      // Detect shooting, register new projectile
      shotTimer++;
      if (shotTimer >= playerFireRate) {
	if (IsKeyDown(KEY_RIGHT)) {
	  shoot(5.0f, 0.0f, playerPos, projectiles, &projectileAddIdx, maxProjectiles);
	  shotTimer = 0;
	} else if (IsKeyDown(KEY_LEFT)) {
	  shoot(-5.0f, 0.0f, playerPos, projectiles, &projectileAddIdx, maxProjectiles);
	  shotTimer = 0;
	} else if (IsKeyDown(KEY_DOWN)) {
	  shoot(0.0f, 5.0f, playerPos, projectiles, &projectileAddIdx, maxProjectiles);
	  shotTimer = 0;
	} else if (IsKeyDown(KEY_UP)) {
	  shoot(0.0f, -5.0f, playerPos, projectiles, &projectileAddIdx, maxProjectiles);
	  shotTimer = 0;
	}
      }
      

      // Update each projectile 
      for (int i = 0; i < maxProjectiles; i++) {
	Projectile* p = &projectiles[i];
	if (p->enabled) {
	  if (p->lifeTime == 0) { // disable if lifetime ran out
	    p->enabled = 0;
	    continue;
	  }
	  p->position.x += p->speed.x;
	  p->position.y += p->speed.y;
	  p->lifeTime -= 1;	    
	}
      }      

      // draw everything
      doDraw(mapUpper,
	     mapLeft,
	     mapWidth,
	     mapHeight,
	     playerPos,
	     playerRadius,
	     projectiles,
	     maxProjectiles);
    }

  // de-init
  CloseWindow();
  return 0;
}
