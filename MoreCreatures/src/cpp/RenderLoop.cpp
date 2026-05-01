#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <header/camera.h>
#include <header/shader.h>

#include <GameObject/GameObject.h>
#include <GameObject/Ground.h>
#include <GameObject/Mouse.h>

#include <Config.h>

#include <vector>
#include <glm/gtc/matrix_transform.hpp>

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


// === 디버그용: 라이트 frustum(그림자 영역) 와이어프레임 박스 그리기 ===
//
// 원리:
//   lightSpaceMatrix = lightProjection * lightView
//   이 행렬은 "월드 좌표 → 라이트 NDC([-1,+1]^3)" 변환이다.
//   따라서 그 역행렬은 "라이트 NDC → 월드 좌표"를 해주므로,
//   NDC 큐브의 8개 코너를 역행렬로 변환하면 ortho 박스의 월드 위치를 얻을 수 있다.
//   이 8개 점을 12개 모서리로 이어 그리면 그림자 영역이 화면에 보인다.
//
// 사용법:
//   RenderScenePass()의 끝부분(swap 직전)에서 RenderShadowFrustumDebug() 호출.
//   카메라 시야에서 박스 안에 들어온 부분만 그림자가 생성된다.

static Shader* debugLineShader = nullptr;
static unsigned int debugLineVAO = 0;
static unsigned int debugLineVBO = 0;

void RenderShadowFrustumDebug()
{
    // 라이트 NDC 큐브의 8개 코너 (라이트 클립공간)
    glm::vec4 ndcCorners[8] = {
        {-1, -1, -1, 1}, {+1, -1, -1, 1}, {+1, +1, -1, 1}, {-1, +1, -1, 1}, // near 면
        {-1, -1, +1, 1}, {+1, -1, +1, 1}, {+1, +1, +1, 1}, {-1, +1, +1, 1}, // far  면
    };

    // lightSpaceMatrix의 역행렬로 월드 좌표 변환
    glm::mat4 invLightSpace = glm::inverse(lightSpaceMatrix);
    glm::vec3 worldCorners[8];
    for (int i = 0; i < 8; ++i)
    {
        glm::vec4 w = invLightSpace * ndcCorners[i];
        worldCorners[i] = glm::vec3(w) / w.w;
    }

    // 큐브 12개 모서리 (각 모서리는 두 점) → 24개 정점
    // near 면(0~3)과 far 면(4~7)을 각각 사각형으로 잇고, near-far를 4개 선으로 잇는다
    int edges[12][2] = {
        // near 면 4개
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        // far 면 4개
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        // near-far 연결 4개
        {0, 4}, {1, 5}, {2, 6}, {3, 7},
    };

    float lineVerts[12 * 2 * 3];
    for (int i = 0; i < 12; ++i)
    {
        glm::vec3 a = worldCorners[edges[i][0]];
        glm::vec3 b = worldCorners[edges[i][1]];
        lineVerts[i * 6 + 0] = a.x; lineVerts[i * 6 + 1] = a.y; lineVerts[i * 6 + 2] = a.z;
        lineVerts[i * 6 + 3] = b.x; lineVerts[i * 6 + 4] = b.y; lineVerts[i * 6 + 5] = b.z;
    }

    // 최초 1회: VAO/VBO/Shader 생성
    if (debugLineVAO == 0)
    {
        glGenVertexArrays(1, &debugLineVAO);
        glGenBuffers(1, &debugLineVBO);
        glBindVertexArray(debugLineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, debugLineVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(lineVerts), nullptr, GL_DYNAMIC_DRAW); // 매 프레임 업데이트
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }
    if (debugLineShader == nullptr)
    {
        debugLineShader = new Shader("src/vs/debugLine.vs", "src/fs/debugLine.fs");
    }

    // 매 프레임 정점 데이터 갱신 (lightSpaceMatrix가 바뀌면 박스도 바뀜)
    glBindBuffer(GL_ARRAY_BUFFER, debugLineVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(lineVerts), lineVerts);

    // 카메라 시점으로 그리기
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();

    debugLineShader->use();
    debugLineShader->setMat4("projection", projection);
    debugLineShader->setMat4("view", view);
    debugLineShader->setVec3("lineColor", glm::vec3(1.0f, 1.0f, 0.0f)); // 노란색

    // 깊이 테스트 끄면 다른 물체에 가려져도 박스가 항상 보임 (취향에 따라 켜둬도 됨)
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(debugLineVAO);
    glDrawArrays(GL_LINES, 0, 24);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}


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

    // 디버그: 그림자가 생성되는 영역(라이트 frustum)을 노란 와이어 박스로 표시
    RenderShadowFrustumDebug();

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(window);
    glfwPollEvents(); // 입력받은 callback 바로 실행
}
