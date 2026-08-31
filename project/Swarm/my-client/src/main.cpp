#include <cstdio>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_stdinc.h>
#include <array>
#include <vector>
#include <string>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <fstream>
#include "AnimationData.h"
#include "Animation.h"
#include "GameObject.h"
#include "Resources.h"
#include "SDLState.h"
#include "AnimationTypes.h"
#include "InputHandler.h"
using json = nlohmann::json;

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define MAP_ROWS 11
#define MAP_COL 20

const float TILE_SIZE = 100.0f;
const float PLAYER_SIZE = TILE_SIZE * 1.5f;

struct InputState
{
    bool left  = false;
    bool right = false;
    bool up    = false;
    bool down  = false;
    bool attack = false;
    bool jump  = false;
};

struct GameState
{
    // ground, static objects, characters
    std::array<std::vector<GameObject>, 3> layer;
    int playerIndex;

    GameState()
    {
        playerIndex = 0;
    }
};

void cleanup(SDLState &state);
bool initialize(SDLState &state);
void drawObject (const SDLState &state, GameObject &obj, float deltaTime);
void drawDecorationObject(const SDLState &state, GameObject &obj);
void drawMap(const SDLState &state, GameState &gs);
PlayerAnim resolvePlayerAnim(InputHandler& input, const GameObject& player, int currentAnimation);
void createTiles(const SDLState &state, GameState &gs, const Resources &res);

struct RenderItem
{
    float depth;
    bool isPlayer;
    GameObject* obj;
};

void sortRenderOrder(std::vector<RenderItem>& items)
{
    std::sort(items.begin(), items.end(), [](const RenderItem& a, const RenderItem& b) {
        return a.depth < b.depth;
    });
}

int main() {
    // --- Create the window ---
    SDLState state;
    if (initialize(state))
        return -1;

    Resources res;
    res.load(state);

    GameState gameState;
    createTiles(state, gameState, res);

    // --- Setup gamedata
    GameObject player;
    player.setPlayerAnimations(res.playerAnimations);
    player.setCurrentAnimation((int)PlayerAnim::IdleDown);
    player.setPosition(glm::vec2(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f));

    uint64_t prevTime = SDL_GetTicks();
    InputState input;
    InputHandler inputHandler;
    
    // --- Start the game loop ---
    bool running = true;
    while (running)
    {
        uint64_t nowTime = SDL_GetTicks();
        float deltaTime = (nowTime - prevTime) / 1000.0f;


        SDL_Event event{0};
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                case SDL_EVENT_KEY_DOWN:
                case SDL_EVENT_KEY_UP:
                {
                    inputHandler.handleEvent(event);
                    break;
                }
            }
        }

        glm::vec2 moveDir{0.0f, 0.0f};
        if (inputHandler.isHeld(InputHandler::Action::MoveUp))    moveDir.y -= 1.0f;
        if (inputHandler.isHeld(InputHandler::Action::MoveDown))  moveDir.y += 1.0f;
        if (inputHandler.isHeld(InputHandler::Action::MoveLeft))  moveDir.x -= 1.0f;
        if (inputHandler.isHeld(InputHandler::Action::MoveRight)) moveDir.x += 1.0f;

        if (glm::length(moveDir) > 0.0f) {
            player.move(glm::normalize(moveDir), deltaTime);
        }

        if (inputHandler.isPressed(InputHandler::Action::Attack)) {
            player.attack();
        }

        PlayerAnim nextAnimation;
        nextAnimation = resolvePlayerAnim(inputHandler, player, player.getCurrentAnimation());

        // --- Switch animation only when changing ---
        if ((int)nextAnimation != player.getCurrentAnimation())
        {
            player.setCurrentAnimation((int)nextAnimation);
            player.getPlayerAnimations()[player.getCurrentAnimation()].anim.reset();
        }

        // --- Vẽ ---
        SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
        SDL_RenderClear(state.renderer);

        drawMap(state, gameState);

        std::vector<RenderItem> renderOrder;
        for (GameObject& object : gameState.layer[1]) {
            const int animIndex = object.getCurrentAnimation();
            const float objDepth = object.getPosition().y + propConfigs[animIndex].footHeight;
            renderOrder.push_back({ objDepth, false, &object });
        }
        renderOrder.push_back({ player.getPosition().y + PLAYER_SIZE * 0.5f, true, &player });
        sortRenderOrder(renderOrder);

        for (const RenderItem& item : renderOrder) {
            if (item.isPlayer) {
                drawObject(state, *item.obj, deltaTime);
            } else {
                drawDecorationObject(state, *item.obj);
            }
        }

        SDL_RenderPresent(state.renderer);
        inputHandler.update();
        prevTime = nowTime;
    }

    res.unload();
    cleanup(state);
    return 0;
}

void cleanup(SDLState &state)
{
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}

bool initialize(SDLState &state)
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

    int width = 1920;
    int height = 1080;
    SDL_SetRenderLogicalPresentation(state.renderer, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    return 0;
}

PlayerAnim resolvePlayerAnim(InputHandler& input, const GameObject& player, int currentAnimation)
{
    const bool attacking = input.isHeld(InputHandler::Action::Attack);

    if (attacking) {
        switch (player.getFacingDirection()) {
            case Direction::Up:    return PlayerAnim::AttackUp;
            case Direction::Left:  return PlayerAnim::AttackLeft;
            case Direction::Right: return PlayerAnim::AttackRight;
            case Direction::Down:
            default:              return PlayerAnim::AttackDown;
        }
    }

    if (input.isHeld(InputHandler::Action::MoveUp))    return PlayerAnim::WalkUp;
    if (input.isHeld(InputHandler::Action::MoveDown))  return PlayerAnim::WalkDown;
    if (input.isHeld(InputHandler::Action::MoveLeft))  return PlayerAnim::WalkLeft;
    if (input.isHeld(InputHandler::Action::MoveRight)) return PlayerAnim::WalkRight;

    switch (player.getFacingDirection()) {
        case Direction::Up:    return PlayerAnim::IdleUp;
        case Direction::Left:  return PlayerAnim::IdleLeft;
        case Direction::Right: return PlayerAnim::IdleRight;
        case Direction::Down:
        default:              return PlayerAnim::IdleDown;
    }
}


void drawObject (const SDLState &state, GameObject &obj, float deltaTime)
{
        auto& animations = obj.getPlayerAnimations();
        int currentAnimation = obj.getCurrentAnimation();
    if (currentAnimation < 0 || currentAnimation >= static_cast<int>(animations.size())) {
        return;
    }
        AnimationInfo& info = animations[currentAnimation];
        // --- update animation theo deltaTime ---
        info.anim.update(deltaTime);

        int frameIndex = currentAnimation != -1
            ? info.anim.getCurrentFrame()
            : 0;

        int currentCol = frameIndex % info.columns;
        int currentRow = frameIndex / info.columns;

        // --- srcRect: cắt frame từ ảnh gốc ---
        SDL_FRect srcRect = {
            currentCol * info.frameWidth,   // x = cột * chiều rộng 1 frame
            currentRow * info.frameHeight,   // y = hàng * chiều cao 1 frame
            info.frameWidth,
            info.frameHeight
        };

        // --- destRect: vị trí + kích thước hiển thị trên màn hình ---
        glm::vec2 pos = obj.getPosition();
        SDL_FRect destRect = {
            pos.x - PLAYER_SIZE / 2.0f,    // vị trí x (centered)
            pos.y - PLAYER_SIZE / 2.0f,    // vị trí y (centered)
            PLAYER_SIZE,
            PLAYER_SIZE
        };

        SDL_RenderTexture(state.renderer, info.texture, &srcRect, &destRect);
}

void drawDecorationObject(const SDLState &state, GameObject &obj)
{
    auto& animations = obj.getDecorationAnimations();
    const int animIndex = obj.getCurrentAnimation();
    if (animIndex < 0 || animIndex >= static_cast<int>(animations.size())) {
        return;
    }

    AnimationInfo& info = animations[animIndex];
    if (!info.texture) {
        return;
    }

    const glm::vec2 position = obj.getPosition();
    SDL_FRect srcRect = {
        0,
        0,
        info.frameWidth,
        info.frameHeight
    };

    float targetSize = TILE_SIZE;
    if (animIndex >= 0 && animIndex < static_cast<int>(propConfigs.size())) {
        targetSize = propConfigs[animIndex].targetHeight;
    }

    SDL_FRect destRect{position.x, position.y, targetSize, targetSize};
    SDL_RenderTexture(state.renderer, info.texture, &srcRect, &destRect);
}

void drawMap(const SDLState &state, GameState &gs)
{
    for (GameObject& tile : gs.layer[0]) {
        auto& animations = tile.getGroundAnimations();
        const int animIndex = tile.getCurrentAnimation();
        if (animIndex < 0 || animIndex >= static_cast<int>(animations.size())) {
            continue;
        }

        AnimationInfo& info = animations[animIndex];
        if (!info.texture) {
            continue;
        }

        const glm::vec2 position = tile.getPosition();
        SDL_FRect srcRect = {
            0,
            0,
            info.frameWidth,
            info.frameHeight
        };
        SDL_FRect destRect{position.x, position.y, TILE_SIZE, TILE_SIZE};
        SDL_RenderTexture(state.renderer, info.texture, nullptr, &destRect);
    }

    for (GameObject& object : gs.layer[1]) {
        auto& animations = object.getDecorationAnimations();
        const int animIndex = object.getCurrentAnimation();
        if (animIndex < 0 || animIndex >= static_cast<int>(animations.size())) {
            continue;
        }

        AnimationInfo& info = animations[animIndex];
        if (!info.texture) {
            continue;
        }
        const glm::vec2 position = object.getPosition();
        SDL_FRect srcRect = {
            0,
            0,
            info.frameWidth,
            info.frameHeight
        };
        SDL_FRect destRect{position.x, position.y, propConfigs[animIndex].targetHeight, propConfigs[animIndex].targetHeight};

        SDL_RenderTexture(state.renderer, info.texture, nullptr, &destRect);
    }
}

void createTiles(const SDLState &state, GameState &gs, const Resources &res)
{
    std::ifstream file("src/map.json");
    if (!file.is_open()) {
        std::fprintf(stderr, "Failed to open map file: src/map.json\n");
        return;
    }

    json data;
    file >> data;
    int rows = data["rows"];
    int cols = data["cols"];

    const auto createGroundTile = [&res](int r, int c, ObjectType type, short value)
    {
        GameObject o;
        o.setType(type);
        o.setPosition(glm::vec2(c * TILE_SIZE, r * TILE_SIZE));
        o.setCurrentAnimation(value);
        o.setGroundAnimations(res.groundAnimations);
        return o;
    };

    const auto createDecorationTile = [&res](int r, int c, ObjectType type, short value)
    {
        GameObject o;
        o.setType(type);
        o.setPosition(glm::vec2(c * TILE_SIZE, r * TILE_SIZE));
        o.setCurrentAnimation(value);
        o.setDecorationAnimations(res.decorationAnimations);
        return o;
    };

    for (int r = 0; r < MAP_ROWS; r++)
    {
        for (int c = 0; c < MAP_COL; c++)
        {
            if (data["groundMap"][r][c] != 0) {
                gs.layer[0].push_back(createGroundTile(r, c, ObjectType::level, data["groundMap"][r][c]));
            }

            if (data["objectMap"][r][c] != 0) {
                gs.layer[1].push_back(createDecorationTile(r, c, ObjectType::level, data["objectMap"][r][c]));
            }
        }
    }
}