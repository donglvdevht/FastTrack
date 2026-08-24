#include <SDL3/SDL.h>
#include <cstdio>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_stdinc.h>
#include <vector>
#include <string>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

struct SDLState
{
    SDL_Window *window;
    SDL_Renderer *renderer;
};

enum class PlayerState {
    IDLE,
    WALK,
    ATTACK,
    HURT,
    DEATH
};

void cleanup(SDLState &state)
{
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}

class Animation
{
    int frameCount, currentFrame;
    float frameTime, timer;
public:
    Animation() : frameTime(0), frameCount(0){}
    Animation(float frameTime, int frameCount) : frameTime(frameTime), frameCount(frameCount){}

    void update(float deltaTime) {
        timer += deltaTime;
        while (timer >= frameTime) {
            timer -= frameTime;
            currentFrame = (currentFrame + 1) % frameCount;
        }
    }
    
    void reset(){
        currentFrame = 0;
        timer = 0;
    }
};

class Timer
{
    float frameTime, timer;
public:
    Timer(float frameTime) : frameTime(frameTime), timer(0)
    {
    }

    void update(float deltaTime) {
        timer += deltaTime;
        while (timer >= frameTime) {
            timer -= frameTime;
        }
    }
};

struct GameObject
{
    std::vector<Animation> animations;
    int currentAnimation;
    SDL_Texture *texture;
    GameObject()
    {
        currentAnimation = -1;
        texture = nullptr;
    }
};

struct Resources
{
    const int ANIM_PLAYER_IDLE = 0;
    std::vector<Animation> playerAnims;
    std::vector<SDL_Texture *> texture;
    SDL_Texture *texIdle;
    SDL_Texture *loadTexture(SDL_Renderer *renderer, const std::string &filepatch)
    {
        // -- Load game assets
        SDL_Texture *tex = IMG_LoadTexture(renderer, filepatch.c_str());
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_LINEAR);
        texture.push_back(tex);
        return tex;
    }

    void load(SDLState &state)
    {
        playerAnims.resize(5);
        playerAnims[ANIM_PLAYER_IDLE] = Animation(16,3.2f);

        texIdle = loadTexture(state.renderer, "src/assets/players/AztecLeader/Spritesheeets/Front-Idle.png");
    }

    void unload()
    {
        for (SDL_Texture *tex:texture)
        {
            SDL_DestroyTexture(tex);
        }
    }
};

bool inittialize(SDLState &state);
void drawObject (const SDLState &state, GameObject &obj, float deltaTime, float texW, float texH);

int main() {
    // --- Create the window ---
    SDLState state;
    if (inittialize(state))
        return -1;

    Resources res;
    res.load(state);

    // --- Setup gamedata
    float texW, texH;
    SDL_GetTextureSize(res.texIdle, &texW, &texH); // lấy đúng kích thước thật của ảnh đã load
    GameObject player;
    player.texture = res.texIdle;
    player.animations = res.playerAnims;
    uint64_t prevTime = SDL_GetTicks();
    // --- Start the game loop ---
    bool running = true;
    while (running)
    {
        uint64_t nowTime = SDL_GetTicks();
        float deltaTime = (nowTime - prevTime) / 1000.0f;   // Chuyển sang s

        SDL_Event event{0};
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            default:
                break;
            }
        }

        // --- Vẽ ---
        SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
        SDL_RenderClear(state.renderer);

        drawObject(state, player, deltaTime, texW, texH);

        SDL_RenderPresent(state.renderer);
        prevTime = nowTime;
    }

    res.unload();
    cleanup(state);
    return 0;
}

bool inittialize(SDLState &state)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
        return 1;
    }
    state.window = SDL_CreateWindow("Swarm", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!state.window) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", nullptr);
        cleanup(state);
        return 1;
    }

    state.renderer = SDL_CreateRenderer(state.window, nullptr);
    if (!state.renderer)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", nullptr);
        cleanup(state);
        return 1;
    }

    int width = 800;
    int height = 600;
    SDL_SetRenderLogicalPresentation(state.renderer, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    return 0;
}

void drawObject (const SDLState &state, GameObject &obj, float deltaTime, float texW, float texH)
{
        const float SHEET_COLS = 4;
        const float SHEET_ROWS = 4;
        const float FRAME_W = texW / SHEET_COLS;
        const float FRAME_H = texH / SHEET_ROWS;
        const float DISPLAY_SIZE = 96.0f; // kích thước hiển thị mong muốn trên màn hình logic
        int currentFrame = 0;

        int currentCol = 0; //
        int currentRow = 0; //

        // --- srcRect: cắt frame từ ảnh gốc ---
        SDL_FRect srcRect = {
            currentCol * FRAME_W,   // x = cột * chiều rộng 1 frame
            currentRow * FRAME_H,   // y = hàng * chiều cao 1 frame
            FRAME_W,
            FRAME_H
        };

        // --- destRect: vị trí + kích thước hiển thị trên màn hình ---
        SDL_FRect destRect = {
            WINDOW_WIDTH / 2.0f,           // vị trí x nhân vật trong world
            WINDOW_HEIGHT / 2.0f,                          // vị trí y nhân vật trong world
            DISPLAY_SIZE,
            DISPLAY_SIZE
        };

        SDL_RenderTexture(state.renderer, obj.texture, &srcRect, &destRect);
}