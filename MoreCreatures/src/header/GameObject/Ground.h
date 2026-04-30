#ifndef GROUND
#define GROUND

#include <GameObject/GameObject.h>

class Ground : public GameObject {

    unsigned int* texture = nullptr;

public:
    Ground()
    {
        initObject(nullptr, glm::vec3(1.0f));
    }
    Ground(Shader& shader, glm::vec3 color)
    {
        initObject(&shader, color);
    }
    ~Ground()
    {
        delete mesh;
    }

    void initObject(Shader* shaderPtr, glm::vec3 color)
    {
        position = glm::vec3(0.0f, -0.5f, 0.0f);
        scale = glm::vec3(128.0f, 1.0f, 128.0f);
        isStatic = true;
        isActive = true;

        float groundVertices[] = {
            // positions                 // normals         // textures
             scale.x / 2, -1.0f,  scale.z / 2,   0.0f, 1.0f, 0.0f,  scale.x / 2, scale.z / 2,
            -scale.x / 2, -1.0f,  scale.z / 2,   0.0f, 1.0f, 0.0f,  0.0f, scale.z / 2,
            -scale.x / 2, -1.0f, -scale.z / 2,   0.0f, 1.0f, 0.0f,  0.0f, 0.0f,

             scale.x / 2, -1.0f,  scale.z / 2,   0.0f, 1.0f, 0.0f,  scale.x / 2, scale.z / 2,
            -scale.x / 2, -1.0f, -scale.z / 2,   0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
             scale.x / 2, -1.0f, -scale.z / 2,   0.0f, 1.0f, 0.0f,  scale.x / 2, 0.0f
        };

        if (shaderPtr)
        {
            Mesh* m = new Mesh(*shaderPtr, color);
            m->setupWithTexcoords(groundVertices, sizeof(groundVertices), 6);
            setMesh(m);
        }
    }

    void setTexture(unsigned int& texture)
    {
        this->texture = &texture;
    }

    void setShadowMap(unsigned int& shadowMap)
    {
        if (mesh) mesh->setShadowMap(shadowMap);
    }

    void drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::mat4 lightSpaceMatrix) override
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

        mesh->updateUniforms(camera, lightColor, lightPos, color,
            position + glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(1.0f));
        mesh->Bind();
        mesh->Draw();
    }

    void drawShadow(Shader& shader)
    {
        if (!mesh) return;

        glBindVertexArray(mesh->getVAO());

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, scale); // 원래 크기

        shader.setMat4("model", model);

        // 직접 6개 정점 (지면 사각형)
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
};

#endif
#pragma once
