#ifndef MESH
#define MESH

#include <header/shader.h>
#include "Component.h"

class Mesh : public Component {
protected:
    unsigned int shadowMap;

    unsigned int vao;
    unsigned int vbo;
    Shader* shader;

    int nSphereVert;
    int nSphereAttr;

    glm::vec3 color;

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

    Mesh(Shader& shader, glm::vec3 color)
    {
        setShader(shader);
        this->color = color;
    }
    ~Mesh()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    unsigned int& getVAO()
    {
        return vao;
    }
    unsigned int& getVBO()
    {
        return vbo;
    }
    void setShader(Shader& shader)
    {
        this->shader = &shader;
    }
    void setShadowMap(unsigned int& shadowMap)
    {
        this->shadowMap = shadowMap;
    }

    void Bind()
    {
        glBindVertexArray(vao);
    }

    void Draw()
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

};
#endif
#pragma once