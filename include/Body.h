#pragma once

#pragma once

class Body {

public:
    bool accelerating;
    float speed;
    float damping;
    float maxSpeed;
    float timeToReachMaxSpeed;

    Body();

    void applyForce();
    float update(float dt);
    void resetForce();
};
