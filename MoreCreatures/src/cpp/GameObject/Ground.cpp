#include <GameObject/Ground.h>

#include <Component/BoxCollider.h>


Ground::Ground()
{
    initObject(nullptr, glm::vec3(1.0f));
}

Ground::Ground(Shader& shader, glm::vec3 color)
{
    initObject(&shader, color);
}

Ground::~Ground()
{
    delete mesh;
}

void Ground::initObject(Shader* shaderPtr, glm::vec3 color)
{
    position = glm::vec3(0.0f, -0.5f, 0.0f);
    scale = glm::vec3(128.0f, 1.0f, 128.0f);
    isStatic = true;
    isActive = true;

    //지면: position.y=-0.5, halfExtents.y=0.5 → collider AABB y범위 [-1, 0], 윗면 world y=0
    //x,z는 매우 넓게 — 지면은 사실상 무한 평면 취급
    BoxCollider* col = new BoxCollider(this, glm::vec3(256.0f, 1.0f, 256.0f));
    setCollider(col);

    //정점 y=0.5 → drawGameObject에서 position(=-0.5)에 더해져 world y=0(collider 윗면)에 그려짐
    float groundVertices[] = {
        // positions                 // normals         // textures
         scale.x / 2, 0.5f,  scale.z / 2,   0.0f, 1.0f, 0.0f,  scale.x / 2, scale.z / 2,
        -scale.x / 2, 0.5f,  scale.z / 2,   0.0f, 1.0f, 0.0f,  0.0f, scale.z / 2,
        -scale.x / 2, 0.5f, -scale.z / 2,   0.0f, 1.0f, 0.0f,  0.0f, 0.0f,

         scale.x / 2, 0.5f,  scale.z / 2,   0.0f, 1.0f, 0.0f,  scale.x / 2, scale.z / 2,
        -scale.x / 2, 0.5f, -scale.z / 2,   0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         scale.x / 2, 0.5f, -scale.z / 2,   0.0f, 1.0f, 0.0f,  scale.x / 2, 0.0f
    };

    if (shaderPtr)
    {
        Mesh* m = new Mesh(*shaderPtr, color);
        m->setupWithTexcoords(groundVertices, sizeof(groundVertices), 6);
        setMesh(m);
    }
}

void Ground::setTexture(unsigned int& texture)
{
    this->texture = &texture;
}

void Ground::setShadowMap(unsigned int& shadowMap)
{
    if (mesh) mesh->setShadowMap(shadowMap);
}

void Ground::drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::mat4 lightSpaceMatrix)
{
    if (!isActive || !mesh) return;

    Shader* shader = mesh->getShader();
    glm::vec3 color = mesh->getColor();

    shader->use();
    shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    //텍스처
    glActiveTexture(GL_TEXTURE0);
    if (texture) glBindTexture(GL_TEXTURE_2D, *texture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mesh->getShadowMap());
    shader->setInt("shadowMap", 1); // "셰이더야, 그림자는 1번에서 읽어와!"

    mesh->updateUniforms(camera, lightColor, lightPos, color, position, glm::vec3(1.0f));
    mesh->Bind();
    mesh->Draw();
}

void Ground::drawShadow(Shader& shader)
{
    if (!mesh) return;

    glBindVertexArray(mesh->getVAO());

    //정점에 scale.x/2, scale.z/2가 이미 박혀 있으므로 scale 행렬 적용 금지 (drawGameObject와 동일)
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    shader.setMat4("model", model);

    // 직접 6개 정점 (지면 사각형)
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
