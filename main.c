#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>

#define MAX_PROJECTILES 50
#define MAX_BLOCKS      50
#define R               3

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

typedef struct BlocksContainer {
  Block* blocks; // array
  int idx;
} BlocksContainer;

typedef struct Room {
  Block* blocks;
  int x;
  int y;
  bool enabled;
} Room;

void doDraw(
	    Vector2 playerPos,
	    int playerRadius,
	    Projectile projectiles[],
	    Block blocks[]) {
  /*
    Helper function to (re)draw everything, in the following order
    - background
    - map
    - player
    - projectiles
    - blocks
   */
  BeginDrawing();
  ClearBackground(RED);
  DrawCircleV(playerPos, playerRadius - 1, GREEN);
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    Projectile p = projectiles[i];
    if (p.enabled) DrawCircleV(p.position, p.radius, BLUE);
  }

  for (int i = 0; i < MAX_BLOCKS; i++) {
    Block b = blocks[i];
    DrawRectangle(b.start.x, b.start.y, b.size.x, b.size.y, YELLOW);
  }

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
  pc->idx = (pc->idx + 1) % MAX_PROJECTILES;
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
  for (int i = 0; i < MAX_PROJECTILES; i++) {
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

void move(Player* player, Block blocks[]) {
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

  bool xAllowed = 1;
  bool yAllowed = 1;

  for (int i = 0; i < MAX_BLOCKS; i++) {
    Block b = blocks[i];
    int bStartX = b.start.x - player->radius;
    int bStartY = b.start.y - player->radius;
    int bEndX = b.start.x + b.size.x + player->radius;
    int bEndY = b.start.y + b.size.y + player->radius;
    if(player->position.y <= bEndY &&
       player->position.y >= bStartY &&
       (newPos.x <= bEndX &&
	newPos.x >= bStartX)) {
      xAllowed = 0;
    }
    if(player->position.x <= bEndX &&
       player->position.x >= bStartX &&
       (newPos.y <= bEndY &&
	newPos.y >= bStartY)) {
      yAllowed = 0;
    }
  }
  if(xAllowed) {
    player->position.x = newPos.x;
  }
  if(yAllowed) {
    player->position.y = newPos.y;
  }
}

int main(void) {
  // init map values
  int playerRadius       = 20;
  const int screenWidth  = 800;
  const int screenHeight = 450;
  
  // init player values
  Player player = {
    { (float)screenWidth/2, (float)screenHeight/2 },
    2.0f,
    playerRadius,
    8,
    8,
    5.0f
  };

  // init projectile values
  Projectile ps[50];
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    ps[i] = (Projectile){(Vector2){0,0}, (Vector2){0,0}, 0, 0, 0};
  }
  ProjectilesContainer pc = {ps, 0};

  int doorSize = 70;
  // generate map



  // 
  bool rooms[R][R] = {
    {0, 0, 1},
    {1, 1, 1},
    {0, 1, 1}
  };
  Room map[R][R];
  for (size_t i = 0; i < R; i++)
  {
    for (size_t j = 0; j < R; j++)
    {
      bool enabled = rooms[i][j];
      if (!enabled) {
	/* Block blocks[MAX_BLOCKS]; */
	map[i][j] = (Room){
	  NULL,
	  i,
	  j,
	  0
	};
      } else {
	bool up    = 0;
	bool down  = 0;
	bool left  = 0;
	bool right = 0;
	if(j > 0) {
	  left = rooms[i][j - 1];
	}
	if(j < R - 1) {
	  right = rooms[i][j + 1];
	}
	if(i > 0) {
	  up = rooms[i - 1][j];
	}
	if(i < R - 1) {
	  down = rooms[i + 1][j];
	}

	bool adjacentDoors[4] = {up, left, down, right};
      
	Block blocks[MAX_BLOCKS];
	/* BlocksContainer bc = {blocks, 0}; */
	// Top border
	blocks[0] = (Block){(Vector2){0, 0}, (Vector2){(screenWidth/2)-((doorSize/2)*adjacentDoors[0]), 10}};
	blocks[1] = (Block){(Vector2){(screenWidth/2)+((doorSize/2)*adjacentDoors[0]), 0}, (Vector2){(screenWidth/2)-((doorSize/2)*adjacentDoors[0]), 10}};
	// Left border
	blocks[2] = (Block){(Vector2){0, 0}, (Vector2){10, (screenHeight/2)-((doorSize/2)*adjacentDoors[1])}};
	blocks[3] = (Block){(Vector2){0, (screenHeight/2)+((doorSize/2)*adjacentDoors[1])}, (Vector2){10, (screenHeight/2)-((doorSize/2)*adjacentDoors[1])}};
	// Bottom border
	blocks[4] = (Block){(Vector2){0, screenHeight - 10}, (Vector2){(screenWidth/2)-((doorSize/2)*adjacentDoors[2]), 10}};
	blocks[5] = (Block){(Vector2){(screenWidth/2)+((doorSize/2)*adjacentDoors[2]), screenHeight - 10}, (Vector2){(screenWidth/2)-((doorSize/2)*adjacentDoors[2]), 10}};
	// Right border
	blocks[6] = (Block){(Vector2){screenWidth - 10, 0}, (Vector2){10, (screenHeight/2)-((doorSize/2)*adjacentDoors[3])}};
	blocks[7] = (Block){(Vector2){screenWidth - 10, (screenHeight/2)+((doorSize/2)*adjacentDoors[3])}, (Vector2){10, (screenHeight/2)-((doorSize/2)*adjacentDoors[3])}};
	Room room = {
	  blocks,
	  i,
	  j,
	  1
	};
	map[i][j] = room;
      }
    }
  }

  Room room = map[1][1];

  // set up raylib
  InitWindow(screenWidth, screenHeight, "Sprutte Game");
  SetTargetFPS(60);

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
    {
      // Player movement
      move(&player, room.blocks);

      player.shotCharge++;
      // Detect shooting, register new projectile
      playerShoot(&player, &pc);

      // Update each projectile
      updateProjectiles(&pc);

      // draw everything
      doDraw(
	     player.position,
	     player.radius,
	     pc.projectiles,
	     room.blocks);
    }

  // de-init
  CloseWindow();
  return 0;
}
