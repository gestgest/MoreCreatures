#include <Manager/AlmondPool.h>
#include <Manager/ChunkManager.h>      //getHeightAt — 헤더에선 forward decl만, 여기서 풀로

#include <algorithm>
#include <cmath>
#include <random>
#include <iostream>


AlmondPool::AlmondPool(Shader& sh, unsigned int sm, ChunkManager& cm)
    : shader(&sh), shadowMap(sm), chunkManager(&cm)
{
    //=== 풀 미리 할당 ===
    //모든 Almond를 시작 시 한 번에 만든다. 핵심 의도:
    //  - VBO 45개 GL 업로드 비용을 게임 루프 진입 전에 다 치름 (렉 spike 방지)
    //  - amond.obj 파싱은 어차피 캐시(Almond.cpp의 s_almondVerts)라 한 번만 일어남
    //처음엔 모두 비활성. 첫 update()가 viewRadius 안 청크에 채워줌.
    slots.reserve(POOL_SIZE);
    allAlmonds.reserve(POOL_SIZE);

    for (int i = 0; i < POOL_SIZE; ++i)
    {
        Slot s;
        //위치는 origin으로 — 어차피 첫 spawn 시 제대로 세팅됨.
        //단, 비활성 상태라 origin에 있어도 안 그려짐 (Almond::drawGameObject가 isActive 체크).
        s.almond = new Almond(*shader, glm::vec3(0.0f));
        s.almond->setShadowMap(shadowMap);
        s.almond->setIsActive(false);
        s.inUse = false;

        slots.push_back(s);
        allAlmonds.push_back(s.almond);
    }
    occupied.reserve((2 * VIEW_RADIUS + 1) * (2 * VIEW_RADIUS + 1));

    std::cout << "AlmondPool: " << POOL_SIZE << "개 인스턴스 풀로 미리 할당 완료" << std::endl;
}


AlmondPool::~AlmondPool()
{
    //풀이 직접 만든 Almond들은 풀이 책임지고 정리.
    //(main.cpp의 objects 벡터는 Almond를 받지 않음 — 풀이 단독 소유)
    for (Slot& s : slots)
    {
        delete s.almond;
    }
}


glm::ivec2 AlmondPool::worldToChunk(float worldX, float worldZ) const
{
    //ChunkManager::worldToChunk와 동일한 공식 — 같은 청크 인덱스 체계 공유해야 함
    return glm::ivec2(
        (int)std::floor((worldX + CHUNK_SIZE * 0.5f) / CHUNK_SIZE),
        (int)std::floor((worldZ + CHUNK_SIZE * 0.5f) / CHUNK_SIZE)
    );
}


int AlmondPool::allocSlot()
{
    for (int i = 0; i < (int)slots.size(); ++i)
    {
        if (!slots[i].inUse) return i;
    }
    return -1;   //이론상 안 일어남 — POOL_SIZE가 desired 청크 × ALMONDS_PER_CHUNK라서 항상 충분
}


int AlmondPool::findOccupied(glm::ivec2 idx) const
{
    for (int i = 0; i < (int)occupied.size(); ++i)
    {
        if (occupied[i].idx == idx) return i;
    }
    return -1;
}


// === 청크에 5개 deterministic spawn ===
//
// 핵심: 청크 좌표 + masterSeed → 시드. 같은 청크는 항상 같은 위치 5곳.
// 떠났다 돌아와도 동일하니까 "맵에 박힌 음식"처럼 느껴짐.
//
// 시드 합성: cx, cz를 그냥 더하면 (1,2)와 (2,1)이 충돌. 곱셈 해시로 섞어야 함.
// 표준적인 splitmix-스타일 mixing 상수 사용 — 작은 입력 범위에서도 골고루 퍼짐.
void AlmondPool::spawnChunk(glm::ivec2 chunkIdx)
{
    if (findOccupied(chunkIdx) >= 0) return;   //이미 점유 중

    //시드 합성 — 이게 위치 deterministic의 핵심
    unsigned int seed = MASTER_SEED;
    seed ^= (unsigned int)(chunkIdx.x * 0x9E3779B9u);
    seed ^= (unsigned int)(chunkIdx.y * 0x85EBCA6Bu);
    seed ^= (seed >> 16);                        //bit-mixing — 인접 청크끼리 시드가 비슷해지지 않게

    std::mt19937 rng(seed);
    const float min = -CHUNK_SIZE * 0.5f + SPAWN_INSET;
    const float max =  CHUNK_SIZE * 0.5f - SPAWN_INSET;
    std::uniform_real_distribution<float> distXZ(min, max);

    ChunkOccupancy occ;
    occ.idx = chunkIdx;

    for (int i = 0; i < ALMONDS_PER_CHUNK; ++i)
    {
        int slotIdx = allocSlot();
        if (slotIdx < 0)
        {
            std::cout << "AlmondPool: 풀 고갈! POOL_SIZE 부족" << std::endl;
            return;
        }

        //청크 로컬 좌표 → 월드 좌표
        float localX = distXZ(rng);
        float localZ = distXZ(rng);
        float worldX = chunkIdx.x * CHUNK_SIZE + localX;
        float worldZ = chunkIdx.y * CHUNK_SIZE + localZ;
        float worldY = chunkManager->getHeightAt(worldX, worldZ);

        Slot& s = slots[slotIdx];
        s.almond->setPosition(glm::vec3(worldX, worldY, worldZ));
        s.almond->setIsActive(true);
        s.chunkIdx = chunkIdx;
        s.inUse    = true;

        occ.slotIndices[i] = slotIdx;
    }

    occupied.push_back(occ);
}


// === 청크 떠남 — 슬롯 회수 ===
//
// 점유 중이던 슬롯 5개 모두 isActive=false, inUse=false로 되돌림.
// 그 슬롯은 즉시 다른 청크가 가져갈 수 있음.
// (먹어서 이미 isActive=false였더라도 inUse=false로 풀어주는 게 핵심)
void AlmondPool::recycleChunk(glm::ivec2 chunkIdx)
{
    int oi = findOccupied(chunkIdx);
    if (oi < 0) return;

    for (int i = 0; i < ALMONDS_PER_CHUNK; ++i)
    {
        int slotIdx = occupied[oi].slotIndices[i];
        Slot& s = slots[slotIdx];
        s.almond->setIsActive(false);
        s.inUse = false;
    }
    occupied.erase(occupied.begin() + oi);
}


// === 메인 update ===
//
// ChunkManager::update와 동일한 sliding window 로직.
// 매 프레임 불러도 청크 안 바뀌면 즉시 리턴 — 비용 거의 없음.
void AlmondPool::update(const glm::vec3& playerPos, std::vector<GameObject*>& /*objects*/)
{
    glm::ivec2 newCenter = worldToChunk(playerPos.x, playerPos.z);

    //청크 안 바뀌었으면 할 일 없음
    if (initialized && newCenter == currentCenter) return;

    initialized = true;
    currentCenter = newCenter;

    //=== 1) desired 청크 집합 ===
    std::vector<glm::ivec2> desired;
    desired.reserve((2 * VIEW_RADIUS + 1) * (2 * VIEW_RADIUS + 1));
    for (int dz = -VIEW_RADIUS; dz <= VIEW_RADIUS; ++dz)
    {
        for (int dx = -VIEW_RADIUS; dx <= VIEW_RADIUS; ++dx)
        {
            desired.push_back(glm::ivec2(newCenter.x + dx, newCenter.y + dz));
        }
    }

    //=== 2) 멀어진 청크 회수 (역순 순회 — erase 안전) ===
    for (int i = (int)occupied.size() - 1; i >= 0; --i)
    {
        glm::ivec2 idx = occupied[i].idx;
        bool stillDesired = false;
        for (const auto& d : desired)
        {
            if (d == idx) { stillDesired = true; break; }
        }
        if (!stillDesired)
        {
            recycleChunk(idx);   //occupied[i] 자체가 erase됨
        }
    }

    //=== 3) 새 청크에 spawn ===
    for (const auto& d : desired)
    {
        if (findOccupied(d) >= 0) continue;
        spawnChunk(d);
    }
}


// === 먹은 아몬드 부활 (RestartGame 전용) ===
//
// 현재 inUse 중인 슬롯 중 isActive=false인 놈들만 다시 true로.
// 청크는 그대로 유지 — 위치는 안 바뀜. "그 자리에 다시 나타남" 효과.
int AlmondPool::reviveEatenAlmonds()
{
    int revived = 0;
    for (Slot& s : slots)
    {
        if (s.inUse && !s.almond->getIsActive())
        {
            s.almond->setIsActive(true);
            revived++;
        }
    }
    return revived;
}
