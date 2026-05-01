#ifndef CREATURE
#define CREATURE

#include <GameObject/GameObject.h>

class Creature : public GameObject {
    bool isGround;

protected:
    glm::vec3 front;
public:
    Creature();

    void SetIsGround(bool isGround);
    bool GetIsGround();

    void setFront(glm::vec3 front);
};
#endif
#pragma once
