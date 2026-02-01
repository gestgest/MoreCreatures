#ifndef CREATURE
#define CREATURE

#include <GameObject/GameObject.h>

class Creature : public GameObject {
    bool isGround;

protected:
    glm::vec3 front;
public:
    Creature()
    {
        front = glm::vec3(0, 0, 0);
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

    void setFront(glm::vec3 front)
    {
        this->front = front;
    }
};
#endif
#pragma once