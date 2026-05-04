#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <header/camera.h>
#include <header/shader.h>

#include <GameObject/GameObject.h>
#include <GameObject/Ground.h>
#include <GameObject/Terrain.h>
#include <GameObject/Mouse.h>

#include <Component/Collider.h>

#include <Config.h>

#include <vector>
#include <algorithm>
#include <cfloat>
#include <glm/gtc/matrix_transform.hpp>

extern Camera camera;
extern glm::vec3 lightPos;
extern glm::vec3 lightColor;

extern Mouse* player;
extern std::vector<GameObject*> objects;

extern GLFWwindow* window;
extern Ground* ground;
extern Terrain* terrain;
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


//그리는 범위를 선으로 표시하는 디버깅 함수
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

    // === 라이트 방향선 추가 그리기 ===
    // 점광원 위치(lightPos)와 그림자맵이 바라보는 곳(origin)을 빨간 선으로 잇는다.
    // 이 선의 방향이 곧 모든 광선의 진행 방향(방향광 가정).
    float dirVerts[6] = {
        lightPos.x, lightPos.y, lightPos.z,
        0.0f,       0.0f,       0.0f,           // lookAt target (현재 RenderShadowPass의 두 번째 인자)
    };
    glBindBuffer(GL_ARRAY_BUFFER, debugLineVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(dirVerts), dirVerts);
    debugLineShader->setVec3("lineColor", glm::vec3(1.0f, 0.0f, 0.0f)); // 빨간색
    glDrawArrays(GL_LINES, 0, 2);

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

            Collider* a = objects[i]->getCollider();
            Collider* b = objects[j]->getCollider();
            if (!a || !b) continue;

            //물체가 닿았는지
            if (a->overlaps(b))
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

// "Stable Shadow Map" 기법.
// 핵심 아이디어 두 가지:
//   1) 박스 크기는 항상 고정 (radius * 2). 그래야 카메라가 회전/줌해도 그림자가
//      커졌다 작아졌다 하지 않음. → 약간 손해보더라도 frustum보다 넉넉히 잡는다.
//   2) 박스 위치를 "그림자맵 텍셀 한 칸 단위"로 스냅. 그래야 카메라가 살짝 움직여도
//      같은 월드 좌표가 같은 텍셀에 떨어져, 그림자 가장자리가 일렁(swimming)이지 않음.
// 


// 결과: 그림자 자체는 월드에 박혀 있고, 단지 "어느 영역을 그릴지"만 카메라를 따라다님.

// 카메라 범위만큼만 그림자 계산 => 최적화를 위한 기술
static glm::mat4 ComputeLightSpaceMatrix(const Camera& cam, const glm::vec3& lightDir,
                                         float radius, unsigned int shadowMapSize)
{
    //light의 노멀벡터
    glm::vec3 normal_light = glm::normalize(lightDir);

    // (A) 박스 중심: 카메라 앞쪽으로 radius만큼 떨어진 지점 (그래야 시야 안에 그림자가 다 들어감)
    glm::vec3 center = cam.Position + cam.Front * radius;

    // (B) 라이트 view 행렬 만들기
    glm::mat4 lightView = glm::lookAt(center - normal_light, center, glm::vec3(0.0f, 1.0f, 0.0f));

    // (C) 텍셀 스냅: 박스 중심을 그림자맵 텍셀 격자에 맞춤
    float worldUnitsPerTexel = (2.0f * radius) / (float)shadowMapSize; //1픽셀에 몇 m인지 => 스크린 사이즈
    glm::vec4 centerLS = lightView * glm::vec4(center, 1.0f); //라이트 공간으로 전환
    centerLS.x = std::floor(centerLS.x / worldUnitsPerTexel) * worldUnitsPerTexel; //12.5면 12로 스크린 사이즈 단위로 변환
    centerLS.y = std::floor(centerLS.y / worldUnitsPerTexel) * worldUnitsPerTexel;
    glm::vec3 snappedCenter = glm::vec3(glm::inverse(lightView) * centerLS); //다시 현실 공간으로 전환

    // 스냅된 중심으로 lightView 다시 만들기
    lightView = glm::lookAt(snappedCenter - normal_light, snappedCenter, glm::vec3(0.0f, 1.0f, 0.0f));

// (D) ortho는 항상 같은 크기 (-radius ~ +radius)
//   z 범위는 그림자 캐스터가 박스 뒤쪽에 있어도 잡히도록 넉넉하게
constexpr float zRange = 100.0f;
glm::mat4 lightProjection = glm::ortho(-radius, radius, -radius, radius, -zRange, zRange);

//빛 뷰(lightView)로 전환 후 정해진 규격(직육면체)로 전환
return lightProjection * lightView;
}

void RenderShadowPass()
{
    // 라이트는 방향광으로 가정. lightPos는 "방향"의 출발점이라 보고 정규화해서 사용
    glm::vec3 lightDir = -glm::normalize(lightPos); // 라이트가 향하는 방향

    // radius: 카메라 주변 그림자 영역의 반지름. "약간 큰 사각형" 정도가 25~30
    // shadowMapSize: depthMap 텍스처 해상도 (depthProcessing에서 SCR_WIDTH로 만들었음)

    //camera, 빛 방향, 반지름, SCR_WIDTH
    lightSpaceMatrix = ComputeLightSpaceMatrix(camera, lightDir, 30.0f, SCR_WIDTH);

    depthShader->use();
    depthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT); // 뷰포트를 도화지 크기에 맞춤
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO); // 그림자 도화지(FBO) 장착!
    glClear(GL_DEPTH_BUFFER_BIT); // 도화지 초기화

    player->drawShadow(*depthShader);
    terrain->drawShadow(*depthShader);

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
