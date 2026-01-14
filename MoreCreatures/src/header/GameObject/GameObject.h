#ifndef GAMEOBJECT
#define GAMEOBJECT
#include <header/shader.h>
#include <header/camera.h>
#include <glad/glad.h>

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

#define COR  -0.33f; //반발계수
#define GRAVITY_ACCELERATION -9.81f

void debugMat(glm::mat4 m)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            std::cout << m[i][j] << ' ';
        }
        std::cout << '\n';

    }

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

class GameObject {
protected:
    unsigned int vao;
    unsigned int vbo;
    Shader* shader;

    int nSphereVert;
    int nSphereAttr;

    glm::vec3 scale; //물리 scale
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 color;

    float movement_speed;

    bool isStatic;
    bool isActive = true;


    void move(glm::vec3 velocity, float deltaTime)
    {
        position += velocity * deltaTime;
    }


    // position, normal, tex_coords.
    // 매개변수 방정식으로 원 그리는 함수 => 
    void init_sphere(float** vertices) //높이는 3.14
    {
        //nAttr : 8
        // sphere: set up vertex data and configure vertex attributes
        float pi = acosf(-1.0f);	// pi = 3.14152...
        float pi2 = 2.0f * pi;
        int nu = 40, nv = 20;
        const double du = pi2 / nu;
        const double dv = pi / nv;

        //19 * 40 * 6
        nSphereVert = (nv - 1) * nu * 6;		// two triangles
        nSphereAttr = 6;
        *vertices = (float*)malloc(sizeof(float) * (nSphereVert) * (nSphereAttr));

        float u, v;
        int k = 0;

        v = 0.0f;
        u = 0.0f;
        for (v = (-0.5f) * pi + dv; v < 0.5f * pi - dv; v += dv)
        {
            for (u = 0.0f; u < pi2; u += du)
            {
                // p(u,v)
                (*vertices)[k++] = cosf(v) * cosf(u); 	(*vertices)[k++] = cosf(v) * sinf(u);	(*vertices)[k++] = sinf(v); 	// position (x,y,z)
                (*vertices)[k++] = cosf(v) * cosf(u);	(*vertices)[k++] = cosf(v) * sinf(u);	(*vertices)[k++] = sinf(v);		// normal (x,y z)
                // 이렇게 해서 8개의 속성 => nAttr


                // p(u+du,v)
                (*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du);	(*vertices)[k++] = sinf(v); 	// position
                (*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du);	(*vertices)[k++] = sinf(v);		// normal

                // p(u,v+dv)
                (*vertices)[k++] = cosf(v + dv) * cosf(u);	(*vertices)[k++] = cosf(v + dv) * sinf(u);	(*vertices)[k++] = sinf(v + dv);	// position
                (*vertices)[k++] = cosf(v + dv) * cosf(u);	(*vertices)[k++] = cosf(v + dv) * sinf(u);	(*vertices)[k++] = sinf(v + dv);	// normal

                // p(u+du,v)
                (*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du);	(*vertices)[k++] = sinf(v); 	// position
                (*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du);	(*vertices)[k++] = sinf(v);		// normal

                // p(u+du,v+dv)
                (*vertices)[k++] = cosf(v + dv) * cosf(u + du);	(*vertices)[k++] = cosf(v + dv) * sinf(u + du);	(*vertices)[k++] = sinf(v + dv); 	// position
                (*vertices)[k++] = cosf(v + dv) * cosf(u + du);	(*vertices)[k++] = cosf(v + dv) * sinf(u + du);	(*vertices)[k++] = sinf(v + dv);	// normal

                // p(u,v+dv)
                (*vertices)[k++] = cosf(v + dv) * cosf(u);	(*vertices)[k++] = cosf(v + dv) * sinf(u);	(*vertices)[k++] = sinf(v + dv);	// position
                (*vertices)[k++] = cosf(v + dv) * cosf(u);	(*vertices)[k++] = cosf(v + dv) * sinf(u);	(*vertices)[k++] = sinf(v + dv);	// normal
            }
        }
        // triangles around north pole and south pole
        for (u = 0.0f; u < pi2; u += du)
        {
            // triangles around north pole
            // p(u,pi/2-dv)
            v = 0.5f * pi - dv;
            (*vertices)[k++] = cosf(v) * cosf(u); 	(*vertices)[k++] = cosf(v) * sinf(u);	(*vertices)[k++] = sinf(v); 	// position
            (*vertices)[k++] = cosf(v) * cosf(u);	(*vertices)[k++] = cosf(v) * sinf(u);	(*vertices)[k++] = sinf(v);		// normal

            // p(u+du,pi/2-dv)
            v = 0.5f * pi - dv;
            (*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du); (*vertices)[k++] = sinf(v); 	// position
            (*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du); (*vertices)[k++] = sinf(v);		// normal

            // p(u,pi/2) = (0, 1. 0)  ~ north pole
            v = 0.5f * pi;
            (*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du);	(*vertices)[k++] = sinf(v); 	 // position
            (*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du);	(*vertices)[k++] = sinf(v);		 // normal

            // triangles around south pole
            // p(u,-pi/2) = (0, -1, 0)  ~ south pole
            v = (-0.5f) * pi;
            (*vertices)[k++] = cosf(v) * cosf(u); 	(*vertices)[k++] = cosf(v) * sinf(u);	(*vertices)[k++] = sinf(v); 		// position
            (*vertices)[k++] = cosf(v) * cosf(u);	(*vertices)[k++] = cosf(v) * sinf(u);	(*vertices)[k++] = sinf(v);			// normal

            // p(u+du,-pi/2+dv)
            v = (-0.5f) * pi + dv;
            (*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du); (*vertices)[k++] = sinf(v);	// position
            (*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du); (*vertices)[k++] = sinf(v);	// normal

            // p(u,-pi/2+dv)
            (*vertices)[k++] = cosf(v) * cosf(u);	(*vertices)[k++] = cosf(v) * sinf(u); (*vertices)[k++] = sinf(v);	// position
            (*vertices)[k++] = cosf(v) * cosf(u);	(*vertices)[k++] = cosf(v) * sinf(u); (*vertices)[k++] = sinf(v);	// normal
        }
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
    GameObject(Shader& shader, glm::vec3 color)
    {
        setShader(shader);
        this->color = color;
    }
    ~GameObject()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }
    void setShader(Shader& shader)
    {
        this->shader = &shader;
    }
    //가상 함수
    virtual void drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos) = 0;
    void drawMiniGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::vec3 color, glm::vec3 addPos,
        glm::vec3 mini_scale = glm::vec3(1, 1, 1)
    )
    {
        //fs 셰이더 속성은 drawObject위에

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); //카메라 사이즈
        //glm::mat4 view = glm::lookAt(camera.Position, glm::vec3(0.0f, 0.0f, 0.0f), glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::mat4 view = camera.GetViewMatrix();


        //m v p 벡터3
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        //debugMat(view);

        shader->setVec3("lightColor", lightColor);
        shader->setVec3("lightPos", lightPos);
        shader->setVec3("viewPos", camera.Position);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position + addPos);
        model = glm::scale(model, mini_scale);
        shader->setMat4("model", model);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6); //삼각형
    }

    void playerMove(glm::vec3 vec, float deltaTime)
    {
        vec = glm::normalize(vec);
        float velocity = movement_speed * deltaTime;

        move(vec, velocity);
    }

    unsigned int& getVAO()
    {
        return vao;
    }
    unsigned int& getVBO()
    {
        return vbo;
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

        /*
        if (object->position.y < 2.0f)
        {
            std::cout << "bullet_y : " << object->position.y << '\n';
            std::cout << "ground_y : " << this->position.y << '\n';
            std::cout << "bullet_scale : " << object->scale.y << '\n';
            std::cout << "ground_scale: " << this->scale.y << '\n';
        }
        */

        // ==이어도 0 ~ 10, 0 ~ 10, 0 ~ 10 즉, 3개가 같아야함 ==> 하나라도 다르면 커트
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
};

#endif
#pragma once
