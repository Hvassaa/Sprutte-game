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


int main(void) {
  // init vals
  const int screenWidth  = 800;
  const int screenHeight = 450;
  const int mapWidth     = screenWidth -  20;
  const int mapHeight    = screenHeight - 20;
  const int mapUpper     = 10;
  const int mapLower     = mapUpper + mapHeight;
  const int mapLeft      = 10;
  const int mapRight     = mapLeft + mapWidth;

  InitWindow(screenWidth, screenHeight, "Sprutte Game");
  SetTargetFPS(60);
  ClearBackground(RAYWHITE);

  Vector2 playerPos       = { (float)screenWidth/2, (float)screenHeight/2 };
  const int playerRadius  = 20;
  const float playerSpeed = 2.0f;

  const int maxProjectiles = 50;
  int projectileAddIdx = 0;  
  Projectile projectiles[maxProjectiles];
  for (int i = 0; i < maxProjectiles; i++) {
    projectiles[i] = (Projectile){(Vector2){0,0}, (Vector2){0,0}, 0, 0, 0};
  }
  
  
  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
    {
      // controls
      if (IsKeyDown(KEY_D) && playerPos.x + playerRadius <= mapRight) playerPos.x += playerSpeed;
      if (IsKeyDown(KEY_A) && playerPos.x - playerRadius >= mapLeft)  playerPos.x -= playerSpeed;
      if (IsKeyDown(KEY_S) && playerPos.y + playerRadius <= mapLower) playerPos.y += playerSpeed;
      if (IsKeyDown(KEY_W) && playerPos.y - playerRadius >= mapUpper) playerPos.y -= playerSpeed;

      if (IsKeyDown(KEY_RIGHT)) {
	Projectile* p = &projectiles[projectileAddIdx];
	p->position = playerPos;
	p->speed = (Vector2) { (float)5, (float)0 };
	p->radius = 5;
	p->lifeTime = 60;	
	p->enabled = 1;
	projectileAddIdx = (projectileAddIdx + 1) % maxProjectiles;
      } else if (IsKeyDown(KEY_LEFT)) {
	Projectile* p = &projectiles[projectileAddIdx];
	p->position = playerPos;
	p->speed = (Vector2) { (float)-5, (float)0 };
	p->radius = 5;
	p->lifeTime = 60;	
	p->enabled = 1;
	projectileAddIdx = (projectileAddIdx + 1) % maxProjectiles;
      } else if (IsKeyDown(KEY_DOWN)) {
	Projectile* p = &projectiles[projectileAddIdx];
	p->position = playerPos;
	p->speed = (Vector2) { (float)0, (float)5 };
	p->radius = 5;
	p->lifeTime = 60;	
	p->enabled = 1;
	projectileAddIdx = (projectileAddIdx + 1) % maxProjectiles;
      } else if (IsKeyDown(KEY_UP)) {
	Projectile* p = &projectiles[projectileAddIdx];
	p->position = playerPos;
	p->speed = (Vector2) { (float)0, (float)-5 };
	p->radius = 5;
	p->lifeTime = 60;	
	p->enabled = 1;
	projectileAddIdx = (projectileAddIdx + 1) % maxProjectiles;
      }      

      for (int i = 0; i < maxProjectiles; i++) {
	Projectile* p = &projectiles[i];
	if (p->enabled) {
	  if (p->lifeTime == 0) {
	    p->enabled = 0;
	    continue;
	  }
	  p->position.x += p->speed.x;
	  p->position.y += p->speed.y;
	  p->lifeTime -= 1;	    
	}
      }      

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
