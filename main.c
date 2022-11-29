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
  Color color;
} Room;

void doDraw(Vector2 playerPos, int playerRadius, Projectile projectiles[],
            Room room) {
  /*
    Helper function to (re)draw everything, in the following order
    - background
    - map
    - player
    - projectiles
    - blocks
   */
  BeginDrawing();
  ClearBackground(room.color);
  // draw player
  DrawCircleV(playerPos, playerRadius - 1, GREEN);
  // draw live projectiles
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    Projectile p = projectiles[i];
    if (p.enabled)
      DrawCircleV(p.position, p.radius, BLUE);
  }
  // draw border and other blocks
  for (int i = 0; i < MAX_BLOCKS; i++) {
    Block b = room.blocks[i];
    DrawRectangle(b.start.x, b.start.y, b.size.x, b.size.y, YELLOW);
  }

  DrawFPS(11, 11);
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

void resetProjectiles(ProjectilesContainer *pc) {
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    Projectile *p = &(pc->projectiles[i]);
    p->enabled = 0;
  }
}

int move(Player *player, Room room, int roomIdx) {
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
    Block b = room.blocks[i];
    int bStartX = b.start.x;         
    int bStartY = b.start.y;         
    int bEndX = b.start.x + b.size.x;
    int bEndY = b.start.y + b.size.y;

    // colliding from left or right
    // if player center is not within Y-interval, allow sliding around corner
    bool playerOverBottomOfBlock = player->position.y < bEndY + rad;
    bool playerUnderTopOfBlock = player->position.y > bStartY - rad;
    bool newPosInsideXInterval = newPos.x < bEndX + rad && newPos.x > bStartX - rad;
    if (playerOverBottomOfBlock && playerUnderTopOfBlock && newPosInsideXInterval) {
      bool playerCenterBelowBlock = newPos.y > bEndY;
      bool playerCenterAboveBlock = newPos.y < bStartY;
      if (playerCenterBelowBlock) {
        // force down
        forceY = 1;
      } else if (playerCenterAboveBlock) {
        // force up
        forceY = -1;
      } else {
        xAllowed = 0;
      }
    }

    // colliding from top or bottom
    // if player center is not within X-interval, allow sliding around corner
    bool playerLeftOfRightBlockSide = player->position.x < bEndX + rad;
    bool playerRightOfLeftBlockSide = player->position.x > bStartX - rad;
    bool newPosInsideYInterval = newPos.y < bEndY + rad && newPos.y > bStartY - rad;
    if (playerLeftOfRightBlockSide && playerRightOfLeftBlockSide && newPosInsideYInterval) {
      bool playerCenterRightOfBlock = newPos.x > bEndX;
      bool playerCenterLeftOfBlock = newPos.x < bStartX;
      if (playerCenterRightOfBlock) {
        // force right
        forceX = 1;
      } else if (playerCenterLeftOfBlock) {
        // force left
        forceX = -1;
      } else {
        yAllowed = 0;
      }
    }
  }

  float yChange = 0;
  float xChange = 0;
  // allow moving on X-axis
  if (xAllowed) {
    // check if sliding allowed
    bool movingLeftOrRightAndShouldSlide = ((player->position.x > newPos.x || player->position.x < newPos.x) && forceY != 0);
    bool movingDownLeftOrRightAndShouldSlide = (player->position.y < newPos.y && (player->position.x < newPos.x || player->position.x > newPos.x) && forceY == 1);
    bool movingUpLeftOrRightAndShouldSlide = (player->position.y > newPos.y && (player->position.x > newPos.x || player->position.x < newPos.x) && forceY == -1);
    if (movingLeftOrRightAndShouldSlide || movingDownLeftOrRightAndShouldSlide || movingUpLeftOrRightAndShouldSlide) {
      yChange = player->position.y + forceY * player->speed;
      xChange = newPos.x;
    }
    else { // moving left or right, unhindered
      player->position.x = newPos.x;
    }
  }
  // allow moving on Y-axis
  if (yAllowed) {
    // check if sliding allowed
    bool movingUpOrDownAndShouldSlide = ((player->position.y > newPos.y || player->position.y < newPos.y) && forceX != 0);
    bool movingRightUpOrDownAndShouldSlide = (player->position.x < newPos.x && (player->position.y > newPos.y || player->position.y < newPos.y) && forceX == 1);
    bool movingLeftUpOrDownAndShouldSlide = (player->position.x > newPos.x && (player->position.y > newPos.y || player->position.y < newPos.y) && forceX == -1);
    if (movingUpOrDownAndShouldSlide || movingRightUpOrDownAndShouldSlide ||movingLeftUpOrDownAndShouldSlide) {
      xChange = player->position.x + forceX * player->speed;
    }
    else { // moving up or down, unhindered
      player->position.y = newPos.y;
    }
  }

  if (xChange != 0) {
    player->position.x = xChange;
  }
  if (yChange != 0) {
    player->position.y = yChange;
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

  // generate map
  int doorSize = 70;
  // enabled rooms
  bool rooms[R * R] = {0, 0, 1, 1, 1, 1, 0, 1, 1};
  Color roomCols[R * R] = {
    BLACK,
    BLACK,
    LIGHTGRAY,
    PINK,
    BEIGE,
    MAGENTA,
    BLACK,
    MAROON,
    VIOLET
  };
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
        Room room = {blocks, i, j, 1, roomCols[realIdx]};
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
    int a = move(&player, room, curRoom);
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
      resetProjectiles(&pc);
    }

    player.shotCharge++;
    // Detect shooting, register new projectile
    playerShoot(&player, &pc);

    // Update each projectile
    updateProjectiles(&pc);

    // draw everything
    doDraw(player.position, player.radius, pc.projectiles, room);
  }

  // de-init
  free(map);
  CloseWindow();
  return 0;
}
