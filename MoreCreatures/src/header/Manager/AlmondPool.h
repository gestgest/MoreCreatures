#ifndef ALMOND_POOL_H
#define ALMOND_POOL_H

#include <GameObject/Almond.h>
#include <header/shader.h>

#include <vector>
#include <glm/glm.hpp>

class ChunkManager;   //getHeightAt만 쓰니까 forward decl로 충분

// 아몬드 오브젝트 풀.
//
// 무한 월드에서 아몬드를 매번 new/delete하면:
//   - 32k vertex VBO를 GPU에 매번 올렸다 내림 → spike + 메모리 단편화
//   - 청크 unload 시점이 어긋나면 dangling pointer 위험
// 그래서 시작 시 POOL_SIZE만큼 한 번에 만들고, 이후엔 isActive 플래그만 토글해서 재사용.
//
// 알고리즘 (ChunkManager와 동일한 sliding window 패턴):
//   - viewRadius 안 청크 각각에 ALMONDS_PER_CHUNK개 스폰
//   - 청크 (cx, cz) + masterSeed로 mt19937 시드 → deterministic 위치
//     → 같은 청크는 항상 같은 자리에 아몬드. 떠났다 돌아와도 동일.
//   - 청크 떠나면 슬롯들 isActive=false로 회수, 다른 청크에서 재사용
//   - 먹은 아몬드도 isActive=false → 떠났다 돌아오면 자동 부활 (그 청크에 다시 spawn)
//
// 풀 크기 = (2*viewRadius+1)^2 * ALMONDS_PER_CHUNK = 9 * 5 = 45
//   - 모든 풀이 다 active일 수 있는 최댓값.
//   - 풀이 모자라면 새 청크 spawn 실패 (assert로 잡음 — 이론상 안 일어남)

class AlmondPool
{
public:
    static constexpr int   ALMONDS_PER_CHUNK = 5;
    static constexpr int   VIEW_RADIUS       = 1;          //3x3 = 9 청크 (ChunkManager와 일치)
    static constexpr int   POOL_SIZE         = (2 * VIEW_RADIUS + 1) * (2 * VIEW_RADIUS + 1) * ALMONDS_PER_CHUNK;
    static constexpr float CHUNK_SIZE        = 64.0f;      //ChunkManager::chunkSize와 일치
    static constexpr float SPAWN_INSET       = 5.0f;       //청크 가장자리에 너무 붙지 않게
    static constexpr unsigned int MASTER_SEED = 0xA1B05EEDu; //위치 결정용 마스터 시드 (아무 값이나 OK)

    AlmondPool(Shader& shader, unsigned int shadowMap, ChunkManager& chunkManager);
    ~AlmondPool();

    //매 프레임 호출. 플레이어가 청크 경계 넘으면 그때만 실제 작업.
    //objects는 처음 한 번 풀 전체를 등록할 때만 사용 (이후엔 isActive 토글로 처리).
    void update(const glm::vec3& playerPos, std::vector<class GameObject*>& objects);

    //RestartGame용 — 현재 활성 슬롯들의 isActive를 다시 true로
    //(먹어서 false 된 것들 부활. 비활성 슬롯은 건드리지 않음 — 어차피 안 보임)
    int reviveEatenAlmonds();

    //UpdatePickup이 순회할 수 있게 모든 풀 인스턴스 노출.
    //(이미 inActive 체크하니까 비활성 포함해도 OK)
    const std::vector<Almond*>& getAllAlmonds() const { return allAlmonds; }

private:
    Shader*       shader = nullptr;
    unsigned int  shadowMap = 0;
    ChunkManager* chunkManager = nullptr;

    //풀 슬롯 — Almond* + 현재 어느 청크에 속해 있는지
    struct Slot
    {
        Almond*   almond = nullptr;
        glm::ivec2 chunkIdx = glm::ivec2(0);
        bool       inUse = false;       //어떤 청크가 점유 중인지 여부 (isActive와 다름!)
        //  inUse=true, isActive=true  → 정상 표시
        //  inUse=true, isActive=false → 점유 중이지만 먹힘 (청크 떠나야 풀로 회수됨)
        //  inUse=false               → free, 새 청크가 가져갈 수 있음
    };
    std::vector<Slot>   slots;
    std::vector<Almond*> allAlmonds;    //getAllAlmonds용 캐시 — UpdatePickup이 매 프레임 부름

    //어느 청크가 어느 슬롯들을 점유 중인지 (역인덱스).
    //9개 정도라 vector + 선형탐색이면 충분 (ChunkManager와 같은 결정).
    struct ChunkOccupancy
    {
        glm::ivec2 idx;
        int        slotIndices[ALMONDS_PER_CHUNK];
    };
    std::vector<ChunkOccupancy> occupied;

    bool       initialized = false;
    glm::ivec2 currentCenter = glm::ivec2(0);


    glm::ivec2 worldToChunk(float worldX, float worldZ) const;

    //free 슬롯 인덱스 하나 찾기. 없으면 -1 (이론상 안 일어남)
    int allocSlot();

    //특정 청크가 점유한 슬롯 occupied[]에서 찾기. -1이면 없음.
    int findOccupied(glm::ivec2 idx) const;

    //새 청크에 5개 아몬드 spawn — deterministic 시드로 위치 결정
    void spawnChunk(glm::ivec2 chunkIdx);

    //청크 떠남 — 그 청크 슬롯들 회수 (isActive=false, inUse=false)
    void recycleChunk(glm::ivec2 chunkIdx);
};

#endif
#pragma once
