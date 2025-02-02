#include "raymob.h"
#include "raymath.h"
#include <array>
#include <string>
#include <cmath>
#include <iomanip>
#include <sstream>

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
    uint8_t value;
};


class Button {
private:
    Rectangle body;
    Color color;
    bool clicked;
    float shrinkFactor;
    const float shrinkAmount = 10.0f;

public:
    Button(Vector2 pos, Color col)
            : color(col), clicked(false), shrinkFactor(0.0f) {
        body = (Rectangle){pos.x, pos.y, 120, 120};
    }

    bool isClicked() const {
        return CheckCollisionPointRec(GetMousePosition(), body) &&
               IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    }

    void draw() {
        Vector2 adjustedPos = {body.x, body.y};
        float adjustedWidth = body.width;
        float adjustedHeight = body.height;

        if (isClicked()) {
            clicked = true;
            shrinkFactor = shrinkAmount;
        }
        else {
            clicked = false;
            shrinkFactor = 0.0f;
        }

        adjustedWidth -= shrinkFactor;
        adjustedHeight -= shrinkFactor;
        adjustedPos.x += shrinkFactor / 2;
        adjustedPos.y += shrinkFactor / 2;

        DrawRectangleRounded((Rectangle){adjustedPos.x, adjustedPos.y, adjustedWidth, adjustedHeight}, 1, 0, Fade(color, 0.8));
    }

    Color getColor() const {
        return color;
    }
};


Font globalFont;


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

    static std::array<Button, 10> buttons = {
            (Button){(Vector2){100, 650}, BLACK},
            (Button){(Vector2){300, 650}, BROWN},
            (Button){(Vector2){500, 650}, RED},
            (Button){(Vector2){100, 850}, ORANGE},
            (Button){(Vector2){300, 850}, YELLOW},
            (Button){(Vector2){500, 850}, GREEN},
            (Button){(Vector2){100, 1050}, BLUE},
            (Button){(Vector2){300, 1050}, VIOLET},
            (Button){(Vector2){500, 1050}, GRAY},
            (Button){(Vector2){100, 450}, WHITE}
    };

    static std::array<Band, 4> bands = {
            (Band){(Rectangle){ body.x + 50, body.y, 70, body.height }, Fade(DARKGRAY, 0.5), false},
            (Band){(Rectangle){ body.x + 160, body.y, 70, body.height }, Fade(DARKGRAY, 0.5), false},
            (Band){(Rectangle){ body.x + 270, body.y, 70, body.height }, Fade(DARKGRAY, 0.5), false},
            (Band){(Rectangle){ body.width - 35, body.y, 70, body.height }, Fade(DARKGRAY, 0.5), false}
    };

    static Band* focusedBand = nullptr;

    for (auto& band : bands) {

        if (CheckCollisionPointRec(GetMousePosition(), band.body) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (band.focused) {
                band.color = Fade(DARKGRAY, 0.5);
                band.focused = false;
            }
            else {
                focusedBand = &band;
                band.focused = true;
                for (auto& b : bands) {
                    if (&b == &band) continue;
                    else b.focused = false;
                }
            }
        }

        DrawRectangleRec(band.body, Fade(band.color, 0.8));

        if (band.focused) {
            DrawRectangleLinesEx(band.body, 10, Fade(YELLOW, 1));
        }
    }

    for (uint8_t i = 0; i < buttons.size(); i++) {

        if (focusedBand != nullptr && buttons[i].isClicked()) {
            focusedBand->value = i;
            focusedBand->color = buttons[i].getColor();
        }

        buttons[i].draw();
    }

    static std::string current = "NaN";
    static std::string resistance = "NaN";

    if (ColorIsEqual(bands[0].color, Fade(DARKGRAY, 0.5)) ||
        ColorIsEqual(bands[1].color, Fade(DARKGRAY, 0.5)) ||
        ColorIsEqual(bands[2].color, Fade(DARKGRAY, 0.5))) {
            current = "NaN";
    }
    else {
        std::string firstDigit = std::to_string(bands[0].value);
        std::string secondDigit = std::to_string(bands[1].value);
        uint8_t base = std::stoi(firstDigit + secondDigit);
        auto multiplier = (uint64_t)std::pow(10, bands[2].value);
        uint64_t result = base * multiplier;

        if (result < 1000) current = std::to_string(result);
        else {
            const std::array<std::string, 5> suffixes = { "", "k", "M", "B", "T" };
            uint8_t suffixIndex = 0;
            auto num = (double)result;

            while (num >= 1000 && suffixIndex < 4) {
                num /= 1000;
                suffixIndex++;
            }

            if (num == std::floor(num)) {
                current = std::to_string((int)num) + suffixes[suffixIndex];
            }
            else {
                std::ostringstream out;
                out << std::fixed << std::setprecision(2) << num;
                current = out.str() + suffixes[suffixIndex];
            }
        }
    }

    if (ColorIsEqual(bands[3].color, WHITE)) {
        resistance = "10%";
    }
    else if (ColorIsEqual(bands[3].color, YELLOW)) {
        resistance = "5%";
    }
    else if (ColorIsEqual(bands[3].color, Fade(DARKGRAY, 0.5))) {
        resistance = "25%";
    }
    else {
        resistance = "NaN";
    }

    // screen
    DrawRectangleRounded((Rectangle){ 275, 400, 375, 200 }, 0.25, 0, Fade(DARKGRAY, 0.5));
    DrawTextEx(globalFont, current.c_str(), (Vector2){ 340, 400 }, 115, 0, Fade(WHITE, 0.8));
    DrawTextEx(globalFont, resistance.c_str(), (Vector2){ 340, 480 }, 115, 0, Fade(WHITE, 0.8));
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

    globalFont = LoadFontEx("Cubano.ttf", 126, NULL, 0);

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

    UnloadFont(globalFont);
    UnloadShader(shader);
    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}