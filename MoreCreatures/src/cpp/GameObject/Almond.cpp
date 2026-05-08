#include <GameObject/Almond.h>

#include <Loader/Loader.h>
#include <Component/Mesh.h>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>


// 아몬드 표피 색상 — MTL 파일이 없어서 OBJ vertex color가 흰색(1,1,1)으로 들어옴.
// loadModel 후 vertex 데이터의 color 슬롯(6,7,8)을 이 값으로 덮어써서
// 셰이더에서 살구색~황갈색 톤으로 보이게 한다.
static const glm::vec3 ALMOND_COLOR(0.78f, 0.55f, 0.35f);


// === amond.obj는 4.7MB, 32k vertex짜리 — 매 인스턴스마다 다시 파싱하면 멈춘 것처럼 보인다. ===
// 첫 Almond 생성 시 한 번만 로드하고, 이후엔 캐시된 vertex 배열을 재사용.
// (VBO는 여전히 인스턴스별로 따로 GPU에 올라가지만, 디스크 I/O와 파싱 비용은 1회로 줄어든다)
static std::vector<float> s_almondVerts;
static int  s_almondNVerts = 0;
static bool s_almondLoaded = false;
static bool s_almondLoadOK = false;

static bool loadAlmondGeometryOnce()
{
    if (s_almondLoaded) return s_almondLoadOK;
    s_almondLoaded = true;

    bool ok = Loader::loadModel("obj/amond.obj", s_almondVerts, s_almondNVerts, "Almond");
    if (!ok || s_almondNVerts == 0)
    {
        std::cout << "Almond: amond.obj load failed (object \"Almond\" 못 찾음)" << std::endl;
        s_almondLoadOK = false;
        return false;
    }

    //색상 오버라이드도 한 번만
    const int stride = 9;
    for (int i = 0; i < s_almondNVerts; ++i)
    {
        s_almondVerts[i * stride + 6] = ALMOND_COLOR.r;
        s_almondVerts[i * stride + 7] = ALMOND_COLOR.g;
        s_almondVerts[i * stride + 8] = ALMOND_COLOR.b;
    }

    std::cout << "Almond geometry cached: " << s_almondNVerts << " verts" << std::endl;
    s_almondLoadOK = true;
    return true;
}


Almond::Almond()
{
    init(nullptr, glm::vec3(0.0f));
}

Almond::Almond(Shader& shader, const glm::vec3& worldPos)
{
    init(&shader, worldPos);
}

Almond::~Almond()
{
    delete mesh;
}


void Almond::init(Shader* shaderPtr, const glm::vec3& worldPos)
{
    position = worldPos;
    scale = glm::vec3(1.0f);     //일단 OBJ 원본 크기 그대로. 너무 크면 줄이자.
    movement_speed = 0.0f;
    isStatic = true;             //아몬드는 가만히 있음 — 물리 적용 안 함
    isActive = true;

    if (!shaderPtr) return;

    //파일은 한 번만 읽고, 모든 Almond 인스턴스는 같은 vertex 배열을 공유
    if (!loadAlmondGeometryOnce()) return;

    //단, VBO는 인스턴스별로 따로 만든다 (현 Mesh 구조 유지)
    Mesh* m = new Mesh(*shaderPtr, ALMOND_COLOR);
    m->setupWithColors(s_almondVerts.data(),
                       (int)(s_almondVerts.size() * sizeof(float)),
                       s_almondNVerts);
    setMesh(m);

    //collider는 다음 단계([4] 상호작용)에서 붙임. 지금은 그리기만.
}


void Almond::setShadowMap(unsigned int& shadowMap)
{
    if (mesh) mesh->setShadowMap(shadowMap);
}


void Almond::drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::mat4 lightSpaceMatrix)
{
    if (!isActive || !mesh) return;

    Shader* shader = mesh->getShader();
    glm::vec3 color = mesh->getColor();

    shader->use();
    shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    //그림자맵 바인딩 (Mouse와 동일하게 텍스처 유닛 1번 사용)
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mesh->getShadowMap());
    shader->setInt("shadowMap", 1);

    mesh->Bind();
    mesh->updateUniforms(camera, lightColor, lightPos, color, position, scale);
    mesh->Draw();
}


void Almond::drawShadow(Shader& shader)
{
    if (!isActive || !mesh) return;

    glBindVertexArray(mesh->getVAO());

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, scale);
    shader.setMat4("model", model);

    glDrawArrays(GL_TRIANGLES, 0, mesh->getVertexCount());
}
