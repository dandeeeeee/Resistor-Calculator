#include "raymob.h"
#include "raymath.h"
#include <array>


#define MIN(a,b) (((a)<(b))?(a):(b))

constexpr Color bgColor = (Color){16, 20, 44, 255};
constexpr uint16_t gameScreenWidth = 720;
constexpr uint16_t gameScreenHeight = 1280;


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

    float bit = pow(2.0, floor(32.0 - uv.x));
    float i = 1.0;

    if (int(uv.y) == 2) i = float(928473456) / bit;
    if (int(uv.y) == 1) i = float(626348112) / bit;
    if (int(uv.y) == 0) i = float(1735745872) / bit;

    return mod(i, 2.0);
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


struct Band
{
    Rectangle body;
    Color color;
    bool focused;
};


struct Circle
{
    Vector2 center;
    float radius;
};


class Button
{
private:
    Circle body;
    Color color;
    bool clicked;

public:
    Button(Vector2 pos, Color col)
        : color(col)
    {
        body = (Circle){pos, 5};
    }

    bool isClicked() const
    {
        return CheckCollisionPointCircle(GetMousePosition(), body.center, body.radius) &&
                IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    }
};

void drawResistor()
{
    constexpr Rectangle body = (Rectangle){ 80, 75, gameScreenWidth - 160, 250 };

    // left leg
    DrawRectangle(10, (int)(body.y + body.height / 2) - 25, (int)body.x - 30, 50, Fade(RAYWHITE, 0.5));
    // right leg
    DrawRectangle((int)body.width + 100, (int)(body.y + body.height / 2) - 25, (int)body.x - 30, 50, Fade(RAYWHITE, 0.5));
    // outline
    DrawRectangleRoundedLinesEx(body, 0.15, 0, 20, Fade(RAYWHITE, 0.5));
    // body
    DrawRectangleRounded(body, 0.15, 0, Fade(bgColor, 0.5));

    constexpr std::array<Color, 10> colors = {
            BLACK,
            BROWN,
            RED,
            ORANGE,
            YELLOW,
            GREEN,
            BLUE,
            VIOLET,
            GRAY,
            WHITE,
    };

    static std::array<Band, 4> bands = {
            (Band){(Rectangle){ body.x + 50, body.y, 70, body.height }, BLACK, false},
            (Band){(Rectangle){ body.x + 160, body.y, 70, body.height }, BLACK, false},
            (Band){(Rectangle){ body.x + 270, body.y, 70, body.height }, BLACK, false},
            (Band){(Rectangle){ body.width - 35, body.y, 70, body.height }, BLACK, false}
    };

    for (auto& band : bands){
        DrawRectangleRec(band.body, Fade(band.color, 0.65));
    }


}



int main()
{
    InitWindow(0, 0, "RESISTORR");
    SetTargetFPS(60);

    RenderTexture2D target = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

    //Texture2D resistor = LoadTexture("resistor.png");

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

        DrawRectangleRounded((Rectangle){10, 0, (float)gameScreenWidth - 20, (float)gameScreenHeight},
                             0.15, 0, Fade(bgColor, 0.8));


        //DrawTexturePro(resistor, (Rectangle){0, 0, (float)resistor.width, (float)resistor.height},
        //               (Rectangle){35, 0, 650, 400}, Vector2Zero(), 0, WHITE);

        drawResistor();

        EndTextureMode();



        BeginDrawing();

        ClearBackground(BLACK);

        BeginShaderMode(shader);

        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);

        EndShaderMode();

        DrawTexturePro(target.texture, (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
                       (Rectangle){((float)GetScreenWidth() - ((float)gameScreenWidth*scale))*0.5f,
                                   ((float)GetScreenHeight() - ((float)gameScreenHeight*scale))*0.5f,
                                   (float)gameScreenWidth*scale, (float)gameScreenHeight*scale},
                                   (Vector2){ 0, 0 }, 0.0f, WHITE);

        EndDrawing();
    }

    UnloadShader(shader);
    //UnloadTexture(resistor);
    CloseWindow();
    return 0;
}