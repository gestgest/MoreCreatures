#ifndef MOUSE_H
#define MOUSE_H

#include <GameObject/Creature.h>

class Mouse : public Creature {
public:
    Mouse()
    {
        init();
    }
    Mouse(Shader& shader, glm::vec3 color) : Creature(shader, color)
    {
        init();
    }

    void init()
    {
        position = glm::vec3(0.0f, 10.0f, 0.0f);

        scale = glm::vec3(1.0f, 1.0f, 1.0f);

        movement_speed = 10.0f;
        isStatic = false;
        isActive = true;

        float* sphereVerts = NULL;

        init_sphere(&sphereVerts);
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, nSphereVert * nSphereAttr * sizeof(float), sphereVerts, GL_STATIC_DRAW); //static draw를 하는게 맞나?

        // 1. 위치(Position) 속성 (layout (location = 0))
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // 2. 법선(Normal) 속성 (layout (location = 1))
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        free(sphereVerts);
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


    void drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos)
    {
        float pi = acosf(-1.0f);
        shader->use();
        //fs
        // light properties
        shader->setVec3("objectColor", color);

        //원 그릴때 포지션은 밑바닥기준
        //몸통
        GameObject::drawMiniGameObject(camera, lightColor, lightPos, color, glm::vec3(0.0f, -0.5f, 0.0f));
        glDrawArrays(GL_TRIANGLES, 0, nSphereVert); 

        //머리
        GameObject::drawMiniGameObject(camera, lightColor, lightPos, color, glm::vec3(0.0f, -0.5f, 0.0f) + front * 1.5f, glm::vec3(0.75f, 0.75f, 0.75f));
        glDrawArrays(GL_TRIANGLES, 0, nSphereVert); 
    }
};
#endif
#pragma once