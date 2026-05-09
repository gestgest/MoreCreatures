#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include <GameObject/Terrain.h>
#include <header/shader.h>

#include <vector>
#include <future>      //std::async, std::future — 비동기 청크 로딩용
#include <glm/glm.hpp>

// 오픈월드 청크 관리자.
// 플레이어 위치를 추적해서 viewRadius 안 청크는 load, 밖은 unload — 무한 월드 시뮬레이션.
//
// Phase 2: update() 호출하면 동적으로 load/unload.
//   - 첫 호출 시: 자동으로 viewRadius 범위 청크 N개 생성
//   - 이후 호출: 플레이어가 청크 경계 넘어가면 그때마다 새 청크 생성, 멀어진 청크 삭제
//
// 청크 소유권:
//   - ChunkManager가 청크 객체를 생성/삭제하지만, **렌더 등록은 외부 objects 벡터에 위임**
//   - update()가 objects 벡터를 직접 수정 (push/erase)
//   - main.cpp 종료 시 objects 모두 delete → 살아있는 청크들 정리

class ChunkManager
{
public:
    ChunkManager(Shader& shader, glm::vec3 color);
    ~ChunkManager();

    // 텍스처/노멀맵/그림자맵 ID 저장 — 새로 load되는 청크에도 자동 적용
    void setTexture(unsigned int& tex);
    void setNormalMap(unsigned int& nm);
    void setShadowMap(unsigned int& sm);

    // 플레이어 위치 보고 청크 load/unload. 매 프레임 호출 권장.
    // 청크 안 바뀌면 즉시 리턴 — 비용 거의 없음.
    void update(const glm::vec3& playerPos, std::vector<class GameObject*>& objects);

    // 임의 월드 좌표의 지형 높이 — 어느 청크 인스턴스든 같은 답
    float getHeightAt(float worldX, float worldZ) const;

    float getChunkSize() const { return chunkSize; }

    // 디버그: 현재 청크 상태 콘솔에 덤프 (F1 키로 호출)
    //   - 활성 청크 갯수, 인덱스 목록
    //   - 플레이어 위치 / 플레이어 청크
    //   - 청크당 추정 메모리 + 총 메모리
    void printChunkMemory(const glm::vec3& playerPos) const;

private:
    Shader* shader = nullptr;       //새 청크 생성 시 재사용
    glm::vec3 color = glm::vec3(1.0f);

    //텍스처 ID들 — load되는 새 청크에 적용하기 위해 저장
    unsigned int textureId   = 0;
    unsigned int normalMapId = 0;
    unsigned int shadowMapId = 0;

    //청크 + 청크 인덱스 (chunk 좌표) 병행 관리.
    //9개 정도라 vector + 선형 탐색이면 충분 (map 오버헤드 불필요)
    std::vector<Terrain*>   chunks;
    std::vector<glm::ivec2> chunkIndices;

    bool       initialized = false;          //첫 update() 호출 트리거용
    glm::ivec2 currentCenter = glm::ivec2(0);

    float chunkSize = 64.0f;   //Terrain의 gridSize*cellSize와 일치
    int   viewRadius = 1;      //±N 청크. 1이면 3x3 = 9개


    //월드 좌표 → 청크 인덱스. 청크 중심 ±chunkSize/2 범위가 그 청크에 속함
    glm::ivec2 worldToChunk(float worldX, float worldZ) const;
    
    //청크 인덱스가 chunks 벡터에서 몇 번째인지 (-1이면 없음)
    int        findChunk(glm::ivec2 idx) const;
    void setDesired(std::vector<glm::ivec2>& desired, glm::ivec2 newCenter);


    void printChunkInfo(glm::ivec2 newCenter, std::vector<glm::ivec2>& unloadedList, std::vector<glm::ivec2>& loadedList) const;

    std::vector<glm::ivec2> unloadFarChunks(const std::vector<glm::ivec2>& desired,
        std::vector<class GameObject*>& objects);
    std::vector<glm::ivec2> loadChunks(const std::vector<glm::ivec2>& desired,
        std::vector<class GameObject*>& objects);

    //=== Step 2: 비동기 청크 로딩 ===
    //워커 스레드에서 buildMeshData를 돌리는 동안 메인 스레드는 멈추지 않음.
    //ready된 결과만 메인에서 GL 업로드.
    struct PendingChunk
    {
        glm::ivec2 idx;
        std::future<ChunkMeshData> future;   //워커가 만들고 있는 메시 데이터
    };
    std::vector<PendingChunk> pendingFutures;

    //한 프레임에 GL 업로드할 최대 청크 수 — spike 분산. 1이면 가장 부드러움, 늘리면 더 빨리 채워짐.
    int maxUploadsPerFrame = 1;

    //새 청크 좌표들에 대해 std::async로 워커 의뢰 — pendingFutures에 추가
    void requestLoadChunks(const std::vector<glm::ivec2>& desired);

    //pending 큐 폴링 — ready된 future들을 메인 스레드에서 GL 업로드 (프레임당 maxUploadsPerFrame 개)
    std::vector<glm::ivec2> processPendingChunks(std::vector<class GameObject*>& objects);
};

#endif
#pragma once
