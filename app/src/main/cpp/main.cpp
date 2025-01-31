#include "raymob.h"
#include "raymath.h"

int main()
{

    InitWindow(0, 0, "RESISTORR");
    SetTargetFPS(60);

    Texture2D resistor = LoadTexture("resistor.png");

    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(BLACK);

        DrawTexturePro(resistor, (Rectangle){0, 0, (float)resistor.width, (float)resistor.height},
                       (Rectangle){150, 350, 800, 500}, Vector2Zero(), 0, WHITE);

        EndDrawing();
    }

    UnloadTexture(resistor);
    CloseWindow();
    return 0;
}