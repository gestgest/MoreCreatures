#ifndef ALMOND_H
#define ALMOND_H

#include <GameObject/GameObject.h>

// 식량 아이템 — 맵 위에 흩뿌려져 있고 플레이어가 닿으면 식량 게이지 회복.
//
// 현재 단계([3] 아몬드 랜덤 생성)에서는 "그리기 + 그림자"만 처리.
// 다음 단계([4] 상호작용)에서 collider를 붙이고, 닿으면 isActive=false로 꺼서
// 화면에서 사라지게 + HUD 식량 +1 처리할 예정.
//
// Mouse와 동일한 셰이더(mouseShader)를 재사용한다 — pos3+normal3+color3 = 9 floats per vertex.
// amond.obj는 Blender 씬 export라 "Floor"와 "Almond" 두 object가 같이 있어서
// Loader::loadModel(..., "Almond") 으로 "Almond"만 골라 가져온다.

class Almond : public GameObject {
public:
    Almond();
    Almond(Shader& shader, const glm::vec3& worldPos);
    ~Almond();

    void init(Shader* shaderPtr, const glm::vec3& worldPos);

    void setShadowMap(unsigned int& shadowMap);

    void drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::mat4 lightSpaceMatrix) override;
    void drawShadow(Shader& shader);
};

#endif
#pragma once
