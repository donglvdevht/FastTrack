#include "Resources.h"
#include "AnimationTypes.h"
#include <cstdio>
#include <SDL3_image/SDL_image.h>
#include "AnimationData.h"
#include "Animation.h"
#include <iomanip>
#include <sstream>

std::string intToString2(int number)
{
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << number;
    return ss.str();
}

SDL_Texture* Resources::loadTexture(SDL_Renderer *renderer, const std::string &filepath)
{
    SDL_Texture *tex = IMG_LoadTexture(renderer, filepath.c_str());
    if (!tex) {
        fprintf(stderr, "Failed to load texture: %s\n", filepath.c_str());
        return nullptr;
    }
    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_LINEAR);
    textures.push_back(tex);
    return tex;
}

void Resources::loadAnim(SDL_Renderer* renderer, std::vector<AnimationInfo>& destination,
                         int index, const std::string& path,
                         float frameTime, int frameCount, int cols, int rows)
{
    AnimationInfo info{};
    info.texture = loadTexture(renderer, path);
    if (!info.texture) {
        fprintf(stderr, "Skipping invalid texture: %s\n", path.c_str());
        return;
    }

    info.anim    = Animation(frameTime, frameCount);
    info.columns = cols;
    info.rows    = rows;

    float w, h;
    SDL_GetTextureSize(info.texture, &w, &h);
    info.frameWidth  = w / cols;
    info.frameHeight = h / rows;

    destination[index] = info;
}

void Resources::loadPlayerAnimations(SDL_Renderer *renderer)
{
    // Pre-allocate vector for all playerAnimations
    playerAnimations.resize((int)PlayerAnim::Count);
    
    std::string path;

    // ===== IDLE DOWN =====
    path = "src/assets/players/AztecLeader/Spritesheeets/Front - Idle.png";
    loadAnim(renderer, playerAnimations, (int)PlayerAnim::IdleDown, path, 0.08f, 16, 4, 4);

    // ===== IDLE UP =====
    path = "src/assets/players/AztecLeader/Spritesheeets/Back - Idle.png";
    loadAnim(renderer, playerAnimations, (int)PlayerAnim::IdleUp, path, 0.08f, 16, 4, 4);

    // ===== IDLE LEFT =====
    path = "src/assets/players/AztecLeader/Spritesheeets/Left - Idle.png";
    loadAnim(renderer, playerAnimations, (int)PlayerAnim::IdleLeft, path, 0.08f, 16, 4, 4);

    // ===== IDLE RIGHT =====
    path = "src/assets/players/AztecLeader/Spritesheeets/Right - Idle.png";
    loadAnim(renderer, playerAnimations, (int)PlayerAnim::IdleRight, path, 0.08f, 16, 4, 4);

    // ===== WALK DOWN =====
    path = "src/assets/players/AztecLeader/Spritesheeets/Front - Walking.png";
    loadAnim(renderer, playerAnimations, (int)PlayerAnim::WalkDown, path, 0.04f, 20, 4, 5);

    // ===== WALK UP =====
    path = "src/assets/players/AztecLeader/Spritesheeets/Back - Walking.png";
    loadAnim(renderer, playerAnimations, (int)PlayerAnim::WalkUp, path, 0.04f, 20, 4, 5);

    // ===== WALK LEFT =====
    path = "src/assets/players/AztecLeader/Spritesheeets/Left - Walking.png";
    loadAnim(renderer, playerAnimations, (int)PlayerAnim::WalkLeft, path, 0.04f, 20, 4, 5);

    // ===== WALK RIGHT =====
    path = "src/assets/players/AztecLeader/Spritesheeets/Right - Walking.png";
    loadAnim(renderer, playerAnimations, (int)PlayerAnim::WalkRight, path, 0.04f, 20, 4, 5);
}

void Resources::loadGroundAnimations(SDL_Renderer *renderer)
{
    const int tileCount = (int)GroundAnim::Count;
    groundAnimations.resize(tileCount + 1);

    std::string path;
    for (int i = 1; i <= tileCount; ++i)
    {
        path = "src/assets/map/Top-Down Simple Summer_Ground "
            + intToString2(i) + ".png";

        loadAnim(
            renderer,
            groundAnimations,
            i,
            path,
            0, 1, 1, 1
        );
    }
}

void Resources::loadDecorationAnimations(SDL_Renderer *renderer)
{
    static const std::vector<std::string> propFiles = {
        "src/assets/map/Top-Down Simple Summer_Prop - Tree Small.png",      // 1
        "src/assets/map/Top-Down Simple Summer_Prop - Tree Medium.png",     // 2
        "src/assets/map/Top-Down Simple Summer_Prop - Tree Large.png",      // 3
        "src/assets/map/Top-Down Simple Summer_Prop - Bushes Small.png",    // 4
        "src/assets/map/Top-Down Simple Summer_Prop - Bushes Medium.png",
        "src/assets/map/Top-Down Simple Summer_Prop - Bushes Large.png",
        "src/assets/map/Top-Down Simple Summer_Prop - Tent.png",
        "src/assets/map/Top-Down Simple Summer_Prop - Campfire.png",
        "src/assets/map/Top-Down Simple Summer_Prop - Magic Stone Tower.png",
        "src/assets/map/Top-Down Simple Summer_Prop - Well.png"
    };

    decorationAnimations.resize(propFiles.size());
    for (size_t i = 0; i < propFiles.size(); ++i)
    {
        loadAnim(renderer, decorationAnimations, static_cast<int>(i), propFiles[i], 0, 1, 1, 1);
    }
}

void Resources::load(SDLState &state)
{
    loadPlayerAnimations(state.renderer);
    loadGroundAnimations(state.renderer);
    loadDecorationAnimations(state.renderer);
}

void Resources::unload()
{
    for (SDL_Texture *tex : textures)
    {
        SDL_DestroyTexture(tex);
    }
}
