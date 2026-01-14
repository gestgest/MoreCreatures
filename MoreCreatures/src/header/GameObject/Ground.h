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
        scale = glm::vec3(24.0f, 1.0f, 24.0f);
        isStatic = true;
        isActive = true;


        float groundVertices[] = {
            // positions                 // normals         // textures
             scale.x, -1.0f,  scale.z,   0.0f, 1.0f, 0.0f,  scale.x / 2, scale.z / 2,
            -scale.x, -1.0f,  scale.z,   0.0f, 1.0f, 0.0f,  0.0f, scale.z / 2,
            -scale.x, -1.0f, -scale.z,   0.0f, 1.0f, 0.0f,  0.0f, 0.0f,

             scale.x, -1.0f,  scale.z,   0.0f, 1.0f, 0.0f,  scale.x / 2, scale.z / 2,
            -scale.x, -1.0f, -scale.z,   0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
             scale.x, -1.0f, -scale.z,   0.0f, 1.0f, 0.0f,  scale.x / 2, 0.0f
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

    void drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos)
    {
        shader->use();

        //택스쳐
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, *(this->texture)); //텍스쳐 넣기?

        GameObject::drawMiniGameObject(camera, lightColor, lightPos, color, glm::vec3(0.0f, 0.5f, 0.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6); //삼각형
    }
};

#endif
#pragma once