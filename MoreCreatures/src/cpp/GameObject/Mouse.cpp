#include <GameObject/Mouse.h>

#include <Loader/Loader.h>
#include <vector>
#include <iostream>
#include <cmath>

#include <glad/glad.h>


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
        //쥐 obj 가져와라
        Mesh* m = new Mesh(*shaderPtr, color);

        std::vector<float> verts;
        int nVerts = 0;
        if (Loader::loadModel("obj/rat.obj", verts, nVerts))
        {
            m->setupWithColors(verts.data(), (int)(verts.size() * sizeof(float)), nVerts);
        }
        else
        {
            std::cout << "Mouse: rat.obj load failed, falling back to sphere" << std::endl;
            m->setupAsSphere();
        }
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
    shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mesh->getShadowMap());
    shader->setInt("shadowMap", 1); // "셰이더야, 그림자는 1번에서 읽어와!"

    // front 방향(XZ평면)으로 yaw 회전
    float yaw = atan2f(front.x, front.z);

    mesh->Bind();
    mesh->updateUniformsWithYaw(camera, lightColor, lightPos, color, position, yaw, scale);
    mesh->Draw();
}

void Mouse::drawShadow(Shader& shader)
{
    if (!mesh) return;

    glBindVertexArray(mesh->getVAO());
    int nVert = mesh->getVertexCount();

    float yaw = atan2f(front.x, front.z);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, scale);

    shader.setMat4("model", model);

    glDrawArrays(GL_TRIANGLES, 0, nVert);
}
