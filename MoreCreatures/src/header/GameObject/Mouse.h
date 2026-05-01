#ifndef MOUSE_H
#define MOUSE_H

#include <GameObject/Creature.h>

class Mouse : public Creature {
public:
    Mouse();
    Mouse(Shader& shader, glm::vec3 color);
    ~Mouse();

    void init(Shader* shaderPtr, glm::vec3 color);

    //공유 forward: 외부에서 ShadowMap 세팅 시 mesh로 위임
    void setShadowMap(unsigned int& shadowMap);

    void drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::mat4 lightSpaceMatrix) override;

    void drawShadow(Shader& shader);
};
#endif
#pragma once
