#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_PROJECTILES 50
#define MAX_BLOCKS 50
#define R 3
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450

typedef struct Projectile {
  Vector2 position;
  Vector2 speed;
  int radius;
  int lifeTime;
  bool enabled;
} Projectile;

typedef struct Player {
  Vector2 position;
  float speed;
  int radius;
  unsigned int firerate;
  unsigned int shotCharge;
  float shotSpeed;
} Player;

typedef struct ProjectilesContainer {
  Projectile *projectiles; // array
  int idx;
} ProjectilesContainer;

typedef struct Block {
  Vector2 start;
  Vector2 size;
} Block;

typedef struct BlocksContainer {
  Block *blocks; // array
  int idx;
} BlocksContainer;

typedef struct Room {
  Block *blocks;
  int x;
  int y;
  bool enabled;
} Room;

void doDraw(Vector2 playerPos, int playerRadius, Projectile projectiles[],
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
    if (p.enabled)
      DrawCircleV(p.position, p.radius, BLUE);
  }

  for (int i = 0; i < MAX_BLOCKS; i++) {
    Block b = blocks[i];
    DrawRectangle(b.start.x, b.start.y, b.size.x, b.size.y, YELLOW);
  }

  EndDrawing();
}

void shoot(float xSpeed, float ySpeed, Vector2 origin,
           ProjectilesContainer *pc) {
  /*
    Register a new projectile
   */
  Projectile *p = &(pc->projectiles[pc->idx]);
  p->position = origin;
  p->speed = (Vector2){xSpeed, ySpeed};
  p->radius = 5;
  p->lifeTime = 60;
  p->enabled = 1;
  pc->idx = (pc->idx + 1) % MAX_PROJECTILES;
}

void playerShoot(Player *player, ProjectilesContainer *pc) {
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

void updateProjectiles(ProjectilesContainer *pc) {
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    Projectile *p = &(pc->projectiles[i]);
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

int move(Player *player, Block blocks[], int roomIdx) {
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
  int forceX = 0;
  int forceY = 0;
  int rad = player->radius;

  for (int i = 0; i < MAX_BLOCKS; i++) {
    Block b = blocks[i];
    int bStartX = b.start.x;          // - player->radius;
    int bStartY = b.start.y;          // - player->radius;
    int bEndX = b.start.x + b.size.x; // + player->radius;
    int bEndY = b.start.y + b.size.y; // + player->radius;

    if (player->position.y < bEndY + rad &&
        player->position.y > bStartY - rad &&
        (newPos.x < bEndX + rad && newPos.x > bStartX - rad)) {
      // colliding from left or right
      if (newPos.y > bEndY) {
        // force down
        forceY = 1;
      } else if (newPos.y < bStartY) {
        // force up
        forceY = -1;
      } else {
        xAllowed = 0;
      }
    }

    if (player->position.x < bEndX + rad &&
        player->position.x > bStartX - rad &&
        (newPos.y < bEndY + rad && newPos.y > bStartY - rad)) {
      // colliding from up or down
      if (newPos.x > bEndX) {
        // force right
        forceX = 1;
      } else if (newPos.x < bStartX) {
        // force left
        forceX = -1;
      } else {
        yAllowed = 0;
      }
    }
  }
  if (xAllowed) {
    player->position.y = player->position.y + forceY * player->speed;
    player->position.x = newPos.x;

  }
  if (yAllowed) {
    player->position.y = newPos.y;
    player->position.x = player->position.x + forceX * player->speed;
  }

  if (player->position.x < 0) {
    return roomIdx - 1;
  } else if (player->position.x > SCREEN_WIDTH) {
    return roomIdx + 1;
  } else if (player->position.y < 0) {
    return roomIdx - R;
  } else if (player->position.y > SCREEN_HEIGHT) {
    return roomIdx + R;
  } else {
    return roomIdx;
  }
}

int main(void) {
  // init map values
  int playerRadius = 20;
  /* const int SCREEN_WIDTH  = 800; */
  /* const int SCREEN_HEIGHT = 450; */

  // init player values
  Player player = {{(float)SCREEN_WIDTH / 2, (float)SCREEN_HEIGHT / 2},
                   2.0f,
                   playerRadius,
                   8,
                   8,
                   5.0f};

  // init projectile values
  Projectile ps[50];
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    ps[i] = (Projectile){(Vector2){0, 0}, (Vector2){0, 0}, 0, 0, 0};
  }
  ProjectilesContainer pc = {ps, 0};

  int doorSize = 70;
  // generate map

  //
  bool rooms[R * R] = {0, 0, 1, 1, 1, 1, 0, 1, 1};
  Room *map = malloc((R * R) * sizeof *map);

  for (size_t i = 0; i < R; i++) {
    for (size_t j = 0; j < R; j++) {
      int realIdx = R * i + j;
      bool enabled = rooms[realIdx];
      if (!enabled) {
        map[realIdx] = (Room){NULL, i, j, 0};
      } else {
        bool up = 0;
        bool down = 0;
        bool left = 0;
        bool right = 0;
        if (realIdx % R != 0) {
          left = rooms[realIdx - 1];
        }
        if ((realIdx + 1) % R != 0) {
          right = rooms[realIdx + 1];
        }
        if (realIdx >= R) {
          up = rooms[realIdx - R];
        }
        if (realIdx < R * (R - 1)) {
          down = rooms[realIdx + R];
        }

        bool adjacentDoors[4] = {up, left, down, right};

        Block *blocks = malloc(MAX_BLOCKS * sizeof *blocks);
        // Top border
        blocks[0] = (Block){
            (Vector2){0, 0},
            (Vector2){(SCREEN_WIDTH / 2) - ((doorSize / 2) * adjacentDoors[0]),
                      10}};
        blocks[1] = (Block){
            (Vector2){(SCREEN_WIDTH / 2) + ((doorSize / 2) * adjacentDoors[0]),
                      0},
            (Vector2){(SCREEN_WIDTH / 2) - ((doorSize / 2) * adjacentDoors[0]),
                      10}};
        // Left border
        blocks[2] =
            (Block){(Vector2){0, 0},
                    (Vector2){10, (SCREEN_HEIGHT / 2) -
                                      ((doorSize / 2) * adjacentDoors[1])}};
        blocks[3] =
            (Block){(Vector2){0, (SCREEN_HEIGHT / 2) +
                                     ((doorSize / 2) * adjacentDoors[1])},
                    (Vector2){10, (SCREEN_HEIGHT / 2) -
                                      ((doorSize / 2) * adjacentDoors[1])}};
        // Bottom border
        blocks[4] = (Block){
            (Vector2){0, SCREEN_HEIGHT - 10},
            (Vector2){(SCREEN_WIDTH / 2) - ((doorSize / 2) * adjacentDoors[2]),
                      10}};
        blocks[5] = (Block){
            (Vector2){(SCREEN_WIDTH / 2) + ((doorSize / 2) * adjacentDoors[2]),
                      SCREEN_HEIGHT - 10},
            (Vector2){(SCREEN_WIDTH / 2) - ((doorSize / 2) * adjacentDoors[2]),
                      10}};
        // Right border
        blocks[6] =
            (Block){(Vector2){SCREEN_WIDTH - 10, 0},
                    (Vector2){10, (SCREEN_HEIGHT / 2) -
                                      ((doorSize / 2) * adjacentDoors[3])}};
        blocks[7] =
            (Block){(Vector2){SCREEN_WIDTH - 10,
                              (SCREEN_HEIGHT / 2) +
                                  ((doorSize / 2) * adjacentDoors[3])},
                    (Vector2){10, (SCREEN_HEIGHT / 2) -
                                      ((doorSize / 2) * adjacentDoors[3])}};
        blocks[8] = (Block){(Vector2){100, 100}, (Vector2){50, 50}};
        Room room = {blocks, i, j, 1};
        map[realIdx] = room;
      }
    }
  }

  int curRoom = R * R / 2;
  Room room = map[curRoom];

  // set up raylib
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sprutte Game");
  SetTargetFPS(60);

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    // Player movement
    int a = move(&player, room.blocks, curRoom);
    if (a != curRoom) {
      if (a == curRoom + 1) {
        player.position.x = 1;
      } else if (a == curRoom - 1) {
        player.position.x = SCREEN_WIDTH - 1;
      } else if (a == curRoom + R) {
        player.position.y = 1;
      } else if (a == curRoom - R) {
        player.position.y = SCREEN_HEIGHT - 1;
      }
      curRoom = a;
      room = map[curRoom];
    }

    player.shotCharge++;
    // Detect shooting, register new projectile
    playerShoot(&player, &pc);

    // Update each projectile
    updateProjectiles(&pc);

    // draw everything
    doDraw(player.position, player.radius, pc.projectiles, room.blocks);
  }

  // de-init
  free(map);
  CloseWindow();
  return 0;
}
