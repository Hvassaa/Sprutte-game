#include "raylib.h"

int main(void)
{

  // init vals
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
  Texture2D squid = LoadTexture("squid.png");
  SetTargetFPS(60);

  Vector2 playerPos = { (float)screenWidth/2, (float)screenHeight/2 };
  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
    {
      // controls
      if (IsKeyDown(KEY_D))playerPos.x += 2.0f;
      if (IsKeyDown(KEY_A)) playerPos.x -= 2.0f;
      if (IsKeyDown(KEY_W))   playerPos.y -= 2.0f;
      if (IsKeyDown(KEY_S)) playerPos.y += 2.0f;

      // Draw
      BeginDrawing();
      ClearBackground(RAYWHITE);
      DrawTextureV(squid, playerPos, WHITE);      
      EndDrawing();
    }

  // de-init
  UnloadTexture(squid);
  CloseWindow();
  return 0;
}
