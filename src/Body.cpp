#include "Body.h"
#include "ScreenLog.h"
Body::Body() : speed(0.0f), accelerating(false), damping(0.98f), maxSpeed(10.0f), timeToReachMaxSpeed(5.0f) {}

void Body::applyForce() { accelerating = true; }

float Body::update(float dt) {
    if (accelerating) {
        float target = maxSpeed;
        float acceleration = std::abs(target) / timeToReachMaxSpeed;

        if (speed != target) {
            speed += (target > speed ? 1.0f : -1.0f) * acceleration * dt;
        }

        if ((target >= 0 && speed > target) || (target < 0 && speed < target)) {
            speed = target;
        }
    } else {
        speed *= damping;
    }
    accelerating = false;
    return speed;
}


void Body::resetForce() { 
    accelerating = false; 
    speed = 0;
}
