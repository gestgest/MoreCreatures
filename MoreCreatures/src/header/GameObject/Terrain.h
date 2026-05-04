#ifndef TERRAIN_H
#define TERRAIN_H

#include <GameObject/GameObject.h>

class Terrain : public GameObject {

    unsigned int* texture = nullptr;

    int gridSize = 128;       // NxN cells (정점은 (N+1)x(N+1))
    float cellSize = 1.0f;    // 한 칸당 월드 단위
    float heightScale = 8.0f; // 최대 언덕 높이
    float noiseScale = 0.05f; // 노이즈 주파수 (작을수록 완만)
    int   octaves = 5;        // FBm 옥타브 수
    int   seed = 1337;

public:
    Terrain();
    Terrain(Shader& shader, glm::vec3 color);
    ~Terrain();

    void initObject(Shader* shaderPtr, glm::vec3 color);

    void setTexture(unsigned int& texture);
    void setShadowMap(unsigned int& shadowMap);

    void drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::mat4 lightSpaceMatrix) override;
    void drawShadow(Shader& shader);

    //월드 (x,z)에서의 지형 높이 — 추후 충돌/물체 배치에 사용
    float getHeightAt(float worldX, float worldZ) const;
};

#endif
#pragma once
