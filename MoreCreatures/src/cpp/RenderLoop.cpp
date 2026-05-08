#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <header/camera.h>
#include <header/shader.h>

#include <GameObject/GameObject.h>
#include <GameObject/Ground.h>
#include <GameObject/Terrain.h>
#include <GameObject/Mouse.h>
#include <GameObject/Almond.h>

#include <Component/Collider.h>

#include <UI/HUD.h>

#include <Config.h>

#include <vector>
#include <algorithm>
#include <cfloat>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

extern Camera camera;
extern glm::vec3 lightPos;
extern glm::vec3 lightColor;

extern Mouse* player;
extern std::vector<GameObject*> objects;
extern std::vector<Almond*> almonds;

extern GLFWwindow* window;
extern Ground* ground;
extern Terrain* terrain;
extern Shader* depthShader;
extern unsigned int depthMapFBO;
extern unsigned int depthMap;
extern glm::mat4 lightSpaceMatrix;
extern HUD* hud;

extern float deltaTime;
extern float lastFrame;

void processInput(GLFWwindow* window);


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


// === 시간 경과에 따른 식량 자동 감소 (배고픔) ===
//
// 디자인:
//   - 플레이어 행동과 무관하게 시간이 흐르면 식량이 줄어듦. "서바이벌 압박" 부여.
//   - 5초마다 1칸씩 감소. 5/5에서 시작하면 25초만에 0이 됨 (짧은 테스트 사이클).
//   - while 루프로 "한 프레임에 dt가 매우 클 때(디버거 정지 등) 여러 칸 한꺼번에 감소" 처리.
//     단순 if였으면 누적된 시간을 무시하고 1칸만 감소했을 것.
//   - 식량이 0이면 더 출력 안 함 — 콘솔 spam 방지.
//
// 향후 확장:
//   - 0 도달 시 "Game Over" 또는 HP 깎이기
//   - 이동 중엔 더 빨리 감소 (running cost)
//   - 시간대별 다른 감소 속도 (밤엔 추워서 더 빨리)
void UpdateHunger(float dt)
{
    if (!hud) return;

    static float decayTimer    = 0.0f;     //함수 호출 사이에 값 유지 (전역과 비슷, 단 함수 내부 한정)
    static float hpDamageTimer = 0.0f;     //식량 0 도달 후 HP 깎이는 주기
    const float DECAY_INTERVAL     = 5.0f; //식량: 5초마다 -1
    const float HP_DAMAGE_INTERVAL = 5.0f; //HP : 식량 0이면 5초마다 -1 (배고파서 죽음)

    // === 1) 식량 자연 감소 ===
    decayTimer += dt;
    while (decayTimer >= DECAY_INTERVAL)
    {
        decayTimer -= DECAY_INTERVAL;

        int curFood = hud->getFood();
        if (curFood > 0)
        {
            hud->setFood(curFood - 1);
            std::cout << "[배고픔] 식량: " << hud->getFood() << "/" << hud->getMaxFood() << std::endl;
        }
    }

    // === 2) 식량 0이면 HP 깎임 (굶주림 패널티) ===
    if (hud->getFood() == 0)
    {
        hpDamageTimer += dt;
        while (hpDamageTimer >= HP_DAMAGE_INTERVAL)
        {
            hpDamageTimer -= HP_DAMAGE_INTERVAL;

            int curHp = hud->getHp();
            if (curHp > 0)
            {
                hud->setHp(curHp - 1);
                std::cout << "[굶주림] HP: " << hud->getHp() << "/" << hud->getMaxHp() << std::endl;

                if (hud->getHp() == 0)
                    std::cout << "[게임 오버] 굶어 죽었습니다..." << std::endl;
            }
            //이미 0이면 더 깎을 거 없음 — 메세지도 안 띄움 (게임 오버 한 번만 출력)
        }
    }
    else
    {
        //식량을 회복했으면 HP 데미지 타이머 리셋 — 다음에 0 도달했을 때 새로 5초 카운트
        hpDamageTimer = 0.0f;
    }
}


// === 플레이어 ↔ 아몬드 거리 체크 → 먹기 처리 ===
//
// 디자인 결정:
//   - 정밀 충돌(BoxCollider) 대신 거리 기반 체크 사용. 픽업 시스템은 정확도보다
//     "근처에 가면 자동으로" 부드럽게 판정하는 게 게임 필이 좋음.
//   - 거리 제곱(dist²)으로 비교 — sqrt() 안 부르니까 빠르고, 비교 결과는 동일.
//     (양수끼리 비교할 때 d < r ⇔ d² < r² 이 성립)
//   - isActive=false 하나로 끝 — Almond::drawGameObject/drawShadow가 이미
//     isActive 체크해서 알아서 스킵함. 메모리는 안 풀고 깃발만 내림.
//
// 향후 확장 여지:
//   - 리스폰 타이머 (먹은 자리에서 N초 후 isActive=true 복구)
//   - 사운드/파티클 효과
//   - "max food 도달 시 픽업 차단" 옵션
void UpdatePickup(float dt)
{
    if (!player || !hud) return;

    const float pickupRadius = 1.5f;                       //이 거리 안에 들어오면 픽업
    const float pickupRadiusSq = pickupRadius * pickupRadius;

    glm::vec3 playerPos = player->getPosition();

    for (Almond* a : almonds)
    {
        //이미 먹은 건 건너뛰기 — 같은 아몬드가 다시 먹히지 않게
        if (!a->getIsActive()) continue;

        glm::vec3 ap = a->getPosition();
        glm::vec3 d = playerPos - ap;
        float distSq = d.x * d.x + d.y * d.y + d.z * d.z;

        if (distSq < pickupRadiusSq)
        {
            //먹기 처리: 사라지게 + HUD +1
            //(HUD::setFood가 [0, maxFood]로 자동 클램프해서 5/5 넘어가도 안전)
            a->setIsActive(false);
            hud->setFood(hud->getFood() + 1);

            std::cout << "[먹기] 식량: " << hud->getFood() << "/" << hud->getMaxFood() << std::endl;
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
static glm::mat4 ComputeLightSpaceMatrix(const glm::vec3& focusPos, const glm::vec3& lightDir,
                                         float radius, unsigned int shadowMapSize)
{
    //light의 노멀벡터
    glm::vec3 normal_light = glm::normalize(lightDir);

    // (A) 박스 중심: focus 위치(보통 플레이어) 기준. 카메라가 어디를 보든 focus 주변 그림자가 잡힘
    glm::vec3 center = focusPos;

    // (B) 라이트 view 행렬 만들기
    glm::mat4 lightView = glm::lookAt(center - normal_light, center, glm::vec3(0.0f, 1.0f, 0.0f));

    // (C) 텍셀 스냅: 박스 중심을 그림자맵 텍셀 격자에 맞춤
    // [디버그] 스냅 비활성화 — "전체 검정" 이슈 원인 진단용. 그림자 가장자리 일렁임 부작용 가능.
    float worldUnitsPerTexel = (2.0f * radius) / (float)shadowMapSize; //1픽셀에 몇 m인지 => 스크린 사이즈
    glm::vec4 centerLS = lightView * glm::vec4(center, 1.0f); //라이트 공간으로 전환
    centerLS.x = std::floor(centerLS.x / worldUnitsPerTexel) * worldUnitsPerTexel; //12.5면 12로 스크린 사이즈 단위로 변환
    centerLS.y = std::floor(centerLS.y / worldUnitsPerTexel) * worldUnitsPerTexel;
    glm::vec3 snappedCenter = glm::vec3(glm::inverse(lightView) * centerLS); //다시 현실 공간으로 전환

    // 스냅된 중심으로 lightView 다시 만들기
    //lightView = glm::lookAt(snappedCenter - normal_light, snappedCenter, glm::vec3(0.0f, 1.0f, 0.0f));

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

    //focus(플레이어 위치), 빛 방향, 반지름
    lightSpaceMatrix = ComputeLightSpaceMatrix(player->getPosition(), lightDir, 30.0f, SHADOW_MAP_SIZE);

    depthShader->use();
    depthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE); // 뷰포트를 도화지 크기에 맞춤
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO); // 그림자 도화지(FBO) 장착!
    glClear(GL_DEPTH_BUFFER_BIT); // 도화지 초기화

    // shadow acne 방지: 깊이를 슬로프 비례로 살짝 뒤로 밀어 표면 자기 자신을 덮는 자글거림 제거
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f); // factor(슬로프 비례), units(고정 오프셋)

    //아몬드도 그림자 캐스트
    for (int i = 0; i < objects.size(); i++)
    {
        objects[i]->drawShadow(*depthShader);
    }

    glDisable(GL_POLYGON_OFFSET_FILL);

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // 찰칵! 끝났으니 다시 모니터 화면으로 복귀
}


//그리는 함수
void Rendering()
{
    // shadow pass에서 viewport가 SHADOW_MAP_SIZE로 바뀌었으니 윈도우 크기로 복원
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // render
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); //sky
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //glEnable(GL_DEPTH_TEST);를 추가하면 GL_DEPTH_BUFFER_BIT도 넣어라

    for (int i = 0; i < objects.size(); i++)
    {
        objects[i]->drawGameObject(camera, lightColor, lightPos, lightSpaceMatrix);
    }

    // 디버그: 그림자가 생성되는 영역(라이트 frustum)을 노란 와이어 박스로 표시
    //RenderShadowFrustumDebug();

    //UI 라인
    

    // HUD는 모든 3D 씬 위에 그려져야 하므로 swap 직전에 호출
    if (hud)
    {
        hud->draw();
    }

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(window);
    glfwPollEvents(); // 입력받은 callback 바로 실행
}
