#include <Component/Mesh.h>

#include <cmath>
#include <cstdlib>

#include <glad/glad.h>


//      =>
void Mesh::init_sphere(float** vertices) // 3.14
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
            //   8  => nAttr


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


Mesh::Mesh(Shader& shader, glm::vec3 color)
{
    setShader(shader);
    this->color = color;
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}


void Mesh::updateUniforms(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::vec3 color, glm::vec3 position,
    glm::vec3 mini_scale)
{
    //
    //fs   drawObject
    //엄준식
    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); //
    //glm::mat4 view = glm::lookAt(camera.Position, glm::vec3(0.0f, 0.0f, 0.0f), glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::mat4 view = camera.GetViewMatrix();


    //m v p 3
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    //debugMat(view);

    shader->setVec3("lightColor", lightColor);
    shader->setVec3("lightPos", lightPos);
    shader->setVec3("viewPos", camera.Position);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, mini_scale);
    shader->setMat4("model", model);

    // drawGameObject
    //mesh->bind()
    //mesh->draw()
}

void Mesh::setupAsSphere()
{
    float* sphereVerts = nullptr;
    init_sphere(&sphereVerts);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, nSphereVert * nSphereAttr * sizeof(float), sphereVerts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    vertexCount = nSphereVert;
    free(sphereVerts);
}

void Mesh::setupWithTexcoords(const float* vertices, int byteSize, int nVertices)
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, byteSize, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    vertexCount = nVertices;
}

void Mesh::setupWithColors(const float* vertices, int byteSize, int nVertices)
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, byteSize, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    vertexCount = nVertices;
}

void Mesh::updateUniformsWithYaw(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::vec3 color, glm::vec3 position,
    float yaw, glm::vec3 mini_scale)
{
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();

    shader->setMat4("projection", projection);
    shader->setMat4("view", view);

    shader->setVec3("lightColor", lightColor);
    shader->setVec3("lightPos", lightPos);
    shader->setVec3("viewPos", camera.Position);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, mini_scale);
    shader->setMat4("model", model);
}


unsigned int& Mesh::getVAO()
{
    return vao;
}
unsigned int& Mesh::getVBO()
{
    return vbo;
}
Shader* Mesh::getShader()
{
    return shader;
}
glm::vec3 Mesh::getColor()
{
    return color;
}
unsigned int Mesh::getShadowMap()
{
    return shadowMap;
}
int Mesh::getVertexCount()
{
    return vertexCount;
}

void Mesh::setShader(Shader& shader)
{
    this->shader = &shader;
}
void Mesh::setShadowMap(unsigned int& shadowMap)
{
    this->shadowMap = shadowMap;
}

void Mesh::Bind()
{
    glBindVertexArray(vao);
}

void Mesh::Draw()
{
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}
