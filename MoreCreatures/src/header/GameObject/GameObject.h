#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H


#include <header/camera.h>
#include <Config.h>
#include <Component/Mesh.h>
#include <glad/glad.h>


class GameObject {
protected:
    glm::vec3 scale; //물리 scale
    glm::vec3 position;
    glm::vec3 velocity;

    float movement_speed;

    bool isStatic;
    bool isActive = true;

    Mesh* mesh = nullptr; 


    void move(glm::vec3 velocity, float deltaTime)
    {
        position += velocity * deltaTime;
    }


public:
    GameObject()
    {
        scale = glm::vec3(1.0f, 1.0f, 1.0f);
        position = glm::vec3(0.0f, 0.0f, 0.0f);
        velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        movement_speed = 0.0f;
        isActive = true;

        isStatic = true;
    }
    virtual ~GameObject() = default;

    void setMesh(Mesh* m) { mesh = m; }
    Mesh* getMesh() { return mesh; }


    //기본 구현: mesh가 있으면 그걸로 그림. 서브클래스가 override 가능
    virtual void drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::mat4 lightSpaceMatrix)
    {
        if (!isActive || mesh == nullptr) return;
        mesh->updateUniforms(camera, lightColor, lightPos, glm::vec3(0.0f), position, scale);
        mesh->Bind();
        mesh->Draw();
    }


    void playerMove(glm::vec3 vec, float deltaTime)
    {
        vec = glm::normalize(vec);
        float velocity = movement_speed * deltaTime;

        move(vec, velocity);
    }

    glm::vec3 getPosition()
    {
        return position;
    }


    bool isCollisionEnter(GameObject* object)
    {
        if (!(this->getIsActive()) || !(object->getIsActive()))
        {
            return false;
        }


        // ==이어도 0 ~ 10, 0 ~ 10, 0 ~ 10 즉, 3개가 같아야함 ==> 하나라도 다르면 물체가 만날 수 없음
        //x비교
        if (!isInBoundary(this->position.x, object->position.x, this->scale.x, object->scale.x))
        {
            return false;
        }

        if (!isInBoundary(this->position.y, object->position.y, this->scale.y, object->scale.y))
        {
            return false;
        }

        if (!isInBoundary(this->position.z, object->position.z, this->scale.z, object->scale.z))
        {
            return false;
        }
        //std::cout << object->position.y;
        return true;
    }

    //반발력 추가
    void addRepulsion(float deltaTime)
    {
        if (isStatic)
        {
            return;
        }
        //velocity *= COR; //반발계수
        if (velocity.y < 0)
        {
            velocity *= 0;
        }
    }

    void applyPhysics(float deltaTime)
    {
        //고정된 물체라면
        if (isStatic || !isActive)
        {
            return;
        }
        //a만큼 속력을 추가

        move(velocity, deltaTime);
        velocity += glm::vec3(0, GRAVITY_ACCELERATION, 0) * deltaTime;
    }

    glm::vec3 getVelocity()
    {
        return velocity;
    }

    void setVelocity(glm::vec3 velocity)
    {
        this->velocity = velocity;
    }

    bool getIsStatic()
    {
        return isStatic;
    }

    void setIsActive(bool isActive)
    {
        this->isActive = isActive;
    }
    bool getIsActive()
    {
        return isActive;
    }

    bool isInBoundary(float a, float b, float a_size, float b_size)
    {
        float a_1 = a - a_size / 2;
        float a_2 = a + a_size / 2;
        float b_1 = b - b_size / 2;
        float b_2 = b + b_size / 2;

        if
            (
                (a_1 <= b_1 && b_1 <= a_2) ||
                (a_1 <= b_2 && b_2 <= a_2)
                )
        {
            return true;
        }
        else if
            (
                (b_1 <= a_1 && a_1 <= b_2) ||
                (b_1 <= a_2 && a_2 <= b_2)
                )
        {
            return true;
        }
        return false;
    }

};

#endif
#pragma once
