#ifndef CREATURE
#define CREATURE

#include <GameObject/GameObject.h>

class Creature : public GameObject {
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
};
#endif
#pragma once