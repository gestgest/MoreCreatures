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

Terrain::~Terrain()
{
    delete mesh;
}

float Terrain::getHeightAt(float worldX, float worldZ) const
{
    return fbm(worldX * noiseScale, worldZ * noiseScale, seed, octaves) * heightScale;
}


void Terrain::initObject(Shader* shaderPtr, glm::vec3 color)
{
    position = glm::vec3(0.0f, 0.0f, 0.0f);
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

    const int n = gridSize; //128 => 격자라고 한다면
    const int vertsPerSide = n + 1; // => 바둑알마냥 격자 꼭지점 4개 의미
    const int totalVerts = vertsPerSide * vertsPerSide;

    //1) 정점 위치를 노이즈로 생성
    std::vector<glm::vec3> positions(totalVerts);
    for (int z = 0; z <= n; ++z)
    {
        for (int x = 0; x <= n; ++x)
        {
            //바둑판처럼 왼쪽 뒤부터 좌표 설정
            float wx = (x - n * 0.5f) * cellSize;
            float wz = (z - n * 0.5f) * cellSize;

            //랜덤 값은 h값만 주면 된다.
            float h = fbm(wx * noiseScale, wz * noiseScale, seed, octaves) * heightScale;

            positions[z * vertsPerSide + x] = glm::vec3(wx, h, wz);
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
            glm::vec3 n1 = glm::normalize(glm::cross(
                positions[i10] - positions[i00],
                positions[i01] - positions[i00]));
            //삼각형 2: i10, i11, i01
            glm::vec3 n2 = glm::normalize(glm::cross(
                positions[i11] - positions[i10],
                positions[i01] - positions[i10]));

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
    std::vector<float> vertexData;
    vertexData.reserve(totalVerts * 11);
    for (int z = 0; z <= n; ++z)
    {
        for (int x = 0; x <= n; ++x)
        {
            int i = z * vertsPerSide + x;
            const glm::vec3& p = positions[i];
            const glm::vec3& nrm = normals[i];
            const glm::vec3& t = tangents[i];

            vertexData.push_back(p.x);
            vertexData.push_back(p.y);
            vertexData.push_back(p.z);
            vertexData.push_back(nrm.x);
            vertexData.push_back(nrm.y);
            vertexData.push_back(nrm.z);
            //텍스처 타일링: 4셀당 1 UV — 너무 잦은 반복으로 인한 자글거림(aliasing) 방지
            const float uvScale = 0.25f;
            vertexData.push_back((float)x * uvScale);
            vertexData.push_back((float)z * uvScale);
            //tangent (월드 공간)
            vertexData.push_back(t.x);
            vertexData.push_back(t.y);
            vertexData.push_back(t.z);
        }
    }

    //4) 인덱스 버퍼: 셀 하나당 삼각형 2개 = 인덱스 6개
    std::vector<unsigned int> indices;
    indices.reserve(n * n * 6);
    for (int z = 0; z < n; ++z)
    {
        for (int x = 0; x < n; ++x)
        {
            unsigned int i00 = z       * vertsPerSide + x;
            unsigned int i10 = z       * vertsPerSide + (x + 1);
            unsigned int i01 = (z + 1) * vertsPerSide + x;
            unsigned int i11 = (z + 1) * vertsPerSide + (x + 1);

            indices.push_back(i00);
            indices.push_back(i10);
            indices.push_back(i01);

            indices.push_back(i10);
            indices.push_back(i11);
            indices.push_back(i01);
        }
    }

    //5)
    if (shaderPtr)
    {
        Mesh* m = new Mesh(*shaderPtr, color);
        m->setupIndexedTexcoords(
            vertexData.data(), (int)(vertexData.size() * sizeof(float)),
            indices.data(), (int)indices.size()
        );
        setMesh(m);
    }
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
