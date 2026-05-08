#ifndef TERRAIN_H
#define TERRAIN_H

#include <GameObject/GameObject.h>

// 오픈월드의 한 청크(타일)를 표현하는 지형 메시.
// chunkCenter로 각 인스턴스의 월드 좌표를 지정 — 같은 fbm 시드를 쓰므로
// 인접 청크 사이 높이가 매끄럽게 이어진다.
//
// 단일 모드 (chunkCenter = (0, 0)): 기존 단일 지형 동작과 동일
// 청크 모드 (chunkCenter = (i*chunkSize, j*chunkSize)): ChunkManager가 사용
class Terrain : public GameObject {

    unsigned int* texture = nullptr;
    unsigned int* normalMap = nullptr;

    int gridSize = 64;        // NxN cells (정점은 (N+1)x(N+1)) — chunk당 64x64 격자
    float cellSize = 1.0f;    // 한 칸당 월드 단위
    float heightScale = 8.0f; // 최대 언덕 높이
    float noiseScale = 0.05f; // 노이즈 주파수 (작을수록 완만)
    int   octaves = 5;        // FBm 옥타브 수
    int   seed = 1337;

    glm::vec2 chunkCenter = glm::vec2(0.0f);  //이 청크의 월드 중심 (XZ 좌표)

public:
    Terrain();
    Terrain(Shader& shader, glm::vec3 color);
    Terrain(Shader& shader, glm::vec3 color, glm::vec2 chunkCenter);   //청크 위치 지정 버전
    ~Terrain();

    void initObject(Shader* shaderPtr, glm::vec3 color);

    void setTexture(unsigned int& texture);
    void setNormalMap(unsigned int& normalMap);
    void setShadowMap(unsigned int& shadowMap);

    void drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::mat4 lightSpaceMatrix) override;
    void drawShadow(Shader& shader);

    //월드 (x,z)에서의 지형 높이 — 추후 충돌/물체 배치에 사용
    float getHeightAt(float worldX, float worldZ) const;
};

#endif
#pragma once
