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
#include <GameObject/ChunkManager.h>
#include <GameObject/Mouse.h>
#include <GameObject/Almond.h>

#include <UI/HUD.h>

#include <Input/InputManager.h>

#include <Loader/Loader.h>

#include <iostream>
#include <vector>
#include <random>

#define STB_IMAGE_IMPLEMENTATION
//std_image.h를 이용해서 이미지 열려면 위에 이거 정의해야함
#include <header/std_image.h>

// 입력 처리는 모두 InputManager로 분리됨 — 콜백 forward decl 불필요

float UpdateDeltaTime();
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
std::vector<Almond*> almonds;


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

    glEnable(GL_DEPTH_TEST);
    Shader mouseShader("src/vs/mouse.vs", "src/fs/mouse.fs");
    Shader groundShader("src/vs/ground.vs", "src/fs/ground.fs");
    depthShader = new Shader("src/vs/DepthShader.vs", "src/fs/DepthShader.fs");

    //오픈월드 청크 관리자 — 시작 시 3x3 청크 생성
    chunkManager = new ChunkManager(groundShader, glm::vec3(1.0f, 1.0f, 1.0f));
    Mouse* mouse = new Mouse(mouseShader, glm::vec3(0.5882353, 0.2941176, 0.0));

    unsigned int ground_texture;
    unsigned int normalMap;
    player = mouse;
    Loader::loadTexture(ground_texture, "textures/snow.png");
    Loader::loadTexture(normalMap, "textures/snow_normal.png");


    //shadow map
    depthProcessing(depthMapFBO, depthMap);

    //텍스처/노멀/그림자맵 ID를 ChunkManager에 저장 (load되는 새 청크에 자동 적용)
    chunkManager->setTexture(ground_texture);
    chunkManager->setNormalMap(normalMap);
    chunkManager->setShadowMap(depthMap);

    //초기 청크 9개 생성 — update()가 첫 호출 때 자동으로 viewRadius 범위 채움
    chunkManager->update(mouse->getPosition(), objects);

    objects.push_back(mouse);

    mouse->setShadowMap(depthMap);

    // HUD 초기화 — 식량/HP 게이지 5칸씩 표시
    hud = new HUD();
    hud->init();
    hud->setFood(5);
    hud->setHp(5);

    // === 아몬드 랜덤 스폰 ===
    // 청크 영역(3x3 = chunkSize*1.5)에 맞춰 스폰 범위 확장.
    // 청크 한 변 64 × viewRadius 1.5(여유) = 96.
    // y는 ChunkManager::getHeightAt()으로 지형 표면에 안착.
    //
    // (Phase 3에서 청크별 절차 스폰으로 리팩터 예정. 지금은 고정 영역 + 고정 시드.)
    {
        const int   numAlmonds = 20;
        const float halfExtent = 96.0f;    //3x3 청크 거의 전체 (chunkSize * 1.5)
        const float inset      = 5.0f;
        const float minXZ = -halfExtent + inset;
        const float maxXZ =  halfExtent - inset;

        std::mt19937 rng(42);
        std::uniform_real_distribution<float> distXZ(minXZ, maxXZ);

        for (int i = 0; i < numAlmonds; ++i)
        {
            float x = distXZ(rng);
            float z = distXZ(rng);
            float y = chunkManager->getHeightAt(x, z);

            Almond* almond = new Almond(mouseShader, glm::vec3(x, y, z));
            almond->setShadowMap(depthMap);
            objects.push_back(almond);
            almonds.push_back(almond);
        }

        std::cout << "Spawned " << numAlmonds << " almonds" << std::endl;
    }

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float dt = UpdateDeltaTime();
        InputManager::processInput(window, dt);   //WASD 폴링 + 카메라 추적
        UpdatePhysics(dt);
        UpdateHunger(dt);     //시간 경과로 식량 자동 감소
        UpdatePickup(dt);     //플레이어가 아몬드 근처면 먹기
        chunkManager->update(player->getPosition(), objects);   //청크 동적 load/unload
        RenderShadowPass();
        Rendering();
    }

    //메모리 제거
    //주의: 청크들은 objects 벡터가 소유 → 여기서 delete됨.
    //chunkManager 자체는 빈 껍데기만 남으므로 별도 delete.
    for (int i = 0; i < objects.size(); i++)
    {
        delete objects[i];
    }
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

    //2. 모든 아몬드 부활 — 먹어서 isActive=false였던 것들 다 살림
    int revived = 0;
    for (Almond* a : almonds)
    {
        if (!a->getIsActive())
        {
            a->setIsActive(true);
            revived++;
        }
    }

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
