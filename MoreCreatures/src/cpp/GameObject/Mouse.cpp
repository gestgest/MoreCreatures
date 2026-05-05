#include <GameObject/Mouse.h>

#include <Loader/Loader.h>
#include <Component/BoxCollider.h>
#include <vector>
#include <iostream>
#include <cmath>
#include <limits>

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
        bool loaded = Loader::loadModel("obj/rat.obj", verts, nVerts);
        if (loaded)
        {
            m->setupWithColors(verts.data(), (int)(verts.size() * sizeof(float)), nVerts);

            //로드된 메시의 실제 AABB로 콜라이더를 맞춘다.
            //(obj 원점이 발이든 중심이든 자동으로 정확히 둘러싸짐)
            //포맷: pos3 + normal3 + color3 = 9 floats / vertex
            const int stride = 9;
            float minX =  std::numeric_limits<float>::infinity();
            float minY =  std::numeric_limits<float>::infinity();
            float minZ =  std::numeric_limits<float>::infinity();
            float maxX = -std::numeric_limits<float>::infinity();
            float maxY = -std::numeric_limits<float>::infinity();
            float maxZ = -std::numeric_limits<float>::infinity();
            for (int i = 0; i < nVerts; ++i)
            {
                float x = verts[i * stride + 0];
                float y = verts[i * stride + 1];
                float z = verts[i * stride + 2];
                if (x < minX) minX = x; if (x > maxX) maxX = x;
                if (y < minY) minY = y; if (y > maxY) maxY = y;
                if (z < minZ) minZ = z; if (z > maxZ) maxZ = z;
            }

            glm::vec3 size  (maxX - minX, maxY - minY, maxZ - minZ);
            glm::vec3 center((maxX + minX) * 0.5f,
                             (maxY + minY) * 0.5f,
                             (maxZ + minZ) * 0.5f);

            //scale 적용 (현재 (1,1,1)이지만 향후 변경 대비)
            size   *= scale;
            center *= scale;

            BoxCollider* col = new BoxCollider(this, size);
            col->setCenter(center);
            setCollider(col);

            std::cout << "Mouse collider: size=("
                      << size.x << "," << size.y << "," << size.z << ") center=("
                      << center.x << "," << center.y << "," << center.z << ")" << std::endl;
        }
        else
        {
            std::cout << "Mouse: rat.obj load failed, falling back to sphere" << std::endl;
            m->setupAsSphere();

            //fallback 콜라이더: 단위 구체 정도
            BoxCollider* col = new BoxCollider(this, glm::vec3(1.0f, 1.0f, 1.0f));
            col->setCenter(glm::vec3(0.0f, 0.0f, 0.0f));
            setCollider(col);
        }
        setMesh(m);
    }
    else
    {
        //쉐이더 없는 경로(테스트용): 안전한 기본 콜라이더
        BoxCollider* col = new BoxCollider(this, glm::vec3(0.6f, 1.8f, 1.2f));
        col->setCenter(glm::vec3(0.0f, 0.9f, 0.0f));
        setCollider(col);
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
