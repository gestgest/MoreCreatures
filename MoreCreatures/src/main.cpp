#include <glad/glad.h>
#include <GLFW/glfw3.h>

//콘솔 한글 출력 위해 UTF-8 코드페이지로 전환
#ifdef _WIN32
#include <Windows.h>
#endif

#include <header/camera.h>
#include <header/shader.h>

#include <GameObject/Ground.h>
#include <GameObject/Terrain.h>
#include <Manager/ChunkManager.h>
#include <Manager/AlmondPool.h>
#include <GameObject/Mouse.h>
#include <GameObject/Almond.h>

#include <UI/HUD.h>

#include <Input/InputManager.h>

#include <Loader/Loader.h>
#include <Debug/DebugTimer.h>   //초기화 단계별 시간 측정 (평소 false, 진단 시 true)

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>   //std::find — 종료 시 objects에서 풀 Almond들 빼낼 때

#define STB_IMAGE_IMPLEMENTATION
//std_image.h를 이용해서 이미지 열려면 위에 이거 정의해야함
#include <header/std_image.h>

// 입력 처리는 모두 InputManager로 분리됨 — 콜백 forward decl 불필요

float UpdateDeltaTime();
void UpdateFpsCounter(GLFWwindow* window, float deltaTime);   //디버그: FPS + spike 측정 (창 제목/콘솔)
void UpdatePhysics(float dt);
void UpdateHunger(float dt);
void UpdatePickup(float dt);
void RenderShadowPass();
void Rendering();
void ResetHunger();      //굶주림 타이머 리셋 (RenderLoop.cpp에 정의)

float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera(glm::vec3(10.0f, 10.0f, 10.0f));
glm::vec3 lightPos(5.0f, 3.0f, 5.0f);
glm::vec3 lightColor(1.0, 1.0, 1.0);

glm::vec3 dir[6] = { glm::vec3(-1.0f,0.0f,1.0f), glm::vec3(1.0f,0,-1.0f), glm::vec3(0,1.0f,0.0f), glm::vec3(0,-1.0f,0.0f), glm::vec3(-1.0f,0.0f,-1.0f), glm::vec3(1.0f,0,1.0f) };

Mouse* player;
std::vector<GameObject*> objects;
//almonds 전역은 AlmondPool이 소유. UpdatePickup/RestartGame에서 pool->getAllAlmonds()로 접근.
//(레거시 std::vector<Almond*> almonds는 제거됨 — pool 단일 소유로 일원화)
AlmondPool* almondPool = nullptr;


GLFWwindow* window = nullptr;
Ground* ground = nullptr;
Terrain* terrain = nullptr;        //(레거시 — 사용 중단, ChunkManager가 대신함)
ChunkManager* chunkManager = nullptr;
Shader* depthShader = nullptr;
unsigned int depthMapFBO = 0;
unsigned int depthMap = 0;
glm::mat4 lightSpaceMatrix;
HUD* hud = nullptr;

//(이전엔 lastX/lastY/firstMouse 마우스 상태 전역이 있었지만 InputManager 안으로 캡슐화됨)


void depthProcessing(unsigned int& depthMapFBO, unsigned int& depthMap);


//todo
//vcpkg 설치
//Assimp 설치


int main()
{
    //Windows 콘솔 코드페이지를 UTF-8로 변경 — 안 하면 한글이 깨져서 출력됨
    //(소스는 /utf-8 플래그로 UTF-8 저장, 콘솔 기본은 CP949 → mismatch)
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    //=== 초기화 단계별 시간 측정 (디버그) ===
    //평소엔 false → report() 출력 안 됨 (총 시간만 마지막에 출력).
    //나중에 흰 화면 / 로딩 spike 등 의심되면 true로 한 글자만 바꿔서 빌드 → 단계별 시간 다 보임.
    DebugTimer initTimer(true);

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "MoreCreatures", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    //모든 GLFW 콜백 등록 — InputManager 한 줄로 묶어서 처리
    InputManager::registerCallbacks(window);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    initTimer.report("glfw + window + glad");

    glEnable(GL_DEPTH_TEST);
    Shader mouseShader("src/vs/mouse.vs", "src/fs/mouse.fs");
    initTimer.report("mouseShader 컴파일");

    Shader groundShader("src/vs/ground.vs", "src/fs/ground.fs");
    initTimer.report("groundShader 컴파일");

    depthShader = new Shader("src/vs/DepthShader.vs", "src/fs/DepthShader.fs");
    initTimer.report("depthShader 컴파일");

    //오픈월드 청크 관리자 — 시작 시 3x3 청크 생성
    chunkManager = new ChunkManager(groundShader, glm::vec3(1.0f, 1.0f, 1.0f));
    initTimer.report("ChunkManager 생성");

    Mouse* mouse = new Mouse(mouseShader, glm::vec3(0.5882353, 0.2941176, 0.0));
    initTimer.report("Mouse 생성 (모델 로딩 포함?)");

    unsigned int ground_texture;
    unsigned int normalMap;
    player = mouse;
    Loader::loadTexture(ground_texture, "textures/snow.png");
    initTimer.report("텍스처: snow.png 로드");

    Loader::loadTexture(normalMap, "textures/snow_normal.png");
    initTimer.report("텍스처: snow_normal.png 로드");


    //shadow map
    depthProcessing(depthMapFBO, depthMap);
    initTimer.report("depthProcessing (shadow FBO)");

    //텍스처/노멀/그림자맵 ID를 ChunkManager에 저장 (load되는 새 청크에 자동 적용)
    chunkManager->setTexture(ground_texture);
    chunkManager->setNormalMap(normalMap);
    chunkManager->setShadowMap(depthMap);

    //초기 청크 9개 생성 — update()가 첫 호출 때 자동으로 viewRadius 범위 채움
    chunkManager->update(mouse->getPosition(), objects);
    initTimer.report("chunkManager->update");

    objects.push_back(mouse);

    mouse->setShadowMap(depthMap);

    // HUD 초기화 — 식량/HP 게이지 5칸씩 표시
    hud = new HUD();
    hud->init();
    hud->setFood(5);
    hud->setHp(5);
    initTimer.report("HUD init");

    // === 아몬드 풀링 시스템 ===
    // 무한 월드라 새 청크마다 아몬드를 new/delete하면 spike + GPU 메모리 단편화.
    // 시작 시 POOL_SIZE개를 한 번에 만들고, 이후 sliding window로 청크 따라 재배치.
    //   - 같은 청크는 deterministic 시드로 항상 같은 자리에 5개 spawn
    //   - 청크 떠나면 슬롯 회수, 새 청크가 그 슬롯 재사용
    //   - 먹은 아몬드도 청크 떠났다 돌아오면 자동 부활 (그 자리 다시 spawn)
    //
    // 풀의 Almond* 들은 objects 벡터에 등록해서 매 프레임 RenderShadow + Rendering에서 그려짐.
    // (isActive=false인 슬롯은 Almond::drawGameObject/drawShadow가 알아서 스킵)
    almondPool = new AlmondPool(mouseShader, depthMap, *chunkManager);
    for (Almond* a : almondPool->getAllAlmonds())
    {
        objects.push_back(a);
    }
    //첫 update — 시작 청크 9개에 45개 아몬드 모두 배치
    almondPool->update(mouse->getPosition(), objects);
    initTimer.report("AlmondPool 초기화 + 첫 spawn");
    std::cout << "[init] === 총 초기화 시간: " << initTimer.total()
              << " ms === 게임 루프 진입" << std::endl;

    //=== 게임 루프 진입 직전: deltaTime 측정 baseline 설정 ===
    //안 하면 첫 프레임 deltaTime에 main 초기화 시간(셰이더 컴파일/텍스처 로딩 등)이 통째로 잡혀서
    //가짜 spike로 보고됨. 진짜 멈춤이 아닌데 응답없음으로 착각하기 좋음.
    lastFrame = (float)glfwGetTime();

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float dt = UpdateDeltaTime();
        UpdateFpsCounter(window, dt);             //디버그: 창 제목에 FPS 표시 + spike 콘솔 로그
        InputManager::processInput(window, dt);   //WASD 폴링 + 카메라 추적
        UpdatePhysics(dt);
        UpdateHunger(dt);     //시간 경과로 식량 자동 감소
        UpdatePickup(dt);     //플레이어가 아몬드 근처면 먹기
        chunkManager->update(player->getPosition(), objects);   //청크 동적 load/unload
        almondPool->update(player->getPosition(), objects);     //아몬드 풀링 — 청크 따라 재배치
        RenderShadowPass();
        Rendering();
    }

    //=== 메모리 제거 — 소유권 주의 ===
    //
    // objects 벡터에는 두 종류 포인터가 섞여 있음:
    //   1) Terrain*      → ChunkManager가 만들지만 소유권은 objects에 위임 → 여기서 delete
    //   2) Almond*       → AlmondPool이 단독 소유 → objects에서 delete하면 double-free
    //   3) Mouse* (player)→ main에서 만든 단일 인스턴스 → 여기서 delete
    //
    // 그래서 풀의 Almond*들을 먼저 objects에서 빼낸 뒤 objects를 정리한다.
    {
        const auto& pooled = almondPool->getAllAlmonds();
        for (Almond* a : pooled)
        {
            auto it = std::find(objects.begin(), objects.end(), static_cast<GameObject*>(a));
            if (it != objects.end()) objects.erase(it);
        }
    }
    for (int i = 0; i < (int)objects.size(); i++)
    {
        delete objects[i];
    }
    delete almondPool;       //AlmondPool 소멸자가 풀 안 Almond* 모두 정리
    delete chunkManager;
    delete hud;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// === 게임 전체 상태를 시작 시점으로 되돌림 ===
//
// 리셋 항목:
//   1. HUD 풀로 (식량 5/5, HP 5/5)
//   2. 모든 아몬드 부활 (먹은 것들 다시 isActive=true)
//   3. 플레이어 위치/속도 초기화
//   4. 굶주림/HP 데미지 타이머 0으로
//
// 리셋 안 하는 것:
//   - 카메라 각도 (사용자 시야 유지가 자연스러움)
//   - 아몬드 위치 (시드 고정이라 어차피 같은 위치)
//   - 라이트 위치 / 시간 (의미 없음)
void RestartGame()
{
    if (!player || !hud) return;

    //1. HUD 풀로 리셋 — getMaxXxx로 가져와서 의도 명확하게
    hud->setFood(hud->getMaxFood());
    hud->setHp(hud->getMaxHp());

    //2. 모든 아몬드 부활 — 풀이 inUse 중인 슬롯만 살림 (비활성 슬롯은 어차피 안 보이니 무관)
    //   풀에 위임한 이유: "현재 active한 청크에 spawn된 것만"이라는 의미가 풀 내부에만 있음
    int revived = almondPool ? almondPool->reviveEatenAlmonds() : 0;

    //3. 플레이어 위치/속도 리셋 — Mouse 초기 spawn과 동일 (Mouse.cpp의 init 참조)
    player->setPosition(glm::vec3(0.0f, 10.0f, 0.0f));
    player->setVelocity(glm::vec3(0.0f, 0.0f, 0.0f));

    //4. 굶주림 타이머 리셋 — 안 하면 리셋 직후 바로 또 깎임
    ResetHunger();

    std::cout << "[리스폰] 게임 재시작 — 아몬드 " << revived << "개 부활" << std::endl;
}


//shadow mapping pre processing
void depthProcessing(unsigned int & depthMapFBO, unsigned int& depthMap)
{
    glGenFramebuffers(1, &depthMapFBO); //프레임버퍼 1번
    glGenTextures(1, &depthMap); //특수 도화지 판(FBO)
    glBindTexture(GL_TEXTURE_2D, depthMap); //빈 캔버스 활성화

    //깊이 전용 메모리 할당. 원래는 GL_RGB지만 깊이만 저장하니 GL_DEPTH_COMPONENT
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    //픽셀을 확대 축소할때 부드럽게 할건지(GL_LINEAR) 그대로 가져올지(GL_NEAREST)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //혹시나 문제생기면 이거로 교체
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // [수정] 맵 밖은 무조건 "그림자 없음(1.0)"으로 처리
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    //FBO와 
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO); //프레임 버퍼로 바인드
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0); //프레임버퍼dhk ㅡ메 qnxdlrl

    glDrawBuffer(GL_NONE);  // 색상 버퍼 비활성화
    glReadBuffer(GL_NONE);  // 색상 버퍼 비활성화
    glBindFramebuffer(GL_FRAMEBUFFER, 0); //초기화
}
