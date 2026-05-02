#ifndef MESH
#define MESH

#include <header/shader.h>
#include <header/camera.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Config.h>
#include "Component.h"

class Mesh : public Component {
protected:
    unsigned int shadowMap = 0;

    unsigned int vao = 0;
    unsigned int vbo = 0;
    Shader* shader;

    int nSphereVert = 0;
    int nSphereAttr = 0;
    int vertexCount = 0; //Draw에서 사용할 정점 개수

    glm::vec3 color;

    // position, normal, tex_coords.
    void init_sphere(float** vertices); // 3.14


public:
    Mesh(Shader& shader, glm::vec3 color);
    ~Mesh();


    void updateUniforms(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::vec3 color, glm::vec3 position,
        glm::vec3 mini_scale = glm::vec3(1, 1, 1));

    //구체용 버퍼 셋업 (서브클래스가 호출)
    void setupAsSphere();

    //텍스처 좌표 포함 (pos3 + normal3 + tex2 = 8 floats per vertex)
    void setupWithTexcoords(const float* vertices, int byteSize, int nVertices);

    //정점 색상 포함 (pos3 + normal3 + color3 = 9 floats per vertex)
    void setupWithColors(const float* vertices, int byteSize, int nVertices);

    //yaw 회전(Y축) 포함 모델 매트릭스 적용
    void updateUniformsWithYaw(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::vec3 color, glm::vec3 position,
        float yaw, glm::vec3 mini_scale = glm::vec3(1, 1, 1));

    // property

    unsigned int& getVAO();
    unsigned int& getVBO();
    Shader* getShader();
    glm::vec3 getColor();
    unsigned int getShadowMap();
    int getVertexCount();

    void setShader(Shader& shader);
    void setShadowMap(unsigned int& shadowMap);

    void Bind();
    void Draw();
};
#endif
#pragma once
