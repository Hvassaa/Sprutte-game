#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROJECTILES 50
#define MAX_BLOCKS 50
#define R 3
#define SCALE 2.0
#define WALL_THICKNESS (9 * SCALE)
#define BLOCK_SIZE (50 * SCALE)
#define DOORSIZE BLOCK_SIZE
#define STARTING_PLAYER_RADIUS ((BLOCK_SIZE / 2) - 10 * SCALE)
#define TILES_X 11
#define TILES_Y 7
#define SCREEN_WIDTH (BLOCK_SIZE * TILES_X + WALL_THICKNESS * 2)
#define SCREEN_HEIGHT (BLOCK_SIZE * TILES_Y + WALL_THICKNESS * 2)

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

// Maybe 11 x  7
typedef struct Block {
  Vector2 start;
  Vector2 size;
  bool enabled;
} Block;

typedef struct Room {
  Block *blocks;
  bool enabled;
  Color color;
} Room;

void doDraw(Player player, Player enemy, Projectile projectiles[], Room room) {
  /*
     Helper function to (re)draw everything, in the following order
     - background
     - map
     - player
     - enemy
     - projectiles
     - blocks
     */
  Vector2 playerPos = player.position;
  int playerRadius = player.radius;
  BeginDrawing();
  ClearBackground(room.color);
  // draw enemy
  DrawCircleV(enemy.position, enemy.radius - 1, BLACK);
  // draw player
  DrawCircleV(playerPos, playerRadius - 1, GREEN);
  // draw live projectiles
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    Projectile p = projectiles[i];
    if (p.enabled)
      DrawCircleV(p.position, p.radius, BLUE);
  }
  // draw border and other blocks
  for (int i = 0; i < 8 + TILES_X * TILES_Y; i++) {
    Color color;
    if (i < 8) {
      color = YELLOW;
    } else {
      color = GRAY;
    }
    Block b = room.blocks[i];
    DrawRectangle(b.start.x, b.start.y, b.size.x, b.size.y, color);
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
  p->radius = 5 * SCALE;
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

void updatePos(Player *player, Block *blocks, Vector2 newPos) {
  bool xAllowed = 1;
  bool yAllowed = 1;
  int forceX = 0;
  int forceY = 0;
  int rad = player->radius;

  for (int i = 0; i < 8 + TILES_X * TILES_Y; i++) {
    Block b = blocks[i];
    int bStartX = b.start.x;
    int bStartY = b.start.y;
    int bEndX = b.start.x + b.size.x;
    int bEndY = b.start.y + b.size.y;

    // colliding from left or right
    // if player center is not within Y-interval, allow sliding around corner
    bool playerOverBottomOfBlock = player->position.y < bEndY + rad;
    bool playerUnderTopOfBlock = player->position.y > bStartY - rad;
    bool newPosInsideXInterval =
        newPos.x < bEndX + rad && newPos.x > bStartX - rad;
    if (playerOverBottomOfBlock && playerUnderTopOfBlock &&
        newPosInsideXInterval) {
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
    bool newPosInsideYInterval =
        newPos.y < bEndY + rad && newPos.y > bStartY - rad;
    if (playerLeftOfRightBlockSide && playerRightOfLeftBlockSide &&
        newPosInsideYInterval) {
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
    bool movingLeftOrRightAndShouldSlide =
        ((player->position.x > newPos.x || player->position.x < newPos.x) &&
         forceY != 0);
    bool movingDownLeftOrRightAndShouldSlide =
        (player->position.y < newPos.y &&
         (player->position.x < newPos.x || player->position.x > newPos.x) &&
         forceY == 1);
    bool movingUpLeftOrRightAndShouldSlide =
        (player->position.y > newPos.y &&
         (player->position.x > newPos.x || player->position.x < newPos.x) &&
         forceY == -1);
    if (movingLeftOrRightAndShouldSlide ||
        movingDownLeftOrRightAndShouldSlide ||
        movingUpLeftOrRightAndShouldSlide) {
      yChange = player->position.y + forceY * player->speed;
      xChange = newPos.x;
    } else { // moving left or right, unhindered
      player->position.x = newPos.x;
    }
  }
  // allow moving on Y-axis
  if (yAllowed) {
    // check if sliding allowed
    bool movingUpOrDownAndShouldSlide =
        ((player->position.y > newPos.y || player->position.y < newPos.y) &&
         forceX != 0);
    bool movingRightUpOrDownAndShouldSlide =
        (player->position.x < newPos.x &&
         (player->position.y > newPos.y || player->position.y < newPos.y) &&
         forceX == 1);
    bool movingLeftUpOrDownAndShouldSlide =
        (player->position.x > newPos.x &&
         (player->position.y > newPos.y || player->position.y < newPos.y) &&
         forceX == -1);
    if (movingUpOrDownAndShouldSlide || movingRightUpOrDownAndShouldSlide ||
        movingLeftUpOrDownAndShouldSlide) {
      xChange = player->position.x + forceX * player->speed;
    } else { // moving up or down, unhindered
      player->position.y = newPos.y;
    }
  }

  if (xChange != 0) {
    player->position.x = xChange;
  }
  if (yChange != 0) {
    player->position.y = yChange;
  }
}

int playerMove(Player *player, Room room, int roomIdx) {
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

  updatePos(player, room.blocks, newPos);

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

void enemyMove(Player *enemy, Vector2 playerPos, Block *blocks) {
  float x = enemy->position.x;
  float y = enemy->position.y;

  float xDiff = playerPos.x - x;
  float yDiff = playerPos.y - y;
  int xSign = (xDiff > 0) - (xDiff < 0);
  int ySign = (yDiff > 0) - (yDiff < 0);

  Vector2 newPos = {(int)x + xSign * enemy->speed,
                    (int)y + ySign * enemy->speed};
  updatePos(enemy, blocks, newPos);
}

Block *makeWall(bool *adjacentDoors) {
  Block *blocks = malloc(8 * sizeof *blocks);
  blocks[0] = (Block){
      (Vector2){0, 0},
      (Vector2){(SCREEN_WIDTH / 2) - ((DOORSIZE / 2) * adjacentDoors[0]),
                WALL_THICKNESS}};
  blocks[1] = (Block){
      (Vector2){(SCREEN_WIDTH / 2) + ((DOORSIZE / 2) * adjacentDoors[0]), 0},
      (Vector2){(SCREEN_WIDTH / 2) - ((DOORSIZE / 2) * adjacentDoors[0]),
                WALL_THICKNESS}};
  // Left border
  blocks[2] = (Block){
      (Vector2){0, 0},
      (Vector2){WALL_THICKNESS,
                (SCREEN_HEIGHT / 2) - ((DOORSIZE / 2) * adjacentDoors[1])}};
  blocks[3] = (Block){
      (Vector2){0, (SCREEN_HEIGHT / 2) + ((DOORSIZE / 2) * adjacentDoors[1])},
      (Vector2){WALL_THICKNESS,
                (SCREEN_HEIGHT / 2) - ((DOORSIZE / 2) * adjacentDoors[1])}};
  // Bottom border
  blocks[4] = (Block){
      (Vector2){0, SCREEN_HEIGHT - WALL_THICKNESS},
      (Vector2){(SCREEN_WIDTH / 2) - ((DOORSIZE / 2) * adjacentDoors[2]),
                WALL_THICKNESS}};
  blocks[5] = (Block){
      (Vector2){(SCREEN_WIDTH / 2) + ((DOORSIZE / 2) * adjacentDoors[2]),
                SCREEN_HEIGHT - WALL_THICKNESS},
      (Vector2){(SCREEN_WIDTH / 2) - ((DOORSIZE / 2) * adjacentDoors[2]),
                WALL_THICKNESS}};
  // Right border
  blocks[6] = (Block){
      (Vector2){SCREEN_WIDTH - WALL_THICKNESS, 0},
      (Vector2){WALL_THICKNESS,
                (SCREEN_HEIGHT / 2) - ((DOORSIZE / 2) * adjacentDoors[3])}};
  blocks[7] = (Block){
      (Vector2){SCREEN_WIDTH - WALL_THICKNESS,
                (SCREEN_HEIGHT / 2) + ((DOORSIZE / 2) * adjacentDoors[3])},
      (Vector2){WALL_THICKNESS,
                (SCREEN_HEIGHT / 2) - ((DOORSIZE / 2) * adjacentDoors[3])}};
  return blocks;
}

Block makeBlock(int x, int y) {
  int realX = WALL_THICKNESS + x * BLOCK_SIZE;
  int realY = WALL_THICKNESS + y * BLOCK_SIZE;
  Block b = {
      (Vector2){realX, realY},
      (Vector2){BLOCK_SIZE, BLOCK_SIZE},
  };
  return b;
}

// read file with name "fname" and put it into buf
void readRoom(char *fname, char *buf) {
  const int tiles = TILES_X * TILES_Y;
  FILE *file;
  file = fopen("test.txt", "r");

  if (file == NULL) {
    perror("Failed reading file");
    exit(1);
  }

  fgets(buf, tiles + 1, file);
  fclose(file);
}

Room makeRoom(bool up, bool down, bool left, bool right, Color color) {
  bool adjacentDoors[4] = {up, left, down, right};
  Block *blocks = malloc((8 + TILES_X * TILES_Y) * sizeof *blocks);
  Block *walls = makeWall(adjacentDoors);
  for (int i = 0; i < 8; i++) {
    blocks[i] = walls[i];
  }
  char *room_buf = malloc((TILES_X * TILES_Y + 1) * sizeof *room_buf);
  readRoom("test.txt", room_buf);

  for (int i = 8; i < TILES_X * TILES_Y + 8; i++) {
    // read line
    bool enabled = room_buf[i - 8] == *"1";
    printf("%c\n", room_buf[i - 8]);
    if (enabled) {
      int x = (i - 8) % TILES_X;
      int y = (int)floor((i - 8) / TILES_X);
      blocks[i] = makeBlock(x, y);
    }
  }
  free(room_buf);
  Room room = {blocks, 1, color};
  return room;
}

int main(void) {
  // init map values
  int playerRadius = STARTING_PLAYER_RADIUS;

  // init player values
  Player player = {{(float)SCREEN_WIDTH / 2, (float)SCREEN_HEIGHT / 2},
                   2.0f * SCALE,
                   (playerRadius),
                   8,
                   8,
                   5.0f * SCALE};

  Player enemy = {{(float)SCREEN_WIDTH / 1.5, (float)SCREEN_HEIGHT / 1.5},
                  1.0f * SCALE,
                  (playerRadius),
                  8,
                  8,
                  5.0f * SCALE};

  // init projectile values
  Projectile ps[50];
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    ps[i] = (Projectile){(Vector2){0, 0}, (Vector2){0, 0}, 0, 0, 0};
  }
  ProjectilesContainer pc = {ps, 0};

  // generate map
  // enabled rooms
  bool rooms[R * R] = {0, 0, 1, 1, 1, 1, 0, 1, 1};
  // Color roomCols[R * R] = {BLACK,   BLACK, LIGHTGRAY, PINK,  BEIGE,
  //                          MAGENTA, BLACK, MAROON,    VIOLET};
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
        Room room = makeRoom(up, down, left, right, RED);
        map[realIdx] = room;
      }
    }
  }

  int curRoom = R * R / 2;
  Room room = map[curRoom];

  // set up raylib
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sprutte Game");
  /* ToggleFullscreen(); */
  SetTargetFPS(60);

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    // Player movement
    int a = playerMove(&player, room, curRoom);
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

    // enemy movement
    enemyMove(&enemy, player.position, room.blocks);
    // draw everything
    doDraw(player, enemy, pc.projectiles, room);
  }

  // de-init
  // How much should be freed???
  for (int i = 0; i < R * R; i++) {
    free(map[i].blocks);
  }
  free(map);
  CloseWindow();
  return 0;
}
