#include <GameObject/Terrain.h>

#include <Component/TerrainCollider.h>

#include <vector>
#include <cmath>


//정수 격자 → [0,1] 해시 (deterministic, seed 적용)
static float hash01(int x, int z, int seed)
{
    unsigned int n = (unsigned int)(x * 374761393) + (unsigned int)(z * 668265263) + (unsigned int)seed;
    n = (n ^ (n >> 13)) * 1274126177u;
    n = n ^ (n >> 16);
    return (n & 0x00FFFFFF) / float(0x01000000);
}

//Hermite smoothstep — 격자 사이 보간을 부드럽게
static float smoothstep01(float t)
{
    return t * t * (3.0f - 2.0f * t);
}

//Value noise: 정수 격자 4점에서 [0,1] 보간
static float valueNoise(float x, float z, int seed)
{
    int xi = (int)floorf(x);
    int zi = (int)floorf(z);
    float xf = x - xi;
    float zf = z - zi;

    //높이
    float a = hash01(xi,     zi,     seed);
    float b = hash01(xi + 1, zi,     seed);
    float c = hash01(xi,     zi + 1, seed);
    float d = hash01(xi + 1, zi + 1, seed);

    float u = smoothstep01(xf);
    float v = smoothstep01(zf);

    float ab = a + (b - a) * u;
    float cd = c + (d - c) * u;
    return ab + (cd - ab) * v;
}

//Fractal Brownian Motion: 여러 옥타브의 노이즈를 합성 → 자연스러운 언덕
static float fbm(float x, float z, int seed, int octaves)
{   
    float total = 0.0f;
    float freq = 1.0f; // 주파수 (얼마나 촘촘한 노이즈인가)
    float amp = 1.0f;  // 진폭 (얼마나 영향력이 큰가)
    float maxAmp = 0.0f;  // 진폭 합계 (정규화용)

    for (int i = 0; i < octaves; ++i)
    {
        //처음에는 기울기가 크게크게 만들고 나중에 amp freq값에 따라 작게작게 바뀜
        total += valueNoise(x * freq, z * freq, seed + i) * amp; //대략 노이즈를 주고
        maxAmp += amp;


        amp *= 0.5f;
        freq *= 2.0f;
    }
    return total / maxAmp; // [0,1]
}


Terrain::Terrain()
{
    initObject(nullptr, glm::vec3(1.0f));
}

Terrain::Terrain(Shader& shader, glm::vec3 color)
{
    initObject(&shader, color);
}

//청크 위치 지정 — 오픈월드 ChunkManager가 사용
Terrain::Terrain(Shader& shader, glm::vec3 color, glm::vec2 center)
{
    chunkCenter = center;
    initObject(&shader, color);
}

//=== Step 2: 비동기 청크 로딩용 생성자 ===
//워커 스레드에서 만든 ChunkMeshData를 받아 GL 업로드만 함.
//기존 initObject와 같은 일을 하지만 buildMeshData를 호출하지 않음 (이미 워커가 만든 거 사용).
Terrain::Terrain(Shader& shader, glm::vec3 color, glm::vec2 center, const ChunkMeshData& md)
{
    chunkCenter = center;

    //위치/상태 — initObject 앞부분과 동일
    position = glm::vec3(chunkCenter.x, 0.0f, chunkCenter.y);
    scale = glm::vec3(1.0f);
    isStatic = true;
    isActive = true;

    //heightfield 충돌체 — initObject와 동일 (콜라이더는 Terrain 객체 자체에 묶이므로 메인에서 생성)
    float worldExtent = gridSize * cellSize;
    float half = worldExtent * 0.5f;
    glm::vec3 localMin(-half, -1e6f, -half);
    glm::vec3 localMax( half, heightScale, half);
    TerrainCollider* col = new TerrainCollider(this, this, localMin, localMax, /*samplesPerSide=*/3);
    setCollider(col);

    //GL 업로드 — 받은 md로 바로 (워커가 미리 만들어둔 정점/인덱스 데이터)
    Mesh* m = new Mesh(shader, color);
    m->setupIndexedTexcoords(
        md.vertexData.data(), (int)(md.vertexData.size() * sizeof(float)),
        md.indices.data(), (int)md.indices.size()
    );
    setMesh(m);
}

Terrain::~Terrain()
{
    delete mesh;
}

float Terrain::getHeightAt(float worldX, float worldZ) const
{
    return fbm(worldX * noiseScale, worldZ * noiseScale, seed, octaves) * heightScale;
}

//Terrain 인스턴스 의존 없는 높이 계산 — 노이즈 파라미터는 static const 값 사용.
//(인스턴스 메서드 getHeightAt은 멤버 변수의 default 값을 쓰는데 그 default가 NOISE_SCALE 등과 동일 →
// 두 함수 결과는 항상 같음. 비동기 로딩 중 chunks가 비어도 ChunkManager가 이 함수로 안전하게 답할 수 있음.)
float Terrain::heightAt(float worldX, float worldZ)
{
    return fbm(worldX * NOISE_SCALE, worldZ * NOISE_SCALE, SEED, OCTAVES) * HEIGHT_SCALE;
}


void Terrain::initObject(Shader* shaderPtr, glm::vec3 color)
{
    //청크의 월드 위치 — 모델 매트릭스가 메시를 여기로 이동시킨다.
    //단일 모드(chunkCenter=(0,0))면 원점에 위치, 청크 모드면 청크 중심에 위치.
    position = glm::vec3(chunkCenter.x, 0.0f, chunkCenter.y);
    scale = glm::vec3(1.0f);
    isStatic = true;
    isActive = true;

    //heightfield 충돌체: 노이즈로 만든 실제 지형 표면을 따라 충돌 판정.
    //지형 메시는 x,z 모두 [-half, +half] 범위, y는 [0, heightScale] 범위에 존재.
    float worldExtent = gridSize * cellSize; // 전체 가로/세로 크기
    float half = worldExtent * 0.5f;

    //지형 코스 AABB — Y 하한은 박스가 깊이 박혀도 reject 안 되도록 충분히 낮게
    glm::vec3 localMin(-half, -1e6f, -half);
    glm::vec3 localMax( half, heightScale, half);

    TerrainCollider* col = new TerrainCollider(this, this, localMin, localMax, /*samplesPerSide=*/3);
    setCollider(col);

    //=== CPU 데이터 빌드 (Step 1에서 정적 함수로 분리 — 추후 워커 스레드로 옮길 부분) ===
    //이 호출 안에서 정점 위치(노이즈), 노멀, 탄젠트, vertexData/indices 패킹까지 모두 끝남.
    ChunkMeshData md = buildMeshData(chunkCenter, gridSize, cellSize,
                                     heightScale, noiseScale, octaves, seed);

    //=== GL 업로드 (메인 스레드 only) ===
    //Mesh 생성과 setupIndexedTexcoords는 VAO/VBO/EBO 만들고 glBufferData 호출 — GL 컨텍스트 필수.
    //그래서 비동기로 옮기면 안 됨. 워커가 만든 md만 받아서 여기서 업로드.
    if (shaderPtr)
    {
        Mesh* m = new Mesh(*shaderPtr, color);
        m->setupIndexedTexcoords(
            md.vertexData.data(), (int)(md.vertexData.size() * sizeof(float)),
            md.indices.data(), (int)md.indices.size()
        );
        setMesh(m);
    }
}


// === Step 1: CPU 데이터만 만드는 정적 함수 ===
// 어떤 스레드에서든 호출 가능 — GL 호출 절대 없음.
// fbm/valueNoise/hash01은 순수 함수(전역 상태 없음)라 thread-safe.
// Step 2에서 std::async로 이 함수를 워커 스레드에 넘길 예정.
ChunkMeshData Terrain::buildMeshData(glm::vec2 chunkCenter,
                                     int   gridSize,
                                     float cellSize,
                                     float heightScale,
                                     float noiseScale,
                                     int   octaves,
                                     int   seed)
{
    const int n = gridSize; //128 => 격자라고 한다면
    const int vertsPerSide = n + 1; // => 바둑알마냥 격자 꼭지점 4개 의미
    const int totalVerts = vertsPerSide * vertsPerSide;

    //1) 정점 위치를 노이즈로 생성
    //
    //   메시 좌표는 청크 로컬 (lx, lz) — 모델 매트릭스가 chunkCenter로 이동시킴.
    //   하지만 노이즈 샘플링은 월드 좌표 (lx + chunkCenter.x, lz + chunkCenter.y)로 해야
    //   인접 청크 경계에서 높이가 매끄럽게 이어진다 (같은 fbm 시드 + 같은 월드 좌표 → 같은 높이).
    std::vector<glm::vec3> positions(totalVerts);
    for (int z = 0; z <= n; ++z)
    {
        for (int x = 0; x <= n; ++x)
        {
            //로컬 좌표 — 청크 중심 기준 [-half, +half]
            float lx = (x - n * 0.5f) * cellSize;
            float lz = (z - n * 0.5f) * cellSize;

            //노이즈는 월드 좌표로 샘플 — 청크 경계 매끄럽게 이어짐
            float worldX = lx + chunkCenter.x;
            float worldZ = lz + chunkCenter.y;
            float h = fbm(worldX * noiseScale, worldZ * noiseScale, seed, octaves) * heightScale;

            positions[z * vertsPerSide + x] = glm::vec3(lx, h, lz);
        }
    }

    //2) 정점 노멀 계산 (인접한 두 삼각형의 면 노멀을 누적 → 정규화)
    std::vector<glm::vec3> normals(totalVerts, glm::vec3(0.0f));
    for (int z = 0; z < n; ++z)
    {
        for (int x = 0; x < n; ++x)
        {
            int i00 = z       * vertsPerSide + x;
            int i10 = z       * vertsPerSide + (x + 1);
            int i01 = (z + 1) * vertsPerSide + x;
            int i11 = (z + 1) * vertsPerSide + (x + 1);

            //삼각형 1: i00, i10, i01
            //cross 순서 주의: edge1=+X, edge2=+Z일 때 edge1×edge2 = X×Z = -Y (아래쪽).
            glm::vec3 n1 = glm::normalize(glm::cross(
                positions[i01] - positions[i00],
                positions[i10] - positions[i00]));
            glm::vec3 n2 = glm::normalize(glm::cross(
                positions[i01] - positions[i10],
                positions[i11] - positions[i10]));

            normals[i00] += n1;
            normals[i10] += n1 + n2;
            normals[i01] += n1 + n2;
            normals[i11] += n2;
        }
    }
    for (auto& n : normals) n = glm::normalize(n);

    //2-1) 정점 tangent 계산 — UV가 월드 (x,z)에 정렬되어 있으므로
    //     U 방향 월드 벡터(1,0,0)을 표면 평면에 그람-슈미트 사영해서 tangent를 얻는다.
    //     B(bitangent)는 셰이더에서 cross(N, T)로 재구성.
    std::vector<glm::vec3> tangents(totalVerts);
    for (int i = 0; i < totalVerts; ++i)
    {
        const glm::vec3& N = normals[i];
        glm::vec3 T = glm::vec3(1.0f, 0.0f, 0.0f);
        T = T - glm::dot(T, N) * N;
        float len2 = glm::dot(T, T);
        tangents[i] = (len2 > 1e-8f) ? T * (1.0f / sqrtf(len2)) : glm::vec3(1.0f, 0.0f, 0.0f);
    }

    //3) 정점 데이터 패킹: pos3 + normal3 + tex2 + tangent3 = 11 floats
    ChunkMeshData out;
    out.vertexData.reserve(totalVerts * 11);
    for (int z = 0; z <= n; ++z)
    {
        for (int x = 0; x <= n; ++x)
        {
            int i = z * vertsPerSide + x;
            const glm::vec3& p = positions[i];
            const glm::vec3& nrm = normals[i];
            const glm::vec3& t = tangents[i];

            out.vertexData.push_back(p.x);
            out.vertexData.push_back(p.y);
            out.vertexData.push_back(p.z);
            out.vertexData.push_back(nrm.x);
            out.vertexData.push_back(nrm.y);
            out.vertexData.push_back(nrm.z);
            //텍스처 타일링: 4셀당 1 UV — 너무 잦은 반복으로 인한 자글거림(aliasing) 방지
            const float uvScale = 0.25f;
            out.vertexData.push_back((float)x * uvScale);
            out.vertexData.push_back((float)z * uvScale);
            //tangent (월드 공간)
            out.vertexData.push_back(t.x);
            out.vertexData.push_back(t.y);
            out.vertexData.push_back(t.z);
        }
    }

    //4) 인덱스 버퍼: 셀 하나당 삼각형 2개 = 인덱스 6개
    out.indices.reserve(n * n * 6);
    for (int z = 0; z < n; ++z)
    {
        for (int x = 0; x < n; ++x)
        {
            unsigned int i00 = z       * vertsPerSide + x;
            unsigned int i10 = z       * vertsPerSide + (x + 1);
            unsigned int i01 = (z + 1) * vertsPerSide + x;
            unsigned int i11 = (z + 1) * vertsPerSide + (x + 1);

            out.indices.push_back(i00);
            out.indices.push_back(i10);
            out.indices.push_back(i01);

            out.indices.push_back(i10);
            out.indices.push_back(i11);
            out.indices.push_back(i01);
        }
    }

    return out;
}

void Terrain::setTexture(unsigned int& texture)
{
    this->texture = &texture;
}

void Terrain::setNormalMap(unsigned int& normalMap)
{
    this->normalMap = &normalMap;
}

void Terrain::setShadowMap(unsigned int& shadowMap)
{
    if (mesh) mesh->setShadowMap(shadowMap);
}

void Terrain::drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::mat4 lightSpaceMatrix)
{
    if (!isActive || !mesh) return;

    Shader* shader = mesh->getShader();
    glm::vec3 color = mesh->getColor();

    shader->use();
    shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    glActiveTexture(GL_TEXTURE0);
    if (texture) glBindTexture(GL_TEXTURE_2D, *texture);
    shader->setInt("texture1", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mesh->getShadowMap());
    shader->setInt("shadowMap", 1);

    glActiveTexture(GL_TEXTURE2);
    if (normalMap) glBindTexture(GL_TEXTURE_2D, *normalMap);
    shader->setInt("normalMap", 2);

    mesh->updateUniforms(camera, lightColor, lightPos, color, position, glm::vec3(1.0f));
    mesh->Bind();
    mesh->Draw();
}

void Terrain::drawShadow(Shader& shader)
{
    if (!mesh) return;

    glBindVertexArray(mesh->getVAO());

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    shader.setMat4("model", model);

    glDrawElements(GL_TRIANGLES, mesh->getIndexCount(), GL_UNSIGNED_INT, 0);
}
