#include "physics/motion.h"
#include "physics/physics_config.h"
#include "utils/math_utils.h"
#include "utils/vector2.h"

void update_physics(Vector2& pos, Vector2& vel, float dt) {
    vel.y -= GRAVITY * dt;
    pos.x += vel.x * dt;
    pos.y += vel.y * dt;
}