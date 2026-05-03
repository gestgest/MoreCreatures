#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H


#include <header/camera.h>
#include <Config.h>
#include <Component/Mesh.h>
#include <Component/Collider.h>
#include <glad/glad.h>


class GameObject {
protected:
    glm::vec3 scale; //비주얼 scale
    glm::vec3 position;
    glm::vec3 velocity;

    float movement_speed;

    bool isStatic;
    bool isActive = true;

    Mesh* mesh = nullptr;
    Collider* collider = nullptr;


    void move(glm::vec3 velocity, float deltaTime);


public:
    GameObject();
    virtual ~GameObject();

    void setMesh(Mesh* m);
    Mesh* getMesh();

    void setCollider(Collider* c);
    Collider* getCollider();


    //기본 구현: mesh가 있으면 그걸로 그림. 서브클래스가 override 가능
    virtual void drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::mat4 lightSpaceMatrix);


    void playerMove(glm::vec3 vec, float deltaTime);

    glm::vec3 getPosition();


    //반발력 추가
    void addRepulsion(float deltaTime);

    void applyPhysics(float deltaTime);

    glm::vec3 getVelocity();

    void setVelocity(glm::vec3 velocity);

    bool getIsStatic();

    void setIsActive(bool isActive);
    bool getIsActive();
};

#endif
#pragma once
