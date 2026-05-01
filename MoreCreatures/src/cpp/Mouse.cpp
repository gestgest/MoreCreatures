#include <GameObject/Mouse.h>


Mouse::Mouse()
{
    init(nullptr, glm::vec3(1.0f));
}

Mouse::Mouse(Shader& shader, glm::vec3 color) : Creature()
{
    init(&shader, color);
}

Mouse::~Mouse()
{
    delete mesh;
}

void Mouse::init(Shader* shaderPtr, glm::vec3 color)
{
    position = glm::vec3(0.0f, 10.0f, 0.0f);
    scale = glm::vec3(1.0f, 1.0f, 1.0f);

    movement_speed = 10.0f;
    isStatic = false;
    isActive = true;

    if (shaderPtr)
    {
        Mesh* m = new Mesh(*shaderPtr, color);
        m->setupAsSphere();
        setMesh(m);
    }
}

void Mouse::setShadowMap(unsigned int& shadowMap)
{
    if (mesh) mesh->setShadowMap(shadowMap);
}

void Mouse::drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::mat4 lightSpaceMatrix)
{
    if (!isActive || !mesh) return;

    Shader* shader = mesh->getShader();
    glm::vec3 color = mesh->getColor();

    shader->use();
    //fs
    // light properties
    shader->setVec3("objectColor", color);
    shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mesh->getShadowMap());
    shader->setInt("shadowMap", 1); // "셰이더야, 그림자는 1번에서 읽어와!"

    mesh->Bind();

    //몸통 (바닥에서 -0.5 위쪽)
    mesh->updateUniforms(camera, lightColor, lightPos, color,
        position + glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(1.0f));
    mesh->Draw();

    //머리 (몸통 + front 방향으로 1.5만큼, 0.75배 크기)
    mesh->updateUniforms(camera, lightColor, lightPos, color,
        position + glm::vec3(0.0f, -0.5f, 0.0f) + front * 1.5f, glm::vec3(0.75f));
    mesh->Draw();
}

void Mouse::drawShadow(Shader& shader)
{
    if (!mesh) return;

    // 1. 구체 메쉬(VAO) 바인드
    glBindVertexArray(mesh->getVAO());
    int nVert = mesh->getVertexCount();

    // ==========================================
    // 2. 몸통(Body)의 위치 행렬 및 그리기
    // ==========================================
    glm::mat4 bodyModel = glm::mat4(1.0f);
    bodyModel = glm::translate(bodyModel, position + glm::vec3(0.0f, -0.5f, 0.0f));

    shader.setMat4("model", bodyModel);

    glDrawArrays(GL_TRIANGLES, 0, nVert);

    // ==========================================
    // 3. 머리(Head)의 위치 행렬 및 그리기
    // ==========================================
    glm::mat4 headModel = glm::mat4(1.0f);
    headModel = glm::translate(headModel, position + glm::vec3(0.0f, -0.5f, 0.0f) + front * 1.5f);
    headModel = glm::scale(headModel, glm::vec3(0.75f, 0.75f, 0.75f));

    shader.setMat4("model", headModel);

    glDrawArrays(GL_TRIANGLES, 0, nVert);
}
