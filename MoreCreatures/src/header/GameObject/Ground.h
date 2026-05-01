#ifndef GROUND
#define GROUND

#include <GameObject/GameObject.h>

class Ground : public GameObject {

    unsigned int* texture = nullptr;

public:
    Ground();
    Ground(Shader& shader, glm::vec3 color);
    ~Ground();

    void initObject(Shader* shaderPtr, glm::vec3 color);

    void setTexture(unsigned int& texture);

    void setShadowMap(unsigned int& shadowMap);

    void drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::mat4 lightSpaceMatrix) override;

    void drawShadow(Shader& shader);
};

#endif
#pragma once
