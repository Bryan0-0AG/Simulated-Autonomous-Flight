#include "physics/motion.h"

void update_position(float& position, float velocity, float dt) {
    position += velocity * dt;
}