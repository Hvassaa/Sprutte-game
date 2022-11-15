#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>

const int maxProjectiles = 50; // TODO make this a defconst

typedef struct Projectile {
  Vector2 position;
  Vector2 speed;
  int     radius;
  int     lifeTime;
  bool    enabled;
} Projectile;

typedef struct Player {
  Vector2      position;
  float        speed;
  int          radius;
  unsigned int firerate;
  unsigned int shotCharge;
  float        shotSpeed;
} Player;

typedef struct ProjectilesContainer {
  Projectile* projectiles; // array
  int idx;
} ProjectilesContainer;

typedef struct Block {
  Vector2 start;
  Vector2 size;
} Block;

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


  DrawRectangle(50, 50, 100, 100, YELLOW);


  EndDrawing();  
}

void shoot(float xSpeed,
	   float ySpeed,
	   Vector2 origin,
	   ProjectilesContainer* pc) {
  /*
    Register a new projectile
   */
  Projectile* p = &(pc->projectiles[pc->idx]);
  p->position = origin;
  p->speed = (Vector2) {xSpeed, ySpeed};
  p->radius = 5;
  p->lifeTime = 60;	
  p->enabled = 1;
  pc->idx = (pc->idx + 1) % maxProjectiles;	
}

void playerShoot(Player* player, ProjectilesContainer* pc) {
  if (player->shotCharge >= player->firerate) {
    float speed = player->shotSpeed;
    if (IsKeyDown(KEY_RIGHT)) {
      shoot(speed, 0.0f, player->position, pc);
      player->shotCharge = 0;
    } else if (IsKeyDown(KEY_LEFT)) {
      shoot(-speed, 0.0f, player->position, pc);
      player->shotCharge = 0;
    } else if (IsKeyDown(KEY_DOWN)) {
      shoot(0.0f, speed, player->position, pc);
      player->shotCharge = 0;
    } else if (IsKeyDown(KEY_UP)) {
      shoot(0.0f, -speed, player->position, pc);
      player->shotCharge = 0;
    }
  }  
}

void updateProjectiles(ProjectilesContainer* pc) {
  for (int i = 0; i < maxProjectiles; i++) {
    Projectile* p = &(pc->projectiles[i]);
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
}

void move(Player* player,
	  int mapUpper,
	  int mapLower,
	  int mapLeft,
	  int mapRight) {
  Vector2 newPos = player->position;
  if (IsKeyDown(KEY_D)) {
    newPos.x += player->speed;
  }
  if (IsKeyDown(KEY_A)) {
      newPos.x -= player->speed;
  }
  if (IsKeyDown(KEY_S)) {
    newPos.y += player->speed;
  }
  if (IsKeyDown(KEY_W)) {
    newPos.y -= player->speed;
  }

  Block b = {
    {50, 50},
    {100, 100}
  };
  bool insideX = 0;
  bool insideY = 0;
  Block blocks[1] = {b};
  for (int i = 0; i < 1; i++) {
    Block b = blocks[i];
    insideX = (newPos.x + player->radius - 2 >= b.start.x &
		     newPos.x - player->radius + 2 <= b.start.x + b.size.x);
    insideY = (newPos.y + player->radius - 2>= b.start.y &&
		     newPos.y - player->radius + 2<= b.start.y + b.size.y);

    if(newPos.x != player->position.x && newPos.y != player->position.y) {
      printf("old: x: %f, y: %f\n", player->position.x, player->position.y);
      printf("new: x: %f, y: %f\n", newPos.x, newPos.y);
    }
    if (insideX && insideY) {
      return;
    }
    player->position = newPos;
    /* if (!insideX) { */
    /*   player->position.y = newPos.y; */
    /* } */
    /* if (!insideY) { */
    /*   player->position.x = newPos.x; */
    /* }     */
  }  
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
  Player player = {
    { (float)screenWidth/2, (float)screenHeight/2 },
    2.0f,
    20,
    8,
    8,
    5.0f
  };

  // init projectile values
  Projectile ps[50];
  for (int i = 0; i < maxProjectiles; i++) {
    ps[i] = (Projectile){(Vector2){0,0}, (Vector2){0,0}, 0, 0, 0};
  }
  ProjectilesContainer pc = {ps, 0};
   
  // set up raylib
  InitWindow(screenWidth, screenHeight, "Sprutte Game");
  SetTargetFPS(60);
  
  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
    {
      // Player movement
      /* if (IsKeyDown(KEY_D) && player.position.x + player.radius <= mapRight) player.position.x += player.speed; */
      /* if (IsKeyDown(KEY_A) && player.position.x - player.radius >= mapLeft)  player.position.x -= player.speed; */
      /* if (IsKeyDown(KEY_S) && player.position.y + player.radius <= mapLower) player.position.y += player.speed; */
      /* if (IsKeyDown(KEY_W) && player.position.y - player.radius >= mapUpper) player.position.y -= player.speed; */
      move(&player, mapUpper, mapLower, mapLeft, mapRight);

      player.shotCharge++;
      // Detect shooting, register new projectile
      playerShoot(&player, &pc);

      // Update each projectile 
      updateProjectiles(&pc);

      // draw everything
      doDraw(mapUpper,
	     mapLeft,
	     mapWidth,
	     mapHeight,
	     player.position,
	     player.radius,
	     pc.projectiles,
	     maxProjectiles);
    }

  // de-init
  CloseWindow();
  return 0;
}
