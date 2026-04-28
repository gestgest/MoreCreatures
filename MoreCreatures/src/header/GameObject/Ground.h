#ifndef GROUND
#define GROUND

#include <GameObject/GameObject.h>

class Ground : public GameObject {

    unsigned int* texture;

public:
    Ground()
    {
        initObject();
    }
    Ground(Shader& shader, glm::vec3 color) : GameObject(shader, color)
    {
        initObject();
    }
    void initObject()
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

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

        // 1. 위치(Position) 속성 (layout (location = 0))
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // 2. 법선(Normal) 속성 (layout (location = 1))
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // texCoord attribute (layout (location = 2))
        //index, 속성 갯수, 타입, 
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }

    void setTexture(unsigned int& texture)
    {
        this->texture = &texture;
    }

    void drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::mat4 lightSpaceMatrix)
    {
        shader->use();
        
        shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

        //택스쳐
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, *(this->texture)); //텍스쳐 넣기?

        //todo - 여기에 그림자 map 넣기
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadowMap);
        shader->setInt("shadowMap", 1); // "쉐이더야, 그림자는 1번에서 읽어와!"

        GameObject::drawMiniGameObject(camera, lightColor, lightPos, color, glm::vec3(0.0f, 0.5f, 0.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6); //삼각형
    }

    void drawShadow(Shader& shader)
    {
        glBindVertexArray(vao);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, scale); // 땅의 크기

        shader.setMat4("model", model);

        // 땅은 사각형이므로 정점이 6개! (쥐의 nSphereVert와 다릅니다)
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
};

#endif
#pragma once