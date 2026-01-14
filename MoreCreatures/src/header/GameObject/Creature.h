#ifndef CREATURE
#define CREATURE

#include <GameObject/GameObject.h>

class Creature : public GameObject {
    bool isGround;
public:
    Creature()
    {
        //initObject();
    }
    Creature(Shader& shader, glm::vec3 color) : GameObject(shader, color)
    {
        //initObject();
    }

    void drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos)
    {

    }

    void SetIsGround(bool isGround)
    {
        //std::cout << "Cd" << '\n';
        this->isGround = isGround;
    }
    bool GetIsGround()
    {
        return isGround;
    }
};
#endif
#pragma once