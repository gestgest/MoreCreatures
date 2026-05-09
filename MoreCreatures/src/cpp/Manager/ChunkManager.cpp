#include <Manager/ChunkManager.h>
#include <GameObject/GameObject.h>

#include <algorithm>
#include <cmath>
#include <iostream>


ChunkManager::ChunkManager(Shader& sh, glm::vec3 col)
    : shader(&sh), color(col)
{
    //중요: 청크 생성을 여기서 안 함.
    //첫 update() 호출 시 플레이어 위치 기준으로 생성.
    //(이렇게 해야 플레이어가 (0,0)이 아닌 데서 시작해도 올바른 청크가 생김)
}


ChunkManager::~ChunkManager()
{
    //주의: 청크들은 main.cpp의 objects 벡터에서 delete됨 — 여기선 안 함.
    //double-free 방지.
}


void ChunkManager::setTexture(unsigned int& tex)
{
    textureId = tex;
    for (Terrain* c : chunks) c->setTexture(textureId);   //이미 만들어진 청크에도 즉시 적용
}

void ChunkManager::setNormalMap(unsigned int& nm)
{
    normalMapId = nm;
    for (Terrain* c : chunks) c->setNormalMap(normalMapId);
}

void ChunkManager::setShadowMap(unsigned int& sm)
{
    shadowMapId = sm;
    for (Terrain* c : chunks) c->setShadowMap(shadowMapId);
}


glm::ivec2 ChunkManager::worldToChunk(float worldX, float worldZ) const
{
    //chunk i가 차지하는 영역: x ∈ [i*chunkSize - chunkSize/2, i*chunkSize + chunkSize/2]
    //→ 월드 x → chunk i = floor((x + chunkSize/2) / chunkSize)
    return glm::ivec2(
        (int)std::floor((worldX + chunkSize * 0.5f) / chunkSize),
        (int)std::floor((worldZ + chunkSize * 0.5f) / chunkSize)
    );
}


int ChunkManager::findChunk(glm::ivec2 idx) const
{
    for (int i = 0; i < (int)chunkIndices.size(); ++i)
    {
        if (chunkIndices[i] == idx) return i;
    }
    return -1;
}


// 메인 함수
void ChunkManager::update(const glm::vec3& playerPos, std::vector<GameObject*>& objects)
{
    glm::ivec2 newCenter = worldToChunk(playerPos.x, playerPos.z);

    //이미 같은 중심 청크면 작업 없음 — 매 프레임 호출되지만 99% 케이스에서 즉시 리턴
    if (initialized && newCenter == currentCenter) return;

    initialized = true;
    currentCenter = newCenter;

    //=== 1) 원하는 청크 인덱스 집합 만들기 ===
    //(중심 ± viewRadius 범위)
    std::vector<glm::ivec2> desired; //청크 (x,z) 3x3 => 즉 9개.
    setDesired(desired, newCenter);


    //2) 멀어진 청크 unload
    std::vector<glm::ivec2> unloadedList = unloadFarChunks(desired, objects);

    //3) 새 청크 load
    std::vector<glm::ivec2> loadedList = loadChunks(desired, objects);   //로그용

    //4) 청크 정보 출력
    printChunkInfo(newCenter, unloadedList, loadedList);
}

void ChunkManager::printChunkInfo(glm::ivec2 newCenter, std::vector<glm::ivec2>& unloadedList, std::vector<glm::ivec2> & loadedList) const
{

    //디버깅 출력
    if (!loadedList.empty() || !unloadedList.empty())
    {
        std::cout << "[청크] 플레이어 청크 (" << newCenter.x << ", " << newCenter.y << ")  활성=" << chunks.size();

        if (!loadedList.empty())
        {
            std::cout << "\n         + LOAD:  ";
            for (auto& l : loadedList) std::cout << "(" << l.x << "," << l.y << ") ";
        }
        if (!unloadedList.empty())
        {
            std::cout << "\n         - UNLOAD: ";
            for (auto& u : unloadedList) std::cout << "(" << u.x << "," << u.y << ") ";
        }
        std::cout << std::endl;
    }
}



float ChunkManager::getHeightAt(float worldX, float worldZ) const
{
    if (chunks.empty()) return 0.0f;
    return chunks[0]->getHeightAt(worldX, worldZ);
}

//desired 할당하는 함수
void ChunkManager::setDesired(std::vector<glm::ivec2> & desired, glm::ivec2 newCenter)
{
    //미리 할당 (capacity). push_back하면 capacity가 남아있어 메모리 요청 안함
    desired.reserve((2 * viewRadius + 1) * (2 * viewRadius + 1));

    //viewRadius는 1, 3 x 3 할당
    for (int dj = -viewRadius; dj <= viewRadius; ++dj)
    {
        for (int di = -viewRadius; di <= viewRadius; ++di)
        {
            desired.push_back(glm::ivec2(newCenter.x + di, newCenter.y + dj));
        }
    }
}

// desired에 없는 청크들 메모리 반납.
// 역순 순회 이유: 중간에서 erase하면 앞쪽 인덱스는 그대로 유지돼서 안전함.
std::vector<glm::ivec2> ChunkManager::unloadFarChunks(const std::vector<glm::ivec2>& desired,
    std::vector<GameObject*>& objects)
{
    std::vector<glm::ivec2> unloadedList;   //로그용
    for (int i = (int)chunks.size() - 1; i >= 0; --i)
    {
        glm::ivec2 idx = chunkIndices[i];

        //desired에 있는지 검사
        bool inDesired = false;
        for (const auto& d : desired)
        {
            if (d == idx)
            {
                inDesired = true; 
                break; 
            }
        }
        //자기 기준으로 내부 청크라면 무시
        if (inDesired) 
            continue;

        //밖을 벗어났다면

        Terrain* old = chunks[i];

        //objects 벡터에서 해당 포인터 찾아 제거
        auto it = std::find(objects.begin(), objects.end(), static_cast<GameObject*>(old));
        if (it != objects.end()) objects.erase(it);

        delete old;

        chunks.erase(chunks.begin() + i);
        chunkIndices.erase(chunkIndices.begin() + i);
        
        unloadedList.push_back(idx);
    }
    return unloadedList;
}

//청크 메모리 가져오는 함수
std::vector<glm::ivec2> ChunkManager::loadChunks(const std::vector<glm::ivec2>& desired,
    std::vector<GameObject*>& objects)
{
    std::vector<glm::ivec2> loadedList;
    for (const auto& d : desired)
    {
        if (findChunk(d) >= 0) continue;   //이미 있음

        //새 청크 만들기
        glm::vec2 worldCenter(d.x * chunkSize, d.y * chunkSize);
        Terrain* chunk = new Terrain(*shader, color, worldCenter);

        //저장된 텍스처 ID 적용
        if (textureId)   chunk->setTexture(textureId);
        if (normalMapId) chunk->setNormalMap(normalMapId);
        if (shadowMapId) chunk->setShadowMap(shadowMapId);

        chunks.push_back(chunk);
        chunkIndices.push_back(d);
        objects.push_back(chunk);
        loadedList.push_back(d);
    }
    return loadedList;
}


// === 디버그 정보 콘솔 출력 (F1 키로 호출) ===
void ChunkManager::printChunkMemory(const glm::vec3& playerPos) const
{
    glm::ivec2 playerChunk = worldToChunk(playerPos.x, playerPos.z);

    //청크당 메모리 추정 (gridSize=64, cellSize=1.0 기준):
    //  - 정점 데이터: 65×65 정점 × 11 float × 4 byte = 약 18.6 KB
    //  - 인덱스 데이터: 64×64 셀 × 6 인덱스 × 4 byte = 약 96 KB
    //  - 총 ≈ 115 KB/청크
    //(VAO/VBO/EBO 등 GL 객체 오버헤드 제외 — 어디까지나 추정치)
    constexpr int verticesPerSide = 65;            //(gridSize=64) + 1
    constexpr int floatsPerVertex = 11;            //pos3 + normal3 + tex2 + tangent3
    constexpr int gridSize = 64;
    constexpr int indicesPerCell = 6;              //삼각형 2개

    size_t vertBytes = (size_t)verticesPerSide * verticesPerSide * floatsPerVertex * sizeof(float);
    size_t idxBytes = (size_t)gridSize * gridSize * indicesPerCell * sizeof(unsigned int);
    size_t perChunk = vertBytes + idxBytes;

    std::cout << "\n========== [청크 디버그 정보] ==========" << std::endl;
    std::cout << "플레이어 위치: (" << playerPos.x << ", " << playerPos.y << ", " << playerPos.z << ")" << std::endl;
    std::cout << "플레이어 청크: (" << playerChunk.x << ", " << playerChunk.y << ")" << std::endl;
    std::cout << "활성 청크 수 : " << chunks.size() << " (viewRadius=" << viewRadius << ")" << std::endl;

    std::cout << "활성 청크 목록:" << std::endl;
    //인덱스 정렬 출력 — 보기 좋게
    std::vector<glm::ivec2> sorted = chunkIndices;
    std::sort(sorted.begin(), sorted.end(), [](const glm::ivec2& a, const glm::ivec2& b) {
        return a.y < b.y || (a.y == b.y && a.x < b.x);
        });
    for (const auto& idx : sorted)
    {
        bool isCurrent = (idx == playerChunk);
        std::cout << "  (" << idx.x << ", " << idx.y << ")" << (isCurrent ? "  ← 현재" : "") << std::endl;
    }

    std::cout << "메모리 추정: " << (perChunk / 1024) << " KB/청크 × "
        << chunks.size() << " = " << (perChunk * chunks.size() / 1024) << " KB" << std::endl;
    std::cout << "========================================\n" << std::endl;
}
