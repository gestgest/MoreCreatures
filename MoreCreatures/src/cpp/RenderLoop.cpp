#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <header/camera.h>
#include <header/shader.h>

#include <GameObject/GameObject.h>
#include <GameObject/Ground.h>
#include <GameObject/Mouse.h>

#include <Config.h>

#include <vector>

extern Camera camera;
extern glm::vec3 lightPos;
extern glm::vec3 lightColor;

extern Mouse* player;
extern std::vector<GameObject*> objects;

extern GLFWwindow* window;
extern Ground* ground;
extern Shader* depthShader;
extern unsigned int depthMapFBO;
extern unsigned int depthMap;
extern glm::mat4 lightSpaceMatrix;

extern float deltaTime;
extern float lastFrame;

void processInput(GLFWwindow* window);


float UpdateDeltaTime()
{
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    return deltaTime;
}

void ProcessInput(float dt)
{
    processInput(window);
    camera.move(player->getPosition());
}

void UpdatePhysics(float dt)
{
    for (int i = 0; i < objects.size(); i++)
    {
        objects[i]->applyPhysics(dt);
        for (int j = i + 1; j < objects.size(); j++)
        {
            //움직이는 물체면
            if (!objects[j]->getIsActive())
            {
                continue;
            }

            //물체가 닿았는지
            if (objects[i]->isCollisionEnter(objects[j]))
            {
                objects[i]->addRepulsion(dt);
                objects[j]->addRepulsion(dt);

                //땅에 닿았는지
                if (objects[i] == player || objects[j] == player)
                    player->SetIsGround(true);
            }
        }
    }
}

void RenderShadowPass()
{
    // shadow projection 계산
    float near_plane = 1.0f, far_plane = 12.5f;
    glm::mat4 lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //start, end, 위 벡터 : end 아무거나 해도 다 알아서 기울기 계산해준다.
    lightSpaceMatrix = lightProjection * lightView;

    depthShader->use();
    depthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT); // 뷰포트를 도화지 크기에 맞춤
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO); // 그림자 도화지(FBO) 장착!
    glClear(GL_DEPTH_BUFFER_BIT); // 도화지 초기화

    player->drawShadow(*depthShader);
    ground->drawShadow(*depthShader);

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // 찰칵! 끝났으니 다시 모니터 화면으로 복귀
}

void RenderScenePass()
{
    // render
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); //sky
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //glEnable(GL_DEPTH_TEST);를 추가하면 GL_DEPTH_BUFFER_BIT도 넣어라

    for (int i = 0; i < objects.size(); i++)
    {
        objects[i]->drawGameObject(camera, lightColor, lightPos, lightSpaceMatrix);
    }

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(window);
    glfwPollEvents(); // 입력받은 callback 바로 실행
}
