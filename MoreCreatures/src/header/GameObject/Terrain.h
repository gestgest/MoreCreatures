#ifndef TERRAIN_H
#define TERRAIN_H

#include <GameObject/GameObject.h>
#include <vector>

// 오픈월드의 한 청크(타일)를 표현하는 지형 메시.
// chunkCenter로 각 인스턴스의 월드 좌표를 지정 — 같은 fbm 시드를 쓰므로
// 인접 청크 사이 높이가 매끄럽게 이어진다.
//
// 단일 모드 (chunkCenter = (0, 0)): 기존 단일 지형 동작과 동일
// 청크 모드 (chunkCenter = (i*chunkSize, j*chunkSize)): ChunkManager가 사용

//=== 비동기 청크 로딩용 데이터 패키지 ===
//워커 스레드에서 만들어 메인 스레드에 넘기는 "메시 재료".
//여기엔 GL 객체 절대 없음 — 순수 CPU 데이터(정점/인덱스 배열)만 담는다.
//→ 어느 스레드에서든 안전하게 만들 수 있고, GL 업로드만 메인 스레드에서.
struct ChunkMeshData
{
    std::vector<float>        vertexData;   //pos3 + normal3 + tex2 + tangent3 = 11 float/정점
    std::vector<unsigned int> indices;
};

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
    //=== 외부에서 buildMeshData 호출할 때 쓰는 default 파라미터 (ChunkManager 비동기 로딩용) ===
    //위 멤버 변수 default 값과 동일 — 한쪽 바꾸면 양쪽 다 갱신해야 함.
    static constexpr int   GRID_SIZE     = 64;
    static constexpr float CELL_SIZE     = 1.0f;
    static constexpr float HEIGHT_SCALE  = 8.0f;
    static constexpr float NOISE_SCALE   = 0.05f;
    static constexpr int   OCTAVES       = 5;
    static constexpr int   SEED          = 1337;

    Terrain();
    Terrain(Shader& shader, glm::vec3 color);
    Terrain(Shader& shader, glm::vec3 color, glm::vec2 chunkCenter);   //청크 위치 지정 버전
    //=== Step 2: 비동기 청크 로딩용 생성자 ===
    //워커 스레드에서 미리 만든 ChunkMeshData를 받아 GL 업로드만 함 (buildMeshData 재호출 안 함).
    //ChunkManager가 std::async로 워커에 의뢰 → ready되면 메인 스레드에서 이 생성자 호출.
    Terrain(Shader& shader, glm::vec3 color, glm::vec2 chunkCenter, const ChunkMeshData& md);
    ~Terrain();

    void initObject(Shader* shaderPtr, glm::vec3 color);

    void setTexture(unsigned int& texture);
    void setNormalMap(unsigned int& normalMap);
    void setShadowMap(unsigned int& shadowMap);

    void drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::mat4 lightSpaceMatrix) override;
    void drawShadow(Shader& shader);

    //월드 (x,z)에서의 지형 높이 — 추후 충돌/물체 배치에 사용
    float getHeightAt(float worldX, float worldZ) const;

    //=== Step 1: CPU 데이터만 만드는 정적 함수 (GL 절대 안 건드림) ===
    //어떤 스레드에서든 호출 가능 — Step 2에서 워커 스레드용으로 사용 예정.
    //멤버 변수에 의존하지 않도록 모든 파라미터를 인자로 받음.
    static ChunkMeshData buildMeshData(glm::vec2 chunkCenter,
                                       int   gridSize,
                                       float cellSize,
                                       float heightScale,
                                       float noiseScale,
                                       int   octaves,
                                       int   seed);
};

#endif
#pragma once
