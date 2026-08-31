#include "GameObject.h"

GameObject::GameObject()
    : currentAnimation(-1),
      position(0.0f, 0.0f),
      type(ObjectType::player),
      facingDirection(Direction::Down)
{
}

void GameObject::moveUp(float deltaTime)
{
    position.y -= MOVE_SPEED * deltaTime;
    facingDirection = Direction::Up;
}

void GameObject::moveDown(float deltaTime)
{
    position.y += MOVE_SPEED * deltaTime;
    facingDirection = Direction::Down;
}

void GameObject::moveLeft(float deltaTime)
{
    position.x -= MOVE_SPEED * deltaTime;
    facingDirection = Direction::Left;
}

void GameObject::moveRight(float deltaTime)
{
    position.x += MOVE_SPEED * deltaTime;
    facingDirection = Direction::Right;
}

void GameObject::attack()
{
    
}

void GameObject::move(const glm::vec2& direction, float deltaTime)
{
    if (glm::length(direction) < 0.0001f) {
        return;
    }

    position += direction * MOVE_SPEED * deltaTime;

    if (std::abs(direction.x) > std::abs(direction.y)) {
        facingDirection = direction.x > 0.0f ? Direction::Right : Direction::Left;
    } else {
        facingDirection = direction.y > 0.0f ? Direction::Down : Direction::Up;
    }
}