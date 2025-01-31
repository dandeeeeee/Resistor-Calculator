#include "raymob.h"
#include "raymath.h"


#define MIN(a,b) (((a)<(b))?(a):(b))

const Color bgColor = (Color){16, 20, 44, 255};


const char* vertexShader = R"(
#version 100
precision mediump float;

attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;

uniform mat4 mvp;

varying vec2 fragTexCoord;

void main() {
    fragTexCoord = vertexTexCoord;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)";

const char* fragmentShader = R"(
#version 100
precision mediump float;

const float PI = 3.14159265359;

uniform vec2 resolution;
uniform float iTime;

varying vec2 fragTexCoord;

float message(vec2 uv) {
    uv -= vec2(1.0, 10.0);
    if (uv.x < 0.0 || uv.x >= 32.0 || uv.y < 0.0 || uv.y >= 3.0) return -1.0;

    float bit = pow(2.0, floor(32.0 - uv.x));  // Ensure floating-point math
    float i = 1.0;

    if (int(uv.y) == 2) i = float(928473456) / bit;
    if (int(uv.y) == 1) i = float(626348112) / bit;
    if (int(uv.y) == 0) i = float(1735745872) / bit;

    return mod(i, 2.0);  // Use mod() instead of integer subtraction
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    float c = message(uv);
    if (c >= 0.0) {
        gl_FragColor = vec4(c, c, c, 1.0);
        return;
    }

    gl_FragColor = vec4(uv, 0.5 + 0.5 * sin(iTime), 1.0);
}
)";



int main()
{
    InitWindow(0, 0, "RESISTORR");
    SetTargetFPS(60);

    const uint16_t gameScreenWidth = 720;
    const uint16_t gameScreenHeight = 1280;
    RenderTexture2D target = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

    Texture2D resistor = LoadTexture("resistor.png");

    Shader shader = LoadShaderFromMemory(vertexShader, fragmentShader);

    if (shader.id == 0) {
        TraceLog(LOG_ERROR, "Shader failed to load!");
        return -1;
    }

    int resolutionLoc = GetShaderLocation(shader, "resolution");
    int iTimeLoc = GetShaderLocation(shader, "iTime");

    Vector2 resolution = (Vector2){(float)GetScreenWidth(), (float)GetScreenHeight()};
    SetShaderValue(shader, resolutionLoc, &resolution, SHADER_UNIFORM_VEC2);

    float iTime = 0.0f;


    while (!WindowShouldClose())
    {
        iTime += GetFrameTime();
        SetShaderValue(shader, iTimeLoc, &iTime, SHADER_UNIFORM_FLOAT);

        float scale = MIN((float)GetScreenWidth()/gameScreenWidth, (float)GetScreenHeight()/gameScreenHeight);

        Vector2 mouse = GetMousePosition();
        Vector2 virtualMouse = { 0 };
        virtualMouse.x = (mouse.x - ((float)GetScreenWidth() - ((float)gameScreenWidth*scale))*0.5f)/scale;
        virtualMouse.y = (mouse.y - ((float)GetScreenHeight() - ((float)gameScreenHeight*scale))*0.5f)/scale;
        virtualMouse = Vector2Clamp(virtualMouse, (Vector2){ 0, 0 }, (Vector2){ (float)gameScreenWidth, (float)gameScreenHeight });

        SetMouseOffset(static_cast<int>(-((float)GetScreenWidth() - ((float)gameScreenWidth*scale))*0.5f),
                       static_cast<int>(-((float)GetScreenHeight() - ((float)gameScreenHeight*scale))*0.5f));
        SetMouseScale(1/scale, 1/scale);



        BeginTextureMode(target);

        ClearBackground(BLANK);

        DrawRectangleRounded((Rectangle){10, 0, (float)gameScreenWidth - 20, (float)gameScreenHeight}, 0.15, 0, Fade(bgColor, 0.9));


        DrawTexturePro(resistor, (Rectangle){0, 0, (float)resistor.width, (float)resistor.height},
                       (Rectangle){35, 0, 650, 400}, Vector2Zero(), 0, WHITE);

        EndTextureMode();



        BeginDrawing();

        ClearBackground(BLACK);

        BeginShaderMode(shader);

        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);

        EndShaderMode();

        DrawTexturePro(target.texture, (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
                       (Rectangle){ ((float)GetScreenWidth() - ((float)gameScreenWidth*scale))*0.5f, ((float)GetScreenHeight() - ((float)gameScreenHeight*scale))*0.5f,
                                    (float)gameScreenWidth*scale, (float)gameScreenHeight*scale }, (Vector2){ 0, 0 }, 0.0f, WHITE);

        EndDrawing();
    }

    UnloadShader(shader);
    UnloadTexture(resistor);
    CloseWindow();
    return 0;
}